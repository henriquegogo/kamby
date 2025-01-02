#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "kamby.h"

void test_new() {
  KaNode *node = ka_new(KA_NONE);

  assert(node->type == KA_NONE);
  assert(node->key == NULL);
  assert(node->value == NULL);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_chain() {
  KaNode *node = ka_chain(ka_new(KA_NUMBER), ka_new(KA_STRING), NULL);

  assert(node->type == KA_NUMBER);
  assert(node->next->type == KA_STRING);
  assert(node->next->next == NULL);

  ka_free(node);
}

void test_number() {
  KaNode *node = ka_number(42);

  assert(node->type == KA_NUMBER);
  assert(node->key == NULL);
  assert(*node->number == 42);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_string() {
  KaNode *node = ka_string("Hello");

  assert(node->type == KA_STRING);
  assert(node->key == NULL);
  assert(strcmp(node->string, "Hello") == 0);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_symbol() {
  KaNode *node = ka_symbol("sum");

  assert(node->type == KA_SYMBOL);
  assert(strcmp(node->key, "sum") == 0);
  assert(node->value == NULL);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_list() {
  KaNode *node = ka_list(ka_new(KA_NUMBER), ka_new(KA_STRING), NULL);

  assert(node->type == KA_LIST);
  assert(node->key == NULL);
  assert(node->children->type == KA_NUMBER);
  assert(node->children->next->type == KA_STRING);
  assert(node->children->next->next == NULL);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_copy() {
  KaNode *list = ka_list(ka_string("Hello"), ka_string("World"), NULL);
  KaNode *child = list->children;
  KaNode *list_copy = ka_copy(list);
  KaNode *child_copy = ka_copy(child);

  assert(child_copy->type == child->type && child->type == KA_STRING);
  assert(strcmp(child_copy->string, child->string) == 0);
  assert(*child_copy->refcount == *child->refcount && *child->refcount == 0);
  assert(child->next != NULL && child_copy->next == NULL);

  assert(list_copy->type == list->type && list->type == KA_LIST);
  assert(*list_copy->refcount == *list->refcount && *list->refcount == 1);
  ka_free(list_copy);
  assert(*list->refcount == 0);

  ka_free(child_copy);
  ka_free(list);
}

int main() {
  printf("\nStarting tests...\n");

  test_new();
  test_chain();
  test_number();
  test_string();
  test_symbol();
  test_list();
  test_copy();

  printf("Done!\n\n");
  return 0;
}
