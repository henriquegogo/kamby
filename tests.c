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
  KaNode *node = ka_chain(
      ka_new(KA_NUMBER),
      ka_chain(
        ka_new(KA_SYMBOL),
        ka_new(KA_SYMBOL), NULL),
      ka_new(KA_STRING), NULL);

  assert(node->type == KA_NUMBER);
  assert(node->next->type == KA_SYMBOL);
  assert(node->next->next->type == KA_SYMBOL);
  assert(node->next->next->next->type == KA_STRING);
  assert(node->next->next->next->next == NULL);

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
  assert(!strcmp(node->string, "Hello"));
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_symbol() {
  KaNode *node = ka_symbol("sum");

  assert(node->type == KA_SYMBOL);
  assert(!strcmp(node->key, "sum"));
  assert(node->value == NULL);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_list() {
  KaNode *node = ka_list(ka_number(42), ka_string("Hello"), NULL);

  assert(node->type == KA_LIST);
  assert(node->key == NULL);
  assert(node->children->type == KA_NUMBER);
  assert(node->children->next->type == KA_STRING);
  assert(node->children->next->next == NULL);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_expr() {
  KaNode *node = ka_expr(ka_symbol("num"), ka_symbol("="), ka_number(7), NULL);

  assert(node->type == KA_EXPR);
  assert(node->key == NULL);
  assert(node->children->type == KA_SYMBOL);
  assert(!strcmp(node->children->key, "num"));
  assert(node->children->next->type == KA_SYMBOL);
  assert(!strcmp(node->children->next->key, "="));
  assert(node->children->next->next->type == KA_NUMBER);
  assert(*node->children->next->next->number == 7);
  assert(node->children->next->next->next == NULL);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_block() {
  KaNode *node = ka_block(ka_symbol("num"), ka_symbol("="), ka_number(7), NULL);

  assert(node->type == KA_BLOCK);
  assert(node->key == NULL);
  assert(node->children->type == KA_SYMBOL);
  assert(!strcmp(node->children->key, "num"));
  assert(node->children->next->type == KA_SYMBOL);
  assert(!strcmp(node->children->next->key, "="));
  assert(node->children->next->next->type == KA_NUMBER);
  assert(*node->children->next->next->number == 7);
  assert(node->children->next->next->next == NULL);
  assert(*node->refcount == 0);
  assert(node->next == NULL);

  ka_free(node);
}

void test_copy() {
  KaNode *list = ka_list(ka_number(1), ka_number(2), ka_number(3), NULL);
  KaNode *first = list->children;
  KaNode *list_copy = ka_copy(list);
  KaNode *first_copy = ka_copy(first);
  KaNode *second_copy = ka_copy(list->children->next);
  KaNode *third_copy = ka_copy(list->children->next->next);

  assert(first_copy->type == first->type && first->type == KA_NUMBER);
  assert(!strcmp(first_copy->string, first->string));
  assert(*first_copy->refcount == *first->refcount && *first->refcount == 0);
  assert(first->next != NULL && first_copy->next == NULL);
  assert(!second_copy->next && !third_copy->next);

  assert(list_copy->type == list->type && list->type == KA_LIST);
  assert(*list_copy->refcount == *list->refcount && *list->refcount == 1);
  ka_free(list_copy);
  assert(*list->refcount == 0);

  ka_free(third_copy);
  ka_free(second_copy);
  ka_free(first_copy);
  ka_free(list);
}

void test_get() {
  KaNode *ctx = ka_new(KA_NONE);
  ka_def(ka_chain(ka_symbol("name"), ka_string("Hello"), NULL), &ctx);
  ka_def(ka_chain(ka_symbol("age"), ka_number(42), NULL), &ctx);

  assert(*ka_get(ka_symbol("age"), &ctx)->number == 42);
  assert(!strcmp(ka_get(ka_symbol("name"), &ctx)->string, "Hello"));
  assert(ka_get(ka_symbol("inexistent"), &ctx) == NULL);

  ka_free(ctx);
}

void test_def() {
  KaNode *ctx = ka_new(KA_NONE);
  ka_def(ka_chain(ka_symbol("name"), ka_string("Hello"), NULL), &ctx);
  ka_def(ka_chain(ka_symbol("age"), ka_number(42), NULL), &ctx);
  ka_def(ka_chain(ka_symbol("name"), ka_string("World"), NULL), &ctx);

  assert(!strcmp(ctx->key, "name"));
  assert(!strcmp(ctx->string, "World"));
  assert(!strcmp(ctx->next->key, "age"));
  assert(*ctx->next->number == 42);
  assert(!strcmp(ctx->next->next->key, "name"));
  assert(!strcmp(ctx->next->next->string, "Hello"));

  ka_free(ctx);
}

void test_set() {
  KaNode *ctx = ka_new(KA_NONE);
  ka_set(ka_chain(ka_symbol("name"),
      ka_list(ka_string("Hello"), ka_string("World"), NULL), NULL), &ctx);
  ka_set(ka_chain(ka_symbol("age"), ka_number(42), NULL), &ctx);
  ka_set(ka_chain(ka_symbol("name"), ka_string("Foo"), NULL), &ctx);

  assert(!strcmp(ctx->key, "age"));
  assert(*ctx->number == 42);
  assert(ctx->next->type == KA_STRING);
  assert(!strcmp(ctx->next->key, "name"));
  assert(!strcmp(ctx->next->string, "Foo"));

  ka_free(ctx);
}

void test_del() {
  KaNode *ctx = ka_new(KA_NONE);
  ka_def(ka_chain(ka_symbol("name"), ka_string("Hello"), NULL), &ctx);
  ka_def(ka_chain(ka_symbol("age"), ka_number(42), NULL), &ctx);
  ka_def(ka_chain(ka_symbol("message"), ka_string("Foo"), NULL), &ctx);

  ka_del(ka_symbol("message"), &ctx);
  assert(!strcmp(ctx->key, "age"));
  assert(!strcmp(ctx->next->key, "name"));

  ka_del(ka_symbol("name"), &ctx);
  assert(ctx->next->type == KA_NONE);

  ka_free(ctx);
}

int main() {
  printf("\nStarting tests...\n");

  test_new();
  test_chain();
  test_number();
  test_string();
  test_symbol();
  //test_function();
  test_copy();
  test_list();
  test_expr();
  test_block();
  test_get();
  test_def();
  test_set();
  test_del();
  //test_eval();

  printf("Done!\n\n");
  return 0;
}
