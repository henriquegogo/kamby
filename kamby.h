#ifndef KAMBY_H
#define KAMBY_H

#include <ctype.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_CTX, KA_FALSE, KA_TRUE, KA_NUMBER, KA_STRING, KA_SYMBOL, KA_FUNC,
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

// Function prototypes that will be defined later

static inline KaNode *ka_eval(KaNode **ctx, KaNode *nodes);
static inline KaNode *ka_while(KaNode **ctx, KaNode *nodes);

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
    int has_key = node->key ? 1 : 0;
    type >= KA_LIST ? ka_free((KaNode *)node->value) :
    type == KA_FUNC ? (void)0 : free(node->value);
    curr = node->next;
    free(node->key), free(node);
    if (type == KA_CTX && !has_key) break;
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
  if (!args) return ka_new(KA_NONE);
  return ka_copy(ka_ref(ctx, args));
}

static inline KaNode *ka_del(KaNode **ctx, KaNode *args) {
  if (!args || (!args->key && !args->symbol))
    return ka_free(args), ka_new(KA_NONE);
  KaNode *prev = *ctx, *node = *ctx;
  char *sym = args->type != KA_STRING && args->key ? args->key : args->symbol;
  while (node && strcmp(sym, node->key ?: "")) node = (prev = node)->next;
  ka_free(args);
  if (!node) return ka_new(KA_NONE);
  node == *ctx ? (*ctx = node->next) : (prev->next = node->next);
  ka_free((node->next = NULL, node));
  return ka_new(KA_NONE);
}

static inline KaNode *ka_key(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *data = ka_copy(args->next);
  free(data->key);
  data->key = strdup(args->symbol);
  ka_free(args);
  return data;
}

static inline KaNode *ka_def(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *data = ka_copy(args->next);
  data->key = strdup(args->symbol);
  data->next = *ctx;
  ka_free(args);
  KaType type = (*ctx = data)->type;
  return type == KA_FUNC || type == KA_BLOCK ? ka_new(KA_NONE) : ka_copy(data);
}

static inline KaNode *ka_set(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *node = ka_ref(ctx, ka_copy(args));
  if (!node) return ka_def(ctx, args);
  else if (!args->next->type) return ka_del(ctx, args);

  KaNode *data = ka_copy(args->next);
  if (!node->key && args->next->key) node->key = strdup(args->next->key);
  if (node->type >= KA_LIST) ka_free((KaNode *)node->value);
  else if (node->type != KA_FUNC) free(node->value);
  node->value = data->value;
  KaType type = node->type = data->type;

  free(data->key), free(data);
  ka_free(args);
  return type == KA_FUNC || type == KA_BLOCK ? ka_new(KA_NONE) : ka_copy(node);
}

static inline KaNode *ka_bind(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next, *last, *last_ret;

  KaNode *blk_ctx = ka_chain(left->children, ka_new(KA_CTX), *ctx, NULL);
  KaNode *blk_ret = ka_eval(&blk_ctx,
      right->type == KA_BLOCK ? right->children : right);

  // Detach block context
  for (last = left->children; last->next->type != KA_CTX; last = last->next);
  ka_free(last->next);
  last->next = NULL;

  if (left->key && right->type == KA_BLOCK)
    ka_free(ka_set(ctx, ka_chain(ka_symbol(left->key), ka_copy(left), NULL)));

  for (last_ret = blk_ret; last_ret->next; last_ret = last_ret->next);
  KaNode *result = ka_copy(last_ret);

  ka_free(blk_ret), ka_free(args);
  return result;
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
    } else last = last->next = ka_copy(curr);
    // Flag next node for special treatment
    if (curr->next && curr->next->type == KA_SYMBOL &&
        (last->func == ka_key || last->func == ka_def ||
        last->func == ka_set || last->func == ka_del)) skip = curr->next; 
    else if (last->func == ka_while) skip = curr->next;
    else if (curr->next && last->func == ka_bind) skip = curr->next->next;
  }

  // Discard first head node
  head = head->next;
  ka_free((first->next = NULL, first));

  // Take actions based on node type
  if (head->type == KA_FUNC) {
    KaNode *result = head->func(ctx, head->next);
    ka_free((head->next = NULL, head));
    return result;
  } else if (head->type == KA_BLOCK && head->next) {
    // Avoid deep recursion. Use loop functions (e.g., while, each) instead.
    KaNode *blk_ctx = ka_chain(head->next, ka_new(KA_CTX), *ctx, NULL);
    KaNode *last_ret, *blk_ret = ka_eval(&blk_ctx, head->children);
    for (last_ret = blk_ret; last_ret->next; last_ret = last_ret->next);
    KaNode *result = ka_copy(last_ret);
    ka_free(blk_ret), ka_free(blk_ctx), ka_free((head->next = NULL, head));
    return result;
  }

  return head;
}

