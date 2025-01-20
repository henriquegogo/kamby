#ifndef KAMBY_H
#define KAMBY_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_CTX, KA_NUMBER, KA_STRING, KA_SYMBOL, KA_FUNC,
  KA_LIST, KA_EXPR, KA_BLOCK
} KaType;

typedef struct KaNode {
  KaType type;
  char *key;
  union {
    long double *number;
    char *string;
    struct KaNode *(*func)(struct KaNode **ctx, struct KaNode *args);
    struct KaNode *children;
    void *value;
  };
  int *refcount;
  struct KaNode *next;
} KaNode;

static inline KaNode *ka_new(KaType type) {
  KaNode *node = (KaNode *)calloc(1, sizeof(KaNode));
  node->type = type;
  node->refcount = (int *)calloc(1, sizeof(int));
  return node;
}

static inline void ka_free(KaNode *node) {
  for (KaNode *curr; node; node = curr) {
    curr = node->next;

    if ((*node->refcount)-- <= 0) {
      node->type >= KA_LIST ? ka_free((KaNode *)node->value) :
      node->type == KA_FUNC ? (void)0 : free(node->value);
      free(node->refcount);
    }

    free(node->key);
    free(node);
  }
}

static inline KaNode *ka_chain(KaNode *arg, ...) {
  va_list args;
  va_start(args, arg);

  for (KaNode *curr = arg; curr; curr = curr->next = va_arg(args, KaNode *))
    while (curr->next) curr = curr->next;

  va_end(args);
  return arg;
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

static inline KaNode *ka_symbol(char *key) {
  KaNode *node = ka_new(KA_SYMBOL);
  node->key = strdup(key);
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
    node->type == KA_SYMBOL ? ka_symbol(node->key) :
    node->type == KA_FUNC ? ka_func(node->func) : ka_new(node->type);

  if (copy->type >= KA_LIST) {
    free(copy->refcount);
    copy->value = node->value;
    copy->refcount = node->refcount;
    (*node->refcount)++;
  }

  copy->key = node->key ? strdup(node->key) : NULL;
  return copy;
}

static inline KaNode *ka_list(KaNode *arg, ...) {
  va_list args;
  va_start(args, arg);
  KaNode *node = ka_new(KA_LIST);

  for (KaNode *curr = node->children = arg;
      (curr->next = va_arg(args, KaNode *)); 
      curr = curr->next->next ? curr->next = ka_copy(curr->next) : curr->next);

  va_end(args);
  return node;
}

static inline KaNode *ka_expr(KaNode *arg, ...) {
  va_list args;
  va_start(args, arg);
  KaNode *node = ka_new(KA_EXPR);

  for (KaNode *curr = node->children = arg; curr;
      curr = curr->next = va_arg(args, KaNode *));

  va_end(args);
  return node;
}

static inline KaNode *ka_block(KaNode *arg, ...) {
  va_list args;
  va_start(args, arg);
  KaNode *node = ka_new(KA_BLOCK);

  for (KaNode *curr = node->children = arg; curr;
      curr = curr->next = va_arg(args, KaNode *));

  va_end(args);
  return node;
}

static inline KaNode *ka_get(KaNode **ctx, KaNode *arg) {
  KaNode *curr = *ctx;

  while (curr && strcmp(arg->key, curr->key ? curr->key : ""))
    curr = curr->next;

  ka_free(arg);
  return curr;
}

static inline KaNode *ka_def(KaNode **ctx, KaNode *args) {
  KaNode *symbol = args, *data = args->next;
  symbol->next = NULL;

  free(data->key);
  data->key = strdup(symbol->key);
  data->next = *ctx;

  ka_free(symbol);
  return *ctx = data;
}

static inline KaNode *ka_set(KaNode **ctx, KaNode *args) {
  KaNode *symbol = args, *data = args->next;
  KaNode *node = ka_get(ctx, ka_symbol(symbol->key));
  if (!node) return ka_def(ctx, args);
  symbol->next = NULL;
  ka_free(symbol);

  node->type >= KA_LIST ? ka_free((KaNode *)node->value) : free(node->value);
  node->type = data->type;
  node->value = data->value;

  free(data->key);
  free(data->refcount);
  free(data);
  return node;
}

static inline void ka_del(KaNode **ctx, KaNode *arg) {
  KaNode *prev = *ctx;
  KaNode *node = *ctx;

  while (node && strcmp(arg->key, node->key ? node->key : "")) {
    prev = node;
    node = node->next;
  }

  if (!node) return;
  node == *ctx ? (*ctx = node->next) : (prev->next = node->next);
  node->next = NULL;
  ka_free(arg);
  ka_free(node);
}

static inline KaNode *ka_eval(KaNode **ctx, KaNode *node) {
  KaNode *head = ka_new(KA_CTX), *first = head, *last = head;

  // Eval expressions and get variables
  for (KaNode *curr = node; curr; curr = curr->next) {
    if (curr->type == KA_SYMBOL) {
      last->next = ka_copy(ka_get(ctx, ka_symbol(curr->key)));
      if (last->next->type == KA_NONE) {
        ka_free(last->next);
        last->next = ka_symbol(curr->key);
      }
      last = last->next;
    } else if (curr->type == KA_LIST) {
      last = last->next = ka_new(curr->type);
      last->children = ka_eval(ctx, curr->children);
    } else if (curr->type == KA_EXPR) {
      last = last->next = ka_eval(ctx, curr->children);
    } else {
      last = last->next = ka_copy(curr);
    }
  }

  // Discard first head node
  head = head->next;
  first->next = NULL;
  ka_free(first);

  // Take actions based on node type
  if (head->type == KA_FUNC) {
    KaNode *result = head->func(ctx, head->next);
    head->next = NULL;
    ka_free(head);
    return result ? ka_copy(result) : ka_new(KA_NONE);
  } else if (head->type == KA_BLOCK) {
    KaNode *result = ka_eval(ctx, head->children); // TODO: block ctx
    ka_free(head);
    return result;
  }

  return head;
}

#endif
