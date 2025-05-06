#ifndef KAMBY_H
#define KAMBY_H

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_CTX, KA_FALSE, KA_TRUE,
  KA_NUMBER, KA_STRING, KA_SYMBOL, KA_FUNC,
  KA_LIST, KA_EXPR, KA_BLOCK
} KaType;

typedef struct KaNode {
  KaType type;
  char *key;
  union {
    long double *number;
    char *string;
    char *symbol;
    struct KaNode *(*func)(struct KaNode **ctx, struct KaNode *args);
    struct KaNode *children;
    void *value;
  };
  struct KaNode *next;
} KaNode;

// Constructors

static inline KaNode *ka_new(KaType type) {
  KaNode *node = (KaNode *)calloc(1, sizeof(KaNode));
  node->type = type;
  return node;
}

static inline KaNode *ka_true() { return ka_new(KA_TRUE); }

static inline KaNode *ka_false() { return ka_new(KA_FALSE); }

static inline void ka_free(KaNode *node) {
  for (KaNode *curr; node; node = curr) {
    KaType type = node->type;
    type >= KA_LIST ? ka_free((KaNode *)node->value) :
    type == KA_FUNC ? (void)0 : free(node->value);
    curr = node->next;
    free(node->key), free(node);
    if (type == KA_CTX) break;
  }
}

static inline KaNode *ka_chain(KaNode *args, ...) {
  va_list vargs;
  va_start(vargs, args);
  for (KaNode *curr = args; curr; curr = curr->next = va_arg(vargs, KaNode *))
    while (curr->next) curr = curr->next;
  va_end(vargs);
  return args;
}

static inline KaNode *ka_number(long double value) {
  KaNode *node = ka_new(KA_NUMBER);
  node->number = (long double *)calloc(1, sizeof(long double));
  *node->number = value;
  return node;
}

static inline KaNode *ka_string(char *value) {
  KaNode *node = ka_new(KA_STRING);
  node->string = strdup(value);
  return node;
}

static inline KaNode *ka_symbol(char *symbol) {
  KaNode *node = ka_new(KA_SYMBOL);
  node->symbol = strdup(symbol);
  return node;
}

static inline KaNode *ka_func(KaNode *(*func)(KaNode **ctx, KaNode *args)) {
  KaNode *node = ka_new(KA_FUNC);
  node->func = func;
  return node;
}

static inline KaNode *ka_copy(KaNode *node) {
  if (!node) return ka_new(KA_NONE);

  KaNode *copy =
    node->type == KA_NUMBER ? ka_number(*node->number) :
    node->type == KA_STRING ? ka_string(node->string) :
    node->type == KA_SYMBOL ? ka_symbol(node->symbol) :
    node->type == KA_FUNC ? ka_func(node->func) : ka_new(node->type);

  if (copy->type >= KA_LIST) {
    KaNode **last = &copy->children;
    for (KaNode *curr = node->children; curr; curr = curr->next)
      last = &(*last = ka_copy(curr))->next;
  }

  if (node->key) copy->key = strdup(node->key);
  return copy;
}

static inline KaNode *ka_list(KaNode *args, ...) {
  va_list vargs;
  va_start(vargs, args);
  for (KaNode *curr = args; curr; curr = curr->next = va_arg(vargs, KaNode *));
  va_end(vargs);

  KaNode *node = ka_new(KA_LIST), **child = &node->children;
  for (KaNode *curr = args; curr; curr = curr->next) 
    child = &(*child = ka_copy(curr))->next;

  ka_free(args);
  return node;
}

static inline KaNode *ka_expr(KaNode *args, ...) {
  va_list vargs;
  va_start(vargs, args);
  for (KaNode *curr = args; curr; curr = curr->next = va_arg(vargs, KaNode *));
  va_end(vargs);

  KaNode *node = ka_new(KA_EXPR), **child = &node->children;
  for (KaNode *curr = args; curr; curr = curr->next)
    child = &(*child = ka_copy(curr))->next;

  ka_free(args);
  return node;
}

static inline KaNode *ka_block(KaNode *args, ...) {
  va_list vargs;
  va_start(vargs, args);
  for (KaNode *curr = args; curr; curr = curr->next = va_arg(vargs, KaNode *));
  va_end(vargs);

  KaNode *node = ka_new(KA_BLOCK), **child = &node->children;
  for (KaNode *curr = args; curr; curr = curr->next)
    child = &(*child = ka_copy(curr))->next;

  ka_free(args);
  return node;
}

// Variables

