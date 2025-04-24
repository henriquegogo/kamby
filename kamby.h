#ifndef KAMBY_H
#define KAMBY_H

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_FALSE, KA_TRUE, KA_CTX,
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
    curr = node->next;

    type >= KA_LIST ? ka_free((KaNode *)node->value) :
    type == KA_FUNC ? (void)0 : free(node->value);

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
    for (KaNode *curr = node->children; curr; curr = curr->next) {
      *last = ka_copy(curr);
      last = &(*last)->next;
    }
  }

  if (node->key) copy->key = strdup(node->key);
  return copy;
}

static inline KaNode *ka_list(KaNode *args, ...) {
  va_list vargs;
  va_start(vargs, args);
  for (KaNode *curr = args; curr; curr = curr->next = va_arg(vargs, KaNode *));
  va_end(vargs);

  KaNode *node = ka_new(KA_LIST);
  KaNode **child = &node->children;
  for (KaNode *curr = args; curr; curr = curr->next) {
    *child = ka_copy(curr);
    child = &(*child)->next;
  }

  ka_free(args);
  return node;
}

static inline KaNode *ka_expr(KaNode *args, ...) {
  va_list vargs;
  va_start(vargs, args);
  for (KaNode *curr = args; curr; curr = curr->next = va_arg(vargs, KaNode *));
  va_end(vargs);

  KaNode *node = ka_new(KA_EXPR);
  KaNode **child = &node->children;
  for (KaNode *curr = args; curr; curr = curr->next) {
    *child = ka_copy(curr);
    child = &(*child)->next;
  }

  ka_free(args);
  return node;
}

static inline KaNode *ka_block(KaNode *args, ...) {
  va_list vargs;
  va_start(vargs, args);
  for (KaNode *curr = args; curr; curr = curr->next = va_arg(vargs, KaNode *));
  va_end(vargs);

  KaNode *node = ka_new(KA_BLOCK);
  KaNode **child = &node->children;
  for (KaNode *curr = args; curr; curr = curr->next) {
    *child = ka_copy(curr);
    child = &(*child)->next;
  }

  ka_free(args);
  return node;
}

// Variables

static inline KaNode *ka_ref(KaNode **ctx, KaNode *args) {
  KaNode *node = *ctx;
  char *sym = args->symbol;
  if (sym[0] == '$' && !isdigit(sym[1])) {
    char num[4096];
    KaNode *ref = ka_ref(ctx, ka_symbol(sym + 1));
    if (ref->type == KA_NUMBER) sprintf(num, "%d", (int)*ref->number);
    node = ka_ref(ctx, ka_symbol(ref->type != KA_NUMBER ? ref->symbol : num));
  } else if (isdigit(sym[0])) {
    int i = atoi(sym);
    while (node && node->type != KA_CTX && i-- > 0) node = node->next;
    if (node && node->type == KA_CTX) node = NULL;
  } else if (sym[0] == '$') node = ka_ref(ctx, ka_symbol(sym + 1));
  else while (node && strcmp(sym, node->key ?: "")) node = node->next;
  ka_free(args);
  return node;
}

static inline KaNode *ka_get(KaNode **ctx, KaNode *args) {
  return ka_copy(ka_ref(ctx, args));
}

static inline KaNode *ka_key(KaNode **ctx, KaNode *args) {
  KaNode *data = ka_copy(args->next);
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

  KaNode *data = ka_copy(args->next);
  node->type >= KA_LIST ? ka_free((KaNode *)node->value) : free(node->value);
  node->value = data->value;
  KaType type = node->type = data->type;

  free(data);
  ka_free(args);
  return type == KA_BLOCK || type == KA_FUNC ? ka_new(KA_NONE) : ka_copy(node);
}

static inline KaNode *ka_del(KaNode **ctx, KaNode *args) {
  KaNode *prev = *ctx, *node = *ctx;
  int i = atoi(args->symbol + 1);

  while (node && args->symbol[0] == '$' ? i-- > 0 :
      node && strcmp(args->symbol, node->key ?: "")) {
    prev = node;
    node = node->next;
  }

  ka_free(args);
  if (!node) return ka_new(KA_NONE);
  node == *ctx ? (*ctx = node->next) : (prev->next = node->next);
  ka_free((node->next = NULL, node));
  return ka_new(KA_NONE);
}

