#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  KA_NONE, KA_NUMBER, KA_STRING, KA_SYMBOL, KA_EXPR, KA_BLOCK, KA_LIST
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

KaNode *ka_new(KaType type) {
  KaNode *node = calloc(1, sizeof(KaNode));
  node->type = type;
  node->refcount = calloc(1, sizeof(int));
  return node;
}

void ka_free(KaNode *node) {
  while (node) {
    KaNode *next = node->next;

    if ((*node->refcount)-- <= 0) {
      node->type == KA_LIST ? ka_free(node->value) : free(node->value);
      free(node->refcount);
    }

    free(node->key);
    free(node);
    node = next;
  }
}

KaNode *ka_number(long double value) {
  KaNode *node = ka_new(KA_NUMBER);
  node->number = calloc(1, sizeof(long double));
  *node->number = value;
  return node;
}

KaNode *ka_string(char *value) {
  KaNode *node = ka_new(KA_STRING);
  node->string = strdup(value);
  return node;
}

KaNode *ka_symbol(char *key) {
  KaNode *node = ka_new(KA_SYMBOL);
  node->key = strdup(key);
  return node;
}

KaNode *ka_copy(KaNode *node) {
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

KaNode *ka_list(KaNode *chain, ...) {
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

KaNode *ka_chain(KaNode *chain, ...) {
  va_list args;
  va_start(args, chain);

  KaNode *last = chain ? chain : va_arg(args, KaNode *);
  while (last) last = last->next = va_arg(args, KaNode *);

  va_end(args);
  return chain;
}

KaNode *ka_key(char *key, KaNode *node) {
  free(node->key);
  node->key = strdup(key);
  return node;
}

KaNode *ka_get(char *key, KaNode **chain) {
  KaNode *item = *chain;
  while (item && strcmp(key, item->key ? item->key : "") != 0) {
    item = item->next;
  }
  return item;
}

KaNode *ka_def(char *key, KaNode *node, KaNode **chain) {
  free(node->key);
  node->key = strdup(key);
  node->next = *chain;
  *chain = node;
  return node;
}

KaNode *ka_set(char *key, KaNode *node, KaNode **chain) {
  KaNode *item = ka_get(key, chain);
  if (!item) return ka_def(key, node, chain);
  item->type == KA_LIST ? ka_free(item->value) : free(item->value);
  item->type = node->type;
  item->value = node->value;
  free(node->key);
  free(node->refcount);
  free(node);
  return item;
}

KaNode *ka_del(char *key, KaNode **chain) {
  KaNode *prev = *chain, *item = *chain;
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

int print_level = 0;
void print_node(KaNode *node) {
  if (!node) return;
  for (int i = 0; i < print_level; i++) printf("  ");
  switch (node->type) {
    case KA_NUMBER:
      printf("(ref %d) number %s: %.Lf\n", *node->refcount, node->key ? node->key : "", *node->number);
      break;
    case KA_STRING:
      printf("(ref %d) string %s: %s\n", *node->refcount, node->key ? node->key : "", node->string);
      break;
    case KA_SYMBOL:
      printf("(ref %d) symbol %s\n", *node->refcount, node->key);
      break;
    case KA_LIST:
      printf("(ref %d) list %s:\n", *node->refcount, node->key);
      print_level++;
      KaNode *child = node->children;
      while (child) {
        print_node(child);
        child = child->next;
      }
      print_level--;
      break;
    default:;
  }
}

void print_chain(KaNode *chain) {
  KaNode *current = chain;
  while (current) {
    print_node(current);
    current = current->next;
  }
}

int main() {
  KaNode *env = ka_new(KA_NONE);
  ka_def("name", ka_string("Henrique"), &env);
  ka_def("age", ka_number(40), &env);

  ka_set("newvar", ka_string("This is not a value"), &env);

  ka_def("seeds", ka_list(
    ka_string("Wheat"),
    ka_string("Rye"),
    ka_string("Barley"), NULL), &env);

  ka_def("fruits", ka_list(
    ka_string("Apple"),
    ka_string("Banana"),
    ka_number(22),
    ka_get("seeds", &env),
    ka_get("name", &env),
    ka_string("Grape"), NULL), &env);

  ka_set("name", ka_string("Mr Soarrs"), &env);

  print_chain(env);
  printf("\n");
  
  KaNode *dupfruit = ka_copy(ka_get("fruits", &env));
  print_node(dupfruit);
  ka_free(dupfruit);
  
  ka_free(env);
  printf("\n");

  return 0;
}
