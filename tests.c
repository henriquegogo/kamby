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

void test_first() {
  KaNode *node = ka_new(KA_NONE);
  KaNode *next = node->next = ka_new(KA_NONE);

  assert(node);
  assert(node->next);
  assert(!ka_first(node)->next);
  assert(ka_first(node));

  ka_free(next);
  ka_free(node);
}

void test_last() {
  KaNode *node = ka_new(KA_NONE);
  node->next = ka_new(KA_NONE);
  node->next->next = ka_new(KA_NONE);

  assert(node);
  assert(node->next);
  assert(ka_last(node) == node->next->next);

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

void test_func() {
  KaNode *node = ka_func(ka_def);

  assert(node->type == KA_FUNC);
  assert(node->key == NULL);
  assert(node->func == ka_def);
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

void test_get() {
  KaNode *ctx = ka_new(KA_CTX);
  ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL));

  assert(*ka_get(&ctx, ka_symbol("age"))->number == 42);
  assert(!strcmp(ka_get(&ctx, ka_symbol("name"))->string, "John"));
  assert(ka_get(&ctx, ka_symbol("inexistent")) == NULL);

  ka_free(ctx);
}

void test_def() {
  KaNode *ctx = ka_new(KA_CTX);
  ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("Doe"), NULL));

  assert(!strcmp(ctx->key, "name"));
  assert(!strcmp(ctx->string, "Doe"));
  assert(!strcmp(ctx->next->key, "age"));
  assert(*ctx->next->number == 42);
  assert(!strcmp(ctx->next->next->key, "name"));
  assert(!strcmp(ctx->next->next->string, "John"));

  ka_free(ctx);
}

void test_set() {
  KaNode *ctx = ka_new(KA_CTX);
  ka_set(&ctx, ka_chain(ka_symbol("name"),
      ka_list(ka_string("John"), ka_string("Doe"), NULL), NULL));
  ka_set(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL));
  ka_set(&ctx, ka_chain(ka_symbol("name"), ka_string("Foo"), NULL));

  assert(!strcmp(ctx->key, "age"));
  assert(*ctx->number == 42);
  assert(ctx->next->type == KA_STRING);
  assert(!strcmp(ctx->next->key, "name"));
  assert(!strcmp(ctx->next->string, "Foo"));

  ka_free(ctx);
}

void test_del() {
  KaNode *ctx = ka_new(KA_CTX);
  ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("message"), ka_string("Foo"), NULL));

  ka_del(&ctx, ka_symbol("message"));
  assert(!strcmp(ctx->key, "age"));
  assert(!strcmp(ctx->next->key, "name"));

  ka_del(&ctx, ka_symbol("name"));
  assert(ctx->next->type == KA_CTX);

  ka_free(ctx);
}

void test_eval() {
  KaNode *ctx = ka_new(KA_CTX), *expr, *result;
  ka_def(&ctx, ka_chain(ka_symbol("def"), ka_func(ka_def), NULL));

  // Define a variable into the context
  expr = ka_expr(ka_symbol("def"), ka_symbol("name"), ka_string("John"), NULL);
  ka_free(ka_eval(&ctx, expr));
  ka_free(expr);
 
  // Define and recover a variable inside a block a block context
  // WIP: This test probably works but it's not the best way to do it
  expr = ka_expr(ka_block(
        ka_expr(ka_symbol("def"), ka_symbol("age"), ka_number(42), NULL),
        ka_expr(ka_symbol("def"), ka_symbol("name"), ka_string("Doe"), NULL),
        ka_expr(ka_symbol("name"), NULL), NULL), NULL);
  result = ka_eval(&ctx, expr);
  assert(!strcmp(result->string, "Doe"));

  ka_free(expr);
  ka_free(result);

  // Recover a global variable using a new block
  expr = ka_expr(ka_symbol("name"), NULL);
  result = ka_eval(&ctx, expr);
  assert(!strcmp(result->string, "John"));

  ka_free(expr);
  ka_free(result);
  ka_free(ctx);
}

void test_logical() {
  KaNode *num1 = ka_number(1);
  KaNode *num2 = ka_number(2);
  KaNode *numeq1 = ka_number(1);
  KaNode *none = ka_new(KA_NONE);
  KaNode *none2 = ka_new(KA_NONE);

  assert(ka_and(NULL, ka_chain(num1, num2, NULL)) == num2);
  assert(ka_and(NULL, ka_chain(num1, none, NULL)) == NULL);
  assert(ka_and(NULL, ka_chain(none, num2, NULL)) == NULL);
  assert(ka_and(NULL, ka_chain(none, none2, NULL)) == NULL);

  assert(ka_or(NULL, ka_chain(num1, num2, NULL)) == num1);
  assert(ka_or(NULL, ka_chain(none, num2, NULL)) == num2);
  assert(ka_or(NULL, ka_chain(none, none2, NULL)) == NULL);

  assert(ka_not(NULL, num1) == NULL);
  assert(ka_not(NULL, NULL) == &KA_TRUE);

  ka_free(none2);
  ka_free(none);
  ka_free(numeq1);
  ka_free(num2);
  ka_free(num1);
}