static inline KaNode *ka_ref(KaNode **ctx, KaNode *args) {
  KaNode *node = *ctx;
  char *sym = args->key ?: args->symbol;

  if (args->type == KA_NUMBER || isdigit(sym[0])) {
    int i = isdigit(sym[0]) ? atoi(sym) : *args->number;
    while (node && node->type != KA_CTX && i-- > 0) node = node->next;
    if (node && node->type == KA_CTX) node = NULL;
  } else while (node && strcmp(sym, node->key ?: "")) node = node->next;

  ka_free(args);
  return node;
}

static inline KaNode *ka_get(KaNode **ctx, KaNode *args) {
  KaNode *node = ka_copy(
      ka_ref(args->next ? &args->children : ctx, ka_copy(args->next ?: args)));
  ka_free(args);
  return node;
}

static inline KaNode *ka_del(KaNode **ctx, KaNode *args) {
  KaNode *prev = *ctx, *node = *ctx;

  while (node && strcmp(args->key ?: args->symbol, node->key ?: ""))
    node = (prev = node)->next;

  ka_free(args);
  if (!node) return ka_new(KA_NONE);
  node == *ctx ? (*ctx = node->next) : (prev->next = node->next);
  ka_free((node->next = NULL, node));
  return ka_new(KA_NONE);
}

static inline KaNode *ka_key(KaNode **ctx, KaNode *args) {
  KaNode *data = ka_copy(args->next);
  free(data->key);
  data->key = strdup(args->symbol);
  ka_free(args);
  return data;
}

static inline KaNode *ka_def(KaNode **ctx, KaNode *args) {
  KaNode *data = ka_copy(args->next);
  data->key = strdup(args->symbol);
  data->next = *ctx;
  ka_free(args);
  KaType type = (*ctx = data)->type;
  return type == KA_BLOCK || type == KA_FUNC ? ka_new(KA_NONE) : ka_copy(data);
}

static inline KaNode *ka_set(KaNode **ctx, KaNode *args) {
  KaNode *node = ka_ref(ctx, ka_copy(args));
  if (!node) return ka_def(ctx, args);
  else if (args->next->type == KA_NONE) return ka_del(ctx, args);

  KaNode *data = ka_copy(args->next);
  node->type >= KA_LIST ? ka_free((KaNode *)node->value) : free(node->value);
  node->value = data->value;
  KaType type = node->type = data->type;

  free(data);
  ka_free(args);
  return type == KA_BLOCK || type == KA_FUNC ? ka_new(KA_NONE) : ka_copy(node);
}

// Parser and Interpreter

static inline KaNode *ka_eval(KaNode **ctx, KaNode *nodes) {
  KaNode *head = ka_new(KA_NONE), *first = head, *last = head;
  if (!nodes) return head;

  // Evaluate expressions and resolve variables
  for (KaNode *curr = nodes, *skip = NULL; curr; curr = curr->next) {
    // Flagged node skip processing
    if (curr == skip && (last = last->next = ka_copy(curr))) continue;
    // Process node based on its type
    if (curr->type == KA_SYMBOL) {
      last = last->next = ka_get(ctx, ka_symbol(curr->symbol));
    } else if (curr->type == KA_LIST) {
      last = last->next = ka_new(curr->type);
      last->key = curr->key ? strdup(curr->key) : NULL;
      last->children = ka_eval(ctx, curr->children);
    } else if (curr->type == KA_EXPR) {
      last = last->next = ka_eval(ctx, curr->children);
    } else {
      last = last->next = ka_copy(curr);
    }
    // Flag next node for special treatment
    if (curr->next && curr->next->type == KA_SYMBOL &&
        (last->func == ka_key || last->func == ka_def ||
        last->func == ka_set || last->func == ka_del)) {
      skip =  curr->next; 
    } else if (last->func == ka_get && curr->next->next &&
        curr->next->next->type == KA_SYMBOL) {
      skip = curr->next->next;
    }
  }

  // Discard first head node
  head = head->next;
  ka_free((first->next = NULL, first));

  // Take actions based on node type
  if (head->type == KA_FUNC) {
    KaNode *result = head->func(ctx, head->next);
    ka_free((head->next = NULL, head));
    return result;
  } else if (head->type == KA_BLOCK) {
    KaNode *block_ctx = !head->next ? ka_chain(ka_new(KA_CTX), *ctx, NULL) : 
      ka_chain(head->next, ka_new(KA_CTX), *ctx, NULL);
    KaNode *block_result = ka_eval(&block_ctx, head->children);
    KaNode *result = block_result;
    while (result->next) result = result->next;
    result = ka_copy(result);
    head->next = NULL;
    ka_free(block_result), ka_free(block_ctx), ka_free(head);
    return result;
  }

  return head;
}