// Parser and Interpreter

static inline KaNode *ka_eval(KaNode **ctx, KaNode *nodes) {
  KaNode *head = ka_new(KA_NONE), *first = head, *last = head;
  if (!nodes) return head;

  // Eval expressions and get variables
  for (KaNode *curr = nodes; curr; curr = curr->next) {
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
    // Skip getting the next node's value and treat it as a SYMBOL
    if (last->func == ka_key || last->func == ka_def ||
        last->func == ka_set || last->func == ka_del) {
      last = last->next = ka_copy(curr = curr->next);
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
      last = last->next = ka_new(KA_EXPR);
      last->children = ka_parser(text, pos);
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
    else if (strchr(";,)]}", c)) length = 0;
    else if (strchr("([{", c)) {
      last->next = ka_new(c == '(' ? KA_EXPR : c == '[' ? KA_LIST : KA_BLOCK);
      last->next->children = ka_parser(text + *pos + 1, &childpos);
      last = last->next;
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
      while (isgraph(c = text[*pos + 1]) && !strchr(";,()[]{}", c)) (*pos)++;
      last = last->next = ka_new(KA_SYMBOL);
      last->symbol = strndup(text + start, *pos - start + 1);
    }
  }

  // Reorder expression nodes when symbols ends with a punctuation.
  for (KaNode *prev = NULL, *a = head; a && a->next && a->next->next;) {
    KaNode *op = a->next, *b = op->next, *next = b->next;
    char *symbol = op->type == KA_SYMBOL ? op->symbol : NULL;

    if (symbol && (!strcmp(symbol, ":=") || !strcmp(symbol, "=") ||
          !strcmp(symbol, ":") || !strcmp(symbol, "?"))) {
      (op->next = a, a->next = b);
      a = prev ? (prev->next = op) : (head = op);
    } else if (symbol && ispunct(symbol[strlen(symbol) - 1])) {
      KaNode *expr = ka_new(KA_EXPR);
      expr->children = (op->next = a, a->next = b, b->next = NULL, op);
      expr->next = next;
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
  KaNode *result = left->value ? ka_copy(left) :
    right->value ? ka_copy(right) : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_not(KaNode **ctx, KaNode *args) {
  KaNode *result = args == NULL || args->type == KA_FALSE ?
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
  KaNode *result = left->type != right->type ? NULL :
    left->type == KA_NUMBER ? ka_number(*left->number + *right->number) : NULL;

  if (left->type == KA_STRING) {
    int size = strlen(left->string) + strlen(right->string) + 1;
    char *str = (char *)calloc(1, size);
    result = ka_string(strcat(strcpy(str, left->string), right->string));
    free(str);
  }

  ka_free(args);
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
  KaType condition = args->type;
  KaNode *block = ka_copy(args->next), *else_block = ka_copy(args->next->next);
  KaNode *result = ka_eval(ctx, condition == KA_TRUE ? block : else_block);
  ka_free(else_block), ka_free(block), ka_free(args);
  return result;
}

static inline KaNode *ka_loop(KaNode **ctx, KaNode *args) {
  KaNode *condition = ka_copy(args), *block = ka_copy(args->next),
         *condition_result;

  while ((condition_result = ka_eval(ctx, condition))->type == KA_TRUE) {
    ka_free(condition_result), ka_free(ka_eval(ctx, block));
  }

  ka_free(condition_result), ka_free(block), ka_free(condition), ka_free(args);
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
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"get"), ka_func(ka_get), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"def"), ka_func(ka_def), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"set"), ka_func(ka_set), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"del"), ka_func(ka_del), NULL)));
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
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"if"),   ka_func(ka_if),   NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"loop"), ka_func(ka_loop), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"?"),    ka_func(ka_if),   NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"..."),  ka_func(ka_loop), NULL)));
    
  // I/O functions
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"exit"),  ka_func(ka_exit), NULL)));
  f(ka_def(&ctx, ka_chain(ka_symbol((char *)"print"), ka_func(ka_print),NULL)));

  return ka_chain(ka_new(KA_CTX), ctx, NULL);
}

#endif