static inline KaNode *ka_parser(char *text, int *pos) {
  KaNode *head = ka_new(KA_NONE), *last = head;
  if (!text) return head;
  int length = strlen(text);

  // If at the initial position, use a negative position as a flag to wrap
  // sentences, parsing each one as a separate expression node.
  if (*pos == 0 && --(*pos)) while (*pos < length) {
    KaNode *children = ka_parser(text, pos);
    if (children) (last = last->next = ka_new(KA_EXPR))->children = children;
    if (strchr(")]}", text[*pos - 1])) length = 0;
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
      while (isdigit(text[*pos + 1]) ||
          (text[*pos + 1] == '.' && isdigit(text[*pos + 2]))) (*pos)++;
      last = last->next = ka_number(strtold(text + start, NULL));
    } else if (isgraph(c)) {
      while (ispunct(c) && !strchr("_", c) ?
          ispunct(text[*pos + 1]) && !strchr("$;,()[]{}'\"\n", text[*pos + 1]) :
          (isalnum(text[*pos + 1]) || strchr("_", text[*pos + 1]))) (*pos)++;
      last = last->next = ka_new(KA_SYMBOL);
      last->symbol = strndup(text + start, *pos - start + 1);
    }
  }

  // Reorder operators by precedence
  for (int step = 1; step <= 5; step++) {
    for (KaNode *prev = NULL, *a = head; a && a->next;) {
      KaNode *op = a->next, *b = op->next, *next = b ? b->next : NULL, *expr;
      char *sym = op->type == KA_SYMBOL ? op->symbol : NULL;
      int isunary = sym && strchr("$!", sym[0]) && !sym[1];
      int isassign = sym && ((strchr(":=", sym[0]) && !sym[1]) ||
          (strchr("+-*/%:", sym[0]) && strchr("=", sym[1] ?: ' ')));
      int isbind = sym && strchr(".", sym[0]) && !sym[1];
      int isexpr = sym && strchr("?", sym[0]) && !sym[1];
      int ispunctuation = sym && ispunct(sym[0]) && !isunary &&
        !isassign && !isbind && !isexpr;
      // Reorder and wrap unary operators in expressions
      if (step == 1 && isunary) {
        (a->next = ka_new(KA_EXPR))->next = next;
        a->next->children = (b && (b->next = NULL), op);
      // Reorder and wrap binary operators in expressions by precedence
      } else if ((step == 2 && isbind) || (step == 3 && ispunctuation) ||
          (step == 4 && isassign)) {
        (expr = ka_new(KA_EXPR))->next = next;
        expr->children =
          (op && (op->next = a), a && (a->next = b), b && (b->next = NULL), op);
        a = prev ? (prev->next = expr) : (head = expr);
      // Reorder operators in expressions
      } else if (step == 4 && isexpr) {
        (op->next = a, a->next = b);
        a = prev ? (prev->next = op) : (head = op);
      } else a = (prev = a)->next;
    }
  }

  KaNode *result = head->next;
  ka_free((head->next = NULL, head));
  return result;
}

// Logical operators