static inline KaNode *ka_parser(char *text, int *pos) {
  KaNode *head = ka_new(KA_NONE), *last = head;
  int length = strlen(text);

  // If at the initial position, use a negative position as a flag to wrap
  // sentences, parsing each one as a separate expression node.
  if (*pos == 0 && --(*pos)) {
    while (*pos < length) {
      KaNode *children = ka_parser(text, pos);
      if (children) (last = last->next = ka_new(KA_EXPR))->children = children;
      if (strchr(")]}", text[*pos - 1])) length = 0;
    }
  }

  // Parse each character, recognize types and create nodes.
  // If position is negative, start at the beginning.
  for (*pos = *pos < 0 ? 0 : *pos; *pos < length; (*pos)++) {
    int start = *pos, childpos = 0;
    char c = text[*pos];

    if (c == '#' || (c == '/' && text[*pos + 1] == '/'))
      while (text[++(*pos)] != '\n');
    else if (c == '/' && text[*pos + 1] == '*')
      while (!(text[++(*pos)] == '*' && text[++(*pos)] == '/'));
    else if (strchr(";,)]}\n", c)) length = 0;
    else if (strchr("([{", c)) {
      last->next = ka_new(c == '(' ? KA_EXPR : c == '[' ? KA_LIST : KA_BLOCK);
      (last = last->next)->children = ka_parser(text + *pos + 1, &childpos);
      *pos += childpos;
    } else if (strchr("'\"", c)) {
      while (text[++(*pos)] != text[start] ||
          (text[*pos - 1] == '\\' && text[*pos - 2] != '\\'));
      last = last->next = ka_new(KA_STRING);
      char *value = last->string = strndup(text + start + 1, *pos - start - 1);
      for (char *str = value; *str; str++)
        if (*str != '\\' || (str[1] != text[start] && str[1] != '\\'))
          *value++ = *str;
      *value = '\0';
    } else if (isdigit(c)) {
      while (isdigit(text[*pos + 1]) || text[*pos + 1] == '.') (*pos)++;
      last = last->next = ka_number(strtold(text + start, NULL));
    } else if (isgraph(c)) {
      while (ispunct(c) && !strchr("_", c) ?
          ispunct(text[*pos + 1]) && !strchr(";,()[]{}'\"\n", text[*pos + 1]) :
          (isalnum(text[*pos + 1]) || strchr("_", text[*pos + 1]))) (*pos)++;
      last = last->next = ka_new(KA_SYMBOL);
      last->symbol = strndup(text + start, *pos - start + 1);
    }
  }

  // Reorder expression nodes based on punctuation symbols
  for (KaNode *prev = NULL, *a = head; a && a->next;) {
    KaNode *op = a->next, *b = op->next, *next = b ? b->next : NULL, *expr;
    char *symbol = op->type == KA_SYMBOL ? op->symbol : NULL;

    // Symbolic operators
    if (symbol && (!strcmp(symbol, ":=") || !strcmp(symbol, "=") ||
          !strcmp(symbol, ":") || !strcmp(symbol, "?"))) {
      (op->next = a, a->next = b);
      a = prev ? (prev->next = op) : (head = op);
    // Unary operators
    } else if (symbol && (!strcmp(symbol, "$") || !strcmp(symbol, "!"))) {
      (expr = ka_new(KA_EXPR))->next = next;
      expr->children = (b->next = NULL, op);
      a = (prev = a)->next = expr;
    // Binary operators
    } else if (symbol && ispunct(symbol[strlen(symbol) - 1])) {
      (expr = ka_new(KA_EXPR))->next = next;
      expr->children = (op->next = a, a->next = b, b->next = NULL, op);
      a = prev ? (prev->next = expr) : (head = expr);
    } else {
      a = (prev = a)->next;
    }
  }

  KaNode *result = head->next;
  ka_free((head->next = NULL, head));

  return result;
}

// Logical operators

