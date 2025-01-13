#ifndef KAMBY_H
#define KAMBY_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_NUMBER, KA_STRING, KA_SYMBOL, KA_FUNC, KA_LIST, KA_EXPR, KA_BLOCK
} KaType;

typedef struct KaNode {
  KaType type;
  char *key;
  union {
    long double *number;
    char *string;
    struct KaNode *(*function)(struct KaNode *node, ...);
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
      node->type >= KA_LIST ? ka_free((KaNode *)node->value) : free(node->value);
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

static inline KaNode *ka_func(KaNode *(*func)(KaNode *node, ...)) {
  KaNode *node = ka_new(KA_FUNC);
  node->function = func;
  return node;
}

static inline KaNode *ka_copy(KaNode *node) {
  if (!node) return NULL;

  KaNode *copy =
    node->type == KA_NUMBER ? ka_number(*node->number) :
    node->type == KA_STRING ? ka_string(node->string) :
    node->type == KA_SYMBOL ? ka_symbol(node->key) : ka_new(node->type);

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

static inline KaNode *ka_get(KaNode *symbol, KaNode **env) {
  KaNode *curr = *env;

  while (curr && strcmp(symbol->key, curr->key ? curr->key : "")) {
    curr = curr->next;
  }

  ka_free(symbol);
  return curr;
}

static inline KaNode *ka_def(KaNode *symbol, KaNode *node, KaNode **env) {
  free(node->key);
  node->key = strdup(symbol->key);
  node->next = *env;
  ka_free(symbol);
  return *env = node;
}

static inline KaNode *ka_set(KaNode *symbol, KaNode *data, KaNode **env) {
  KaNode *node = ka_get(ka_symbol(symbol->key), env);
  if (!node) return ka_def(symbol, data, env);
  ka_free(symbol);

  node->type >= KA_LIST ? ka_free((KaNode *)node->value) : free(node->value);
  node->type = data->type;
  node->value = data->value;

  free(data->key);
  free(data->refcount);
  free(data);
  return node;
}

static inline void ka_del(KaNode *symbol, KaNode **env) {
  KaNode *prev = *env;
  KaNode *node = *env;

  while (node && strcmp(symbol->key, node->key ? node->key : "")) {
    prev = node;
    node = node->next;
  }

  if (!node) return;
  node == *env ? (*env = node->next) : (prev->next = node->next);
  node->next = NULL;
  ka_free(symbol);
  ka_free(node);
}

static inline KaNode *ka_eval(KaNode *node, KaNode **env) {
  KaNode *head = ka_new(KA_NONE), *first = head, *last = head;

  // Eval expressions and get variables
  for (KaNode *curr = node; curr; curr = curr->next) {
    KaNode *children;
    switch (curr->type) {
      case KA_SYMBOL:
        last = last->next = ka_copy(ka_get(ka_symbol(curr->key), env));
        break;
      case KA_EXPR:
        last = last->next = ka_eval(curr->children, env);
        break;
      case KA_LIST:
        last = last->next = ka_new(curr->type);
        last->children = ka_eval(curr->children, env);
        break;
      default:
        last = last->next = ka_copy(curr);
    }
  }

  // Discard first head node
  head = head->next;
  first->next = NULL;
  ka_free(first);

  // Take actions based on node type
  for (KaNode *curr = head; curr; curr = curr->next) {
  }

  return head;
}

#endif