static inline KaNode *ka_and(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->value && right->value ? ka_copy(right) : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_or(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->value ? ka_copy(left) :
    right->value ? ka_copy(right) : ka_false();
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
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
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
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number > *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_lt(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number < *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_gte(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number >= *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

static inline KaNode *ka_lte(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type != KA_NUMBER || right->type != KA_NUMBER ?
    ka_false() : *left->number <= *right->number ? ka_true() : ka_false();
  ka_free(args);
  return result;
}

// Conditional, lists and loops

static inline KaNode *ka_if(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *cond = args, *block = args->next;
  while (cond && cond->type <= KA_FALSE)
    block = (cond = cond->next->next) && cond->next ? cond->next : cond;
  KaNode *result = ka_eval(ctx, (block = ka_copy(block))->type == KA_BLOCK ?
      block->children : block);
  ka_free(block), ka_free(args);
  return result;
}

static inline KaNode *ka_while(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *cond = ka_copy(args), *cond_ret;
  KaNode *block = args->next->type == KA_BLOCK ? args->next->children : NULL;
  while ((cond_ret = ka_eval(ctx, cond->children))->type >= KA_TRUE)
    ka_free(cond_ret), ka_free(ka_eval(ctx, block));
  ka_free(cond_ret), ka_free(cond), ka_free(args);
  return ka_new(KA_NONE);
}

static inline KaNode *ka_for(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *result = ka_new(KA_LIST);
  KaNode *last = result->children = ka_new(KA_NONE), *children = last;
  KaNode *block = args->next->type == KA_BLOCK ? args->next->children : NULL;
  for (KaNode *curr = args->children; curr; curr = curr->next) {
    KaNode *blk_ctx = ka_chain(ka_copy(curr), ka_new(KA_CTX), *ctx, NULL);
    KaNode *blk_ret = ka_eval(&blk_ctx, block);
    free(blk_ret->key);
    blk_ret->key = curr->key ? strdup(curr->key) : NULL;
    if (blk_ret->type) last = last->next = ka_copy(blk_ret);
    ka_free(blk_ret), ka_free(blk_ctx);
  }
  result->children = children->next;
  ka_free((children->next = NULL, children)), ka_free(args);
  return result;
}

static inline KaNode *ka_range(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *result = ka_new(KA_LIST), *last = NULL;
  int start = *args->number, j = *args->next->number;
  for (int i = start; (start <= j ? i <= j : i >= j); (start <= j ? i++ : i--))
    last = last ? last->next = ka_number(i) : (result->children = ka_number(i));
  ka_free(args);
  return result;
}

static inline KaNode *ka_merge(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next, *result = ka_new(KA_LIST);
  KaType ltype = left->type, rtype = right->type;
  // Merge lists
  if (ltype == KA_LIST && rtype == KA_LIST) {
    result->children = ka_chain(left->children, right->children, NULL);
    left->children = NULL, right->children = NULL;
  }
  // Prepend or append to list
  else if (ltype == KA_LIST || rtype == KA_LIST) {
    result->children = rtype != KA_LIST ?
      ka_chain(left->children, ka_copy(right), NULL) :
      ka_chain(ka_copy(left), right->children, NULL);
    rtype != KA_LIST ? (left->children = NULL) : (right->children = NULL);
  }
  ka_free(args);
  return result;
}

static inline KaNode *ka_cat(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaType ltype = left->type, rtype = right->type;
  char *str, *lstr, *rstr;
  int r;
  lstr = ltype == KA_STRING ? strdup(left->string) : ltype == KA_NUMBER ?
    (r = asprintf(&lstr, "%.*Lf", *left->number == (long long)*left->number ?
              0 : 2, *left->number), lstr) : NULL;
  rstr = rtype == KA_STRING ? strdup(right->string) : rtype == KA_NUMBER ?
    (r = asprintf(&rstr, "%.*Lf", *right->number == (long long)*right->number ?
              0 : 2, *right->number), rstr) : NULL;
  r = asprintf(&str, "%s%s", lstr, rstr);
  KaNode *result = ka_string(str);
  free(str), free(rstr), free(lstr);
  ka_free(args);
  return result;
}

static inline KaNode *ka_split(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next,
         *result = ka_new(KA_LIST), *last = NULL;
  KaType ltype = left->type, rtype = right->type;
  // Split string into list - empty separator
  if (ltype == KA_STRING && rtype == KA_STRING && !right->string[0]) {
    for (int i = 0; left->string[i]; i++) {
      KaNode *str = ka_string((char[]){ left->string[i], '\0' });
      last = last ? (last->next = str) : (result->children = str);
    }
  }
  // Split string into list - string separator
  else if (ltype == KA_STRING && rtype == KA_STRING) {
    char *str = strtok(left->string, right->string);
    if (str) last = result->children = ka_string(str);
    while ((str = strtok(NULL, right->string)))
      last = last->next = ka_string(str);
  }
  ka_free(args);
  return result;
}

// Arithmetic operators

static inline KaNode *ka_add(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaType ltype = left->type, rtype = right->type;
  // Add numbers
  if (ltype == KA_NUMBER && rtype == KA_NUMBER) {
    long double lnum = *left->number, rnum = *right->number;
    ka_free(args);
    return ka_number(lnum + rnum);
  }
  else if (ltype == KA_LIST || rtype == KA_LIST) return ka_merge(ctx, args);
  return ka_cat(ctx, args);
}

static inline KaNode *ka_sub(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type == KA_NUMBER && right->type == KA_NUMBER ?
    ka_number(*left->number - *right->number) : ka_new(KA_NONE);
  ka_free(args);
  return result;
}

static inline KaNode *ka_mul(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaType ltype = left->type, rtype = right->type;
  // Multiply numbers
  if (ltype == KA_NUMBER && rtype == KA_NUMBER) {
    KaNode *result = ltype == KA_NUMBER && rtype == KA_NUMBER ?
      ka_number(*left->number * *right->number) : ka_new(KA_NONE);
    ka_free(args);
    return result;
  }
  else if (ltype == KA_LIST || rtype == KA_BLOCK) return ka_for(ctx, args);
  return ka_free(args), ka_new(KA_NONE);
}

static inline KaNode *ka_div(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaType ltype = left->type, rtype = right->type;
  // Divide numbers
  if (ltype == KA_NUMBER && rtype == KA_NUMBER) {
    KaNode *result = left->type == KA_NUMBER && right->type == KA_NUMBER ?
      ka_number(*left->number / *right->number) : NULL;
    ka_free(args);
    return result;
  }
  else if (ltype == KA_STRING && rtype == KA_STRING) return ka_split(ctx, args);
  return ka_free(args), ka_new(KA_NONE);
}

static inline KaNode *ka_mod(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *left = args, *right = args->next;
  KaNode *result = left->type == KA_NUMBER && right->type == KA_NUMBER ?
    ka_number((int)*left->number % (int)*right->number) : ka_new(KA_NONE);
  ka_free(args);
  return result;
}

static inline KaNode *ka_addset(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *symbol = ka_symbol(args->key);
  return ka_set(ctx, ka_chain(symbol, ka_add(ctx, args), NULL));
}

static inline KaNode *ka_subset(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *symbol = ka_symbol(args->key);
  return ka_set(ctx, ka_chain(symbol, ka_sub(ctx, args), NULL));
}

static inline KaNode *ka_mulset(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *symbol = ka_symbol(args->key);
  return ka_set(ctx, ka_chain(symbol, ka_mul(ctx, args), NULL));
}

static inline KaNode *ka_divset(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *symbol = ka_symbol(args->key);
  return ka_set(ctx, ka_chain(symbol, ka_div(ctx, args), NULL));
}

static inline KaNode *ka_modset(KaNode **ctx, KaNode *args) {
  if (!args || !args->next) return ka_free(args), ka_new(KA_NONE);
  KaNode *symbol = ka_symbol(args->key);
  return ka_set(ctx, ka_chain(symbol, ka_mod(ctx, args), NULL));
}

// I/O functions

static inline KaNode *ka_print(KaNode **ctx, KaNode *args) {
  for (KaNode *arg = args; arg != NULL; arg = arg->next) {
    if (arg->type == KA_NUMBER && *arg->number == (long long)(*arg->number))
      printf("%lld", (long long)(*arg->number));
    else if (arg->type == KA_NUMBER) printf("%.2Lf", *arg->number);
    else if (arg->type == KA_STRING) printf("%s", arg->string);
    else if (arg->type == KA_LIST) {
      KaNode *copy = ka_copy(arg);
      ka_free(ka_print(ctx, copy->children));
      printf("\x1B[A");
      ka_free((copy->children = NULL, copy));
    }
  }
  printf("\n");
  ka_free(args);
  return ka_new(KA_NONE);
}

static inline KaNode *ka_input(KaNode **ctx, KaNode *args) {
  ka_free(args);
  char input[8192], *end;
  int r = scanf("%[^\n]", input);
  getchar();
  long double number = strtold(input, &end);
  return *end == '\0' ? ka_number(number) : ka_string(input);
}

static inline KaNode *ka_read(KaNode **ctx, KaNode *args) {
  FILE *file = args ? fopen(args->string, "r") : NULL;
  ka_free(args);
  if (!file) return ka_new(KA_NONE);
  size_t cap = 1024, len = 0, n;
  char *buf = (char *)calloc(1, cap);
  while ((n = fread(buf + len, 1, cap - len, file)) > 0 && (len += n) == cap)
    buf = (char *)realloc(buf, (cap *= 2));
  buf[len] = '\0';
  KaNode *result = ka_string(buf);
  fclose(file), free(buf);
  return result;
}

static inline KaNode *ka_write(KaNode **ctx, KaNode *args) {
  FILE *file = args ? fopen(args->string, "w") : NULL;
  if (!file || !args->next) return ka_new(KA_NONE);
  fputs(args->next->string, file);
  fclose(file);
  ka_free(args);
  return ka_new(KA_NONE);
}

static inline KaNode *ka_load(KaNode **ctx, KaNode *args) {
  if (!args) return ka_new(KA_NONE);
  // Load dynamic library
  if (!strcmp(args->string + strlen(args->string) - 3, ".so")) {
    void *lib = dlopen(args->string, RTLD_NOW);
    ka_free(args);
    if (!lib) return ka_new(KA_NONE);
    typedef void (*ka_extend)(KaNode **);
    ka_extend extend = (ka_extend)dlsym(lib, "ka_extend");
    if (extend) extend(ctx);
    else dlclose(lib);
    return ka_new(KA_NONE);
  }
  // Load and evaluate script file
  int pos = 0;
  KaNode *source = ka_read(ctx, ka_copy(args));
  KaNode *expr = ka_parser(source->string, &pos);
  KaNode *result = ka_eval(ctx, expr);
  ka_free(expr), ka_free(source), ka_free(args);
  return result;
}

// Initialize context with built-in functions

static inline KaNode *ka_init() {
  const KaNode kv[] = {
    // Variables
    { .key = (char *)"$",   .value = ka_func(ka_get)  },
    { .key = (char *)":",   .value = ka_func(ka_key)  },
    { .key = (char *)":=",  .value = ka_func(ka_def)  },
    { .key = (char *)"=",   .value = ka_func(ka_set)  },
    { .key = (char *)".",   .value = ka_func(ka_bind) },
    { .key = (char *)"get", .value = ka_func(ka_get)  },
    { .key = (char *)"def", .value = ka_func(ka_def)  },
    { .key = (char *)"set", .value = ka_func(ka_set)  },
    { .key = (char *)"del", .value = ka_func(ka_del)  },
    // Logical operators
    { .key = (char *)"&&", .value = ka_func(ka_and) },
    { .key = (char *)"||", .value = ka_func(ka_or)  },
    { .key = (char *)"!",  .value = ka_func(ka_not) },
    // Comparison operators
    { .key = (char *)"==", .value = ka_func(ka_eq)  },
    { .key = (char *)"!=", .value = ka_func(ka_neq) },
    { .key = (char *)">",  .value = ka_func(ka_gt)  },
    { .key = (char *)"<",  .value = ka_func(ka_lt)  },
    { .key = (char *)">=", .value = ka_func(ka_gte) },
    { .key = (char *)"<=", .value = ka_func(ka_lte) },
    // Conditional, lists and loops
    { .key = (char *)"?",     .value = ka_func(ka_if)    },
    { .key = (char *)"..",    .value = ka_func(ka_range) },
    { .key = (char *)"if",    .value = ka_func(ka_if)    },
    { .key = (char *)"while", .value = ka_func(ka_while) },
    { .key = (char *)"for",   .value = ka_func(ka_for)   },
    // Arithmetic operators
    { .key = (char *)"+",  .value = ka_func(ka_add)    },
    { .key = (char *)"-",  .value = ka_func(ka_sub)    },
    { .key = (char *)"*",  .value = ka_func(ka_mul)    },
    { .key = (char *)"/",  .value = ka_func(ka_div)    },
    { .key = (char *)"%",  .value = ka_func(ka_mod)    },
    { .key = (char *)"+=", .value = ka_func(ka_addset) },
    { .key = (char *)"-=", .value = ka_func(ka_subset) },
    { .key = (char *)"*=", .value = ka_func(ka_mulset) },
    { .key = (char *)"/=", .value = ka_func(ka_divset) },
    { .key = (char *)"%=", .value = ka_func(ka_modset) },
    // Default values
    { .key = (char *)"true",  .value = ka_true()  },
    { .key = (char *)"false", .value = ka_false() },
    { .key = (char *)"else",  .value = ka_true()  },
    // I/O
    { .key = (char *)"print", .value = ka_func(ka_print) },
    { .key = (char *)"input", .value = ka_func(ka_input) },
    { .key = (char *)"read",  .value = ka_func(ka_read)  },
    { .key = (char *)"write", .value = ka_func(ka_write) },
    { .key = (char *)"load",  .value = ka_func(ka_load)  },
  };

  KaNode *init = ka_new(KA_CTX), *ctx = ka_new(KA_CTX);
  ctx->key = strdup("(ctx)");
  for (int i = 0; i < sizeof(kv) / sizeof(KaNode); i++)
    ka_free(ka_def(&init, ka_chain(ka_symbol(kv[i].key), kv[i].value, NULL)));
  return ka_chain(ctx, init, NULL);
}

#endif