static inline KaNode *ka_and(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->value && right->value ? ka_copy(right) : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_or(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->value ? ka_copy(left) : right->value ?
    ka_copy(right) : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_not(KaNode **ctx, KaNode *args) {
  KaNode *result = args == NULL || args->type <= KA_FALSE ?
    ka_true() : ka_false();
  ka_free(args);
  return result;
}

// Comparison operators

static inline KaNode *ka_eq(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type == KA_NUMBER && *left->number == *right->number ||
    left->type == KA_STRING && !strcmp(left->string, right->string) ||
    left->value == right->value ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_neq(KaNode **ctx, KaNode *args) {
  return ka_not(ctx, ka_eq(ctx, args));
}

static inline KaNode *ka_gt(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number > *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_lt(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number < *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_gte(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number >= *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_lte(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number <= *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

// Arithmetic operators

static inline KaNode *ka_add(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaType ltype = left->type, rtype = right->type;

  // Add numbers
  if (ltype == KA_NUMBER && rtype == KA_NUMBER) {
    long double lnum = *left->number, rnum = *right->number;
    ka_free(args);
    return ka_number(lnum + rnum);
  }

  // Concatenate strings and numbers
  char *lstr = ltype == KA_STRING ? strdup(left->string) : ltype == KA_NUMBER ?
    (asprintf(&lstr, "%.*Lf", *left->number == (long long)*left->number ?
              0 : 2, *left->number), lstr) : NULL;
  char *rstr = rtype == KA_STRING ? strdup(right->string) : rtype == KA_NUMBER ?
    (asprintf(&rstr, "%.*Lf", *right->number == (long long)*right->number ?
              0 : 2, *right->number), rstr) : NULL;
  char *str;
  asprintf(&str, "%s%s", lstr, rstr);
  KaNode *result = ka_string(str);
  free(str), free(rstr), free(lstr), ka_free(args);
  return result;
}

static inline KaNode *ka_sub(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ? NULL :
    ka_number(*left->number - *right->number);
  ka_free(args);
  return result;
}

static inline KaNode *ka_mul(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ? NULL :
    ka_number(*left->number * *right->number);
  ka_free(args);
  return result;
}

static inline KaNode *ka_div(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ? NULL :
    ka_number(*left->number / *right->number);
  ka_free(args);
  return result;
}

static inline KaNode *ka_mod(KaNode **ctx, KaNode *args) {
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ? NULL :
    ka_number((int)*left->number % (int)*right->number);
  ka_free(args);
  return result;
}

// Conditional and loops

static inline KaNode *ka_if(KaNode **ctx, KaNode *args) {
  KaNode *cond = args, *block = args->next;
  while (cond && cond->type <= KA_FALSE)
    block = (cond = cond->next->next) && cond->next ? cond->next : cond;
  KaNode *result = ka_eval(ctx, block = ka_copy(block));
  ka_free(block), ka_free(args);
  return result;
}

static inline KaNode *ka_loop(KaNode **ctx, KaNode *args) {
  KaNode *cond = ka_copy(args), *block = ka_copy(args->next), *cond_result;
  while ((cond_result = ka_eval(ctx, cond))->type == KA_TRUE)
    ka_free(cond_result), ka_free(ka_eval(ctx, block));
  ka_free(cond_result), ka_free(block), ka_free(cond), ka_free(args);
  return ka_new(KA_NONE);
}

// I/O functions

static inline KaNode *ka_exit(KaNode **ctx, KaNode *args) {
  ka_free(args);
  for (KaNode *curr = *ctx; curr; curr = curr->next) {
    if (curr->type == KA_CTX) curr->type = KA_NONE;
  }
  ka_free(*ctx);
  exit(0);
  return NULL;
}

static inline KaNode *ka_print(KaNode **ctx, KaNode *args) {
  for (KaNode *arg = args; arg != NULL; arg = arg->next) {
    switch (arg->type) {
      case KA_NUMBER:
        if (*arg->number == (long long)(*arg->number)) {
          printf("%lld", (long long)(*arg->number));
        } else {
          printf("%.2Lf", *arg->number);
        }
        break;
      case KA_STRING:
        printf("%s", arg->string);
        break;
      default:;
    }
  }
  printf("\n");
  ka_free(args);
  return ka_new(KA_NONE);
}

// Initialize context with built-in functions

static inline KaNode *ka_init() {
  KaNode *ctx = ka_new(KA_CTX);
  void (*f)(KaNode *) = ka_free;

  // Variables
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"."),   ka_func(ka_get), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"$"),   ka_func(ka_get), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)":"),   ka_func(ka_key), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)":="),  ka_func(ka_def), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"="),   ka_func(ka_set), NULL)));

  // Logical operators
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"&&"), ka_func(ka_and), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"||"), ka_func(ka_or),  NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"!"),  ka_func(ka_not), NULL)));

  // Comparison operators
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"=="), ka_func(ka_eq),  NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"!="), ka_func(ka_neq), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)">"),  ka_func(ka_gt),  NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"<"),  ka_func(ka_lt),  NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)">="), ka_func(ka_gte), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"<="), ka_func(ka_lte), NULL)));

  // Arithmetic operators
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"+"), ka_func(ka_add), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"-"), ka_func(ka_sub), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"*"), ka_func(ka_mul), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"/"), ka_func(ka_div), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"%"), ka_func(ka_mod), NULL)));

  // Conditional and loops
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"?"),    ka_func(ka_if),   NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"..."),  ka_func(ka_loop), NULL)));
    
  // I/O functions
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"exit"),  ka_func(ka_exit), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"print"), ka_func(ka_print),NULL)));

  return ka_chain(ka_new(KA_CTX), ctx, NULL);
}

#endif
