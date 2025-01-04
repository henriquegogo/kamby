#ifndef KAMBY_H
#define KAMBY_H

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_NUMBER, KA_STRING, KA_SYMBOL, KA_LIST, KA_BLOCK
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
  while (node) {
    KaNode *next = node->next;

    if ((*node->refcount)-- <= 0) {
      if (node->type == KA_LIST) ka_free((KaNode *)node->value);
      else free(node->value);
      free(node->refcount);
    }

    free(node->key);
    free(node);

    node = next;
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
    (node->type == KA_NUMBER) ? ka_number(*node->number) :
    (node->type == KA_STRING) ? ka_string(node->string) :
    (node->type == KA_SYMBOL) ? ka_symbol(node->key) :
    (node->type == KA_LIST) ? ka_new(KA_LIST) : ka_new(node->type);

  if (copy->type == KA_LIST) {
    copy->value = node->value;

    free(copy->refcount);
    copy->refcount = node->refcount;
    (*node->refcount)++;
  }

  copy->key = node->key ? strdup(node->key) : NULL;

  return copy;
}

static inline KaNode *ka_list(KaNode *chain, ...) {
  va_list args;
  va_start(args, chain);

  KaNode *last = chain ? chain : va_arg(args, KaNode *);
  
  while ((last->next = va_arg(args, KaNode *))) { 
    last = last->next->next ? last->next = ka_copy(last->next) : last->next;
  }

  KaNode *node = ka_new(KA_LIST);
  node->children = chain;

  va_end(args);

  return node;
}

static inline KaNode *ka_chain(KaNode *chain, ...) {
  va_list args;
  va_start(args, chain);

  KaNode *last = chain;
  
  while (last) {
    while (last->next) last = last->next;
    last = last->next = va_arg(args, KaNode *);
  }

  va_end(args);

  return chain;
}

static inline KaNode *ka_get(char *key, KaNode **chain) {
  KaNode *item = *chain;
  
  while (item && strcmp(key, item->key ? item->key : "") != 0) {
    item = item->next;
  }
  
  return item;
}

static inline KaNode *ka_def(char *key, KaNode *node, KaNode **chain) {
  free(node->key);
  
  node->key = strdup(key);
  node->next = *chain;
  *chain = node;
  
  return node;
}

static inline KaNode *ka_set(char *key, KaNode *node, KaNode **chain) {
  KaNode *item = ka_get(key, chain);
  if (!item) return ka_def(key, node, chain);

  item->type == KA_LIST ? ka_free((KaNode *)item->value) : free(item->value);
  item->type = node->type;
  item->value = node->value;

  free(node->key);
  free(node->refcount);
  free(node);
  
  return item;
}

static inline KaNode *ka_del(char *key, KaNode **chain) {
  KaNode *prev = *chain;
  KaNode *item = *chain;

  while (item && strcmp(key, item->key ? item->key : "") != 0) {
    prev = item;
    item = item->next;
  }

  if (!item) return *chain;
  if (item == *chain) *chain = item->next;
  else prev->next = item->next;
  
  item->next = NULL;
  ka_free(item);
  
  return *chain;
}

#endif