void test_comparison() {
  KaNode *num1 = ka_number(1);
  KaNode *num2 = ka_number(2);
  KaNode *numeq1 = ka_number(1);
  KaNode *str1 = ka_string("Hello");
  KaNode *str2 = ka_string("World");
  KaNode *str3 = ka_string("Hello");
  KaNode *none = ka_new(KA_NONE);

  assert(ka_eq(NULL, ka_chain(num1, num2, NULL)) == NULL);
  assert(ka_eq(NULL, ka_chain(none, num2, NULL)) == NULL);
  assert(ka_eq(NULL, ka_chain(num1, numeq1, NULL)) == &KA_TRUE);
  assert(ka_eq(NULL, ka_chain(str1, str2, NULL)) == NULL);
  assert(ka_eq(NULL, ka_chain(str1, str3, NULL)) == &KA_TRUE);

  assert(ka_neq(NULL, ka_chain(num1, num2, NULL)) == &KA_TRUE);
  assert(ka_neq(NULL, ka_chain(none, num2, NULL)) == &KA_TRUE);
  assert(ka_neq(NULL, ka_chain(num1, numeq1, NULL)) == NULL);
  assert(ka_neq(NULL, ka_chain(str1, str2, NULL)) == &KA_TRUE);
  assert(ka_neq(NULL, ka_chain(str1, str3, NULL)) == NULL);

  assert(ka_gt(NULL, ka_chain(num2, num1, NULL)) == &KA_TRUE);
  assert(ka_gt(NULL, ka_chain(num1, num2, NULL)) == NULL);
  assert(ka_gt(NULL, ka_chain(num1, numeq1, NULL)) == NULL);

  assert(ka_lt(NULL, ka_chain(num2, num1, NULL)) == NULL);
  assert(ka_lt(NULL, ka_chain(num1, num2, NULL)) == &KA_TRUE);
  assert(ka_lt(NULL, ka_chain(num1, numeq1, NULL)) == NULL);

  assert(ka_gte(NULL, ka_chain(num2, num1, NULL)) == &KA_TRUE);
  assert(ka_gte(NULL, ka_chain(num1, num2, NULL)) == NULL);
  assert(ka_gte(NULL, ka_chain(num1, numeq1, NULL)) == &KA_TRUE);

  assert(ka_lte(NULL, ka_chain(num2, num1, NULL)) == NULL);
  assert(ka_lte(NULL, ka_chain(num1, num2, NULL)) == &KA_TRUE);
  assert(ka_lte(NULL, ka_chain(num1, numeq1, NULL)) == &KA_TRUE);

  ka_free(none);
  ka_free(str3);
  ka_free(str2);
  ka_free(str1);
  ka_free(numeq1);
  ka_free(num2);
  ka_free(num1);
}

void test_arithmetic() {
  KaNode *num2 = ka_number(2);
  KaNode *num3 = ka_number(3);
  KaNode *str1 = ka_string("Hello");
  KaNode *str2 = ka_string("World");
  KaNode *list1 = ka_list(ka_number(1), ka_number(2), NULL);
  KaNode *list2 = ka_list(ka_string("a"), ka_string("b"), NULL);
  KaNode *result;

  assert(*(result = ka_add(NULL, ka_chain(num3, num2, NULL)))->number == 5);
  ka_free(result);
  assert(*(result = ka_sub(NULL, ka_chain(num3, num2, NULL)))->number == 1);
  ka_free(result);
  assert(*(result = ka_mul(NULL, ka_chain(num3, num2, NULL)))->number == 6);
  ka_free(result);
  assert(*(result = ka_div(NULL, ka_chain(num3, num2, NULL)))->number == 1.5);
  ka_free(result);
  assert(*(result = ka_mod(NULL, ka_chain(num3, num2, NULL)))->number == 1);
  ka_free(result);

  assert((result = ka_add(NULL, ka_chain(num2, str1, NULL))) == NULL);
  ka_free(result);

  result = ka_add(NULL, ka_chain(str1, str2, NULL));
  assert(!strcmp(result->string, "HelloWorld"));
  ka_free(result);

  //result = ka_add(NULL, ka_chain(list1, list2, NULL));
  //ka_free(result);

  ka_free(list2);
  ka_free(list1);
  ka_free(str2);
  ka_free(str1);
  ka_free(num3);
  ka_free(num2);
}

void test_conditional() {
}

void test_loop() {
}

int main() {
  printf("\nStarting tests...\n");

  test_new();
  test_first();
  test_last();
  test_chain();
  test_number();
  test_string();
  test_symbol();
  test_func();
  test_copy();
  test_list();
  test_expr();
  test_block();
  test_get();
  test_def();
  test_set();
  test_del();
  test_eval();
  test_logical();
  test_comparison();
  test_arithmetic();
  test_conditional();
  test_loop();

  printf("Done!\n\n");
  return 0;
}
