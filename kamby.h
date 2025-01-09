#ifndef KAMBY_H
#define KAMBY_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_NUMBER, KA_STRING, KA_SYMBOL, KA_LIST, KA_EXPR, KA_BLOCK
} KaType;

typedef struct KaNode {
  KaType type;
  char *key;
  union {
    long double *number;
    char *string;
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
  for (KaNode *next; node; node = next) {
    next = node->next;

    if ((*node->refcount)-- <= 0) {
      node->type >= KA_LIST ? ka_free((KaNode *)node->value) : free(node->value);
      free(node->refcount);
    }

    free(node->key);
    free(node);
  }
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

static inline KaNode *ka_chain(KaNode *chain, ...) {
  va_list args;
  va_start(args, chain);

  for (KaNode *last = chain; last; last = last->next = va_arg(args, KaNode *))
    while (last->next) last = last->next;

  va_end(args);
  return chain;
}

static inline KaNode *ka_list(KaNode *chain, ...) {
  va_list args;
  va_start(args, chain);
  KaNode *node = ka_new(KA_LIST);

  for (KaNode *last = node->children = chain;
      (last->next = va_arg(args, KaNode *)); 
      last = last->next->next ? last->next = ka_copy(last->next) : last->next);

  va_end(args);
  return node;
}

static inline KaNode *ka_expr(KaNode *chain, ...) {
  va_list args;
  va_start(args, chain);
  KaNode *node = ka_new(KA_EXPR);

  for (KaNode *last = node->children = chain; last;
      last = last->next = va_arg(args, KaNode *));

  va_end(args);
  return node;
}

static inline KaNode *ka_block(KaNode *chain, ...) {
  va_list args;
  va_start(args, chain);
  KaNode *node = ka_new(KA_BLOCK);

  for (KaNode *last = node->children = chain; last;
      last = last->next = va_arg(args, KaNode *));

  va_end(args);
  return node;
}

static inline KaNode *ka_get(char *key, KaNode **chain) {
  KaNode *node = *chain;
  while (node && strcmp(key, node->key ? node->key : "")) node = node->next;
  return node;
}

static inline KaNode *ka_def(char *key, KaNode *node, KaNode **chain) {
  free(node->key);
  node->key = strdup(key);
  node->next = *chain;
  return *chain = node;
}

static inline KaNode *ka_set(char *key, KaNode *data, KaNode **chain) {
  KaNode *node = ka_get(key, chain);
  if (!node) return ka_def(key, data, chain);

  node->type >= KA_LIST ? ka_free((KaNode *)node->value) : free(node->value);
  node->type = data->type;
  node->value = data->value;

  free(data->key);
  free(data->refcount);
  free(data);
  return node;
}

static inline void ka_del(char *key, KaNode **chain) {
  KaNode *prev = *chain;
  KaNode *node = *chain;

  while (node && strcmp(key, node->key ? node->key : "")) {
    prev = node;
    node = node->next;
  }

  if (!node) return;
  node == *chain ? (*chain = node->next) : (prev->next = node->next);
  node->next = NULL;
  ka_free(node);
}

static inline KaNode *ka_eval(KaNode *node, KaNode **env) {
  if (node->type == KA_BLOCK) return ka_eval((KaNode *)node->value, NULL);

  for (KaNode *current = node; current; current = current->next) {
    if (current->key) printf("%s\n", current->key); 
  }

  return node;
}

#endif
