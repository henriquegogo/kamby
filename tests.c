#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "kamby.h"

int print_level = 0;
void print_node(KaNode *node) {
  if (!node) return;
  KaNode *child;
  for (int i = 0; i < print_level; i++) printf("  ");
  switch (node->type) {
    case KA_NUMBER:
      printf("number %s: %.2Lf\n", node->key ? node->key : "", *node->number);
      break;
    case KA_STRING:
      printf("string %s: %s\n", node->key ? node->key : "", node->string);
      break;
    case KA_SYMBOL:
      printf("symbol : %s\n", node->symbol);
      break;
    case KA_FUNC:
      printf("func %s: %p\n", node->key, node->func);
      break;
    case KA_LIST:
      printf("list %s:\n", node->key);
      print_level++;
      child = node->children;
      while (child) { print_node(child); child = child->next; }
      print_level--;
      break;
    case KA_EXPR:
      printf("expr %s:\n", node->key);
      print_level++;
      child = node->children;
      while (child) { print_node(child); child = child->next; }
      print_level--;
      break;
    case KA_BLOCK:
      printf("block %s:\n", node->key);
      print_level++;
      child = node->children;
      while (child) { print_node(child); child = child->next; }
      print_level--;
      break;
    case KA_NONE:
      printf("none %s\n", node->key);
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

void test_new() {
  KaNode *node = ka_new(KA_NONE);

  assert(node->type == KA_NONE);
  assert(node->key == NULL);
  assert(node->value == NULL);
  assert(node->next == NULL);

  ka_free(node);
}

void test_chain() {
  KaNode *node = ka_chain(
      ka_new(KA_NUMBER),
      ka_chain(
        ka_new(KA_SYMBOL),
        ka_new(KA_SYMBOL), NULL),
      ka_chain(
        ka_new(KA_NUMBER),
        ka_new(KA_STRING), NULL),
      ka_new(KA_SYMBOL), NULL);

  assert(node->type == KA_NUMBER);
  assert(node->next->type == KA_SYMBOL);
  assert(node->next->next->type == KA_SYMBOL);
  assert(node->next->next->next->type == KA_NUMBER);
  assert(node->next->next->next->next->type == KA_STRING);
  assert(node->next->next->next->next->next->type == KA_SYMBOL);
  assert(node->next->next->next->next->next->next == NULL);

  ka_free(node);
}

void test_ctx() {
  KaNode *node = ka_ctx();

  assert(node->type == KA_CTX);
  assert(!strcmp(node->key, ""));
  assert(node->value == NULL);
  assert(node->next == NULL);

  ka_free(node);
}

void test_number() {
  KaNode *node = ka_number(42);

  assert(node->type == KA_NUMBER);
  assert(node->key == NULL);
  assert(*node->number == 42);
  assert(node->next == NULL);

  ka_free(node);
}

void test_string() {
  KaNode *node = ka_string("Hello");

  assert(node->type == KA_STRING);
  assert(node->key == NULL);
  assert(!strcmp(node->string, "Hello"));
  assert(node->next == NULL);

  ka_free(node);
}

void test_symbol() {
  KaNode *node = ka_symbol("sum");

  assert(node->type == KA_SYMBOL);
  assert(node->key == NULL);
  assert(!strcmp(node->symbol, "sum"));
  assert(node->next == NULL);

  ka_free(node);
}

void test_func() {
  KaNode *node = ka_func(ka_def);

  assert(node->type == KA_FUNC);
  assert(node->key == NULL);
  assert(node->func == ka_def);
  assert(node->next == NULL);

  ka_free(node);
}

void test_copy() {
  KaNode *list = ka_list(ka_number(1), ka_string("a"), ka_symbol("b"), NULL);
  KaNode *first = list->children;
  KaNode *list_copy = ka_copy(list);
  KaNode *first_copy = ka_copy(first);
  KaNode *second_copy = ka_copy(list->children->next);
  KaNode *third_copy = ka_copy(list->children->next->next);

  assert(first_copy->type == first->type && first->type == KA_NUMBER);
  assert(*first_copy->number == *first->number);
  assert(first->next != NULL && first_copy->next == NULL);
  assert(!second_copy->next && !third_copy->next);

  assert(list_copy->type == list->type && list->type == KA_LIST);
  assert(list_copy->children != first_copy);
  assert(*list_copy->children->number == *first_copy->number);
  assert(list_copy->children->next != second_copy);
  assert(!strcmp(list_copy->children->next->string, second_copy->string));
  assert(list_copy->children->next->next != third_copy);
  assert(list_copy->children->next->next->type == third_copy->type);
  assert(third_copy->type == KA_SYMBOL);
  assert(!strcmp(list_copy->children->next->next->string, third_copy->string));
  assert(!list_copy->children->next->next->next);

  ka_free(list_copy);
  ka_free(third_copy), ka_free(second_copy), ka_free(first_copy);
  ka_free(list);
}

void test_list() {
  KaNode *node = ka_list(ka_number(42), ka_string("Hello"), NULL);

  assert(node->type == KA_LIST);
  assert(node->key == NULL);
  assert(node->children->type == KA_NUMBER);
  assert(node->children->next->type == KA_STRING);
  assert(node->children->next->next == NULL);
  assert(node->next == NULL);

  ka_free(node);
}

void test_expr() {
  KaNode *node = ka_expr(ka_symbol("num"), ka_symbol("="), ka_number(7), NULL);

  assert(node->type == KA_EXPR);
  assert(node->key == NULL);
  assert(node->children->type == KA_SYMBOL);
  assert(!strcmp(node->children->symbol, "num"));
  assert(node->children->next->type == KA_SYMBOL);
  assert(!strcmp(node->children->next->symbol, "="));
  assert(node->children->next->next->type == KA_NUMBER);
  assert(*node->children->next->next->number == 7);
  assert(node->children->next->next->next == NULL);
  assert(node->next == NULL);

  ka_free(node);
}

void test_block() {
  KaNode *node = ka_block(ka_symbol("num"), ka_symbol("="), ka_number(7), NULL);

  assert(node->type == KA_BLOCK);
  assert(node->key == NULL);
  assert(node->children->type == KA_SYMBOL);
  assert(!strcmp(node->children->symbol, "num"));
  assert(node->children->next->type == KA_SYMBOL);
  assert(!strcmp(node->children->next->symbol, "="));
  assert(node->children->next->next->type == KA_NUMBER);
  assert(*node->children->next->next->number == 7);
  assert(node->children->next->next->next == NULL);
  assert(node->next == NULL);

  ka_free(node);
}

void test_get() {
  KaNode *ctx = ka_ctx(), *result;
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));

  assert(*(result = ka_get(&ctx, ka_symbol("age")))->number == 42);
  ka_free(result);
  assert(!strcmp((result = ka_get(&ctx, ka_symbol("name")))->string, "John"));
  ka_free(result);
  assert(ka_get(&ctx, ka_symbol("inexistent")) == NULL);

  ka_free(ctx);
}

void test_def() {
  KaNode *ctx = ka_ctx();
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("Doe"), NULL)));

  assert(!strcmp(ctx->key, "name"));
  assert(!strcmp(ctx->string, "Doe"));
  assert(!strcmp(ctx->next->key, "age"));
  assert(*ctx->next->number == 42);
  assert(!strcmp(ctx->next->next->key, "name"));
  assert(!strcmp(ctx->next->next->string, "John"));

  ka_free(ctx);
}

void test_set() {
  KaNode *ctx = ka_ctx();
  ka_free(ka_set(&ctx, ka_chain(ka_symbol("name"),
      ka_list(ka_string("John"), ka_string("Doe"), NULL), NULL)));
  ka_free(ka_set(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
  ka_free(ka_set(&ctx, ka_chain(ka_symbol("name"), ka_string("Foo"), NULL)));

  assert(!strcmp(ctx->key, "age"));
  assert(*ctx->number == 42);
  assert(ctx->next->type == KA_STRING);
  assert(!strcmp(ctx->next->key, "name"));
  assert(!strcmp(ctx->next->string, "Foo"));

  ka_free(ctx);
}

void test_del() {
  KaNode *ctx = ka_ctx();
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("message"), ka_string("Foo"), NULL)));

  ka_del(&ctx, ka_symbol("message"));
  assert(!strcmp(ctx->key, "age"));
  assert(!strcmp(ctx->next->key, "name"));

  ka_del(&ctx, ka_symbol("name"));
  assert(ctx->next->type == KA_CTX);

  ka_free(ctx);
}

void test_eval() {
  KaNode *ctx = ka_ctx(), *expr, *result;
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("def"), ka_func(ka_def), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("add"), ka_func(ka_add), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("lt"), ka_func(ka_lt), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("i"), ka_number(5), NULL)));

  // Define a variable into the context
  expr = ka_expr(ka_symbol("def"), ka_symbol("name"), ka_string("John"), NULL);
  result = ka_eval(&ctx, expr);
  KaNode *var = ka_get(&ctx, ka_symbol("name"));
  assert(!strcmp(var->string, result->string));
  assert(!strcmp(result->string, "John"));
  ka_free(var), ka_free(result), ka_free(expr);

  // Define and recover a variable inside a block context
  expr = ka_expr(ka_block(
        ka_expr(ka_symbol("def"), ka_symbol("age"), ka_number(42), NULL),
        ka_expr(ka_symbol("def"), ka_symbol("name"), ka_string("Doe"), NULL),
        ka_expr(ka_symbol("name"), NULL), NULL), NULL);
  result = ka_eval(&ctx, expr);
  assert(!strcmp(result->string, "Doe"));
  ka_free(result), ka_free(expr);

  // Recover a global variable using a new block
  expr = ka_expr(ka_symbol("name"), NULL);
  result = ka_eval(&ctx, expr);
  assert(!strcmp(result->string, "John"));
  ka_free(result), ka_free(expr);

  // Use other kind of functions
  expr = ka_chain(ka_symbol("lt"), ka_number(5), ka_number(10), NULL);
  result = ka_eval(&ctx, expr);
  assert(result->type == KA_TRUE);
  ka_free(result), ka_free(expr);

  expr = ka_chain(ka_symbol("lt"), ka_symbol("i"), ka_number(10), NULL);
  result = ka_eval(&ctx, expr);
  assert(result->type == KA_TRUE);
  ka_free(result), ka_free(expr);

  expr = ka_chain(ka_symbol("add"), ka_number(5), ka_number(10), NULL);
  result = ka_eval(&ctx, expr); // Memory leak
  assert(*result->number == 15);
  ka_free(result), ka_free(expr);

  ka_free(ctx);
}

void test_parser() {
  KaNode *result;
  int pos;

  result = ka_parser("# This is a comment", (pos = 0, &pos));
  assert(!result->children);
  ka_free(result);

  result = ka_parser("42.21 # This is a comment", (pos = 0, &pos));
  assert(fabsl(*result->children->number - 42.21) < 1e-10);
  assert(!result->children->next);
  ka_free(result);

  result = ka_parser("42.21 age # This is a comment", (pos = 0, &pos));
  assert(fabsl(*result->children->number - 42.21) < 1e-10);
  assert(!strcmp(result->children->next->symbol, "age"));
  assert(!result->children->next->next);
  ka_free(result);

  result = ka_parser("age\n# This is a comment\n42", (pos = 0, &pos));
  assert(!strcmp(result->children->symbol, "age"));
  assert(*result->children->next->number == 42);
  ka_free(result);

  result = ka_parser("'It\\'s John Doe. Backslash: \\\\ OK'", (pos = 0, &pos));
  assert(!strcmp(result->children->string, "It\'s John Doe. Backslash: \\ OK"));
  ka_free(result);

  result = ka_parser("age;42 'John Doe' 21;name", (pos = 0, &pos));
  assert(!strcmp(result->children->symbol, "age"));
  assert(*result->next->children->number == 42);
  assert(!strcmp(result->next->children->next->string, "John Doe"));
  assert(*result->next->children->next->next->number == 21);
  assert(!strcmp(result->next->next->children->symbol, "name"));
  ka_free(result);

  result = ka_parser("42 'John Doe' name (22) {71} [1 2]", (pos = 0, &pos));
  KaNode *number = result->children, *string = result->children->next,
         *symbol = result->children->next->next,
         *expr = result->children->next->next->next,
         *block = result->children->next->next->next->next,
         *list = result->children->next->next->next->next->next;
  
  assert(number->type == KA_NUMBER && *number->number == 42);
  assert(string->type == KA_STRING && !strcmp(string->string, "John Doe"));
  assert(symbol->type == KA_SYMBOL && !strcmp(symbol->symbol, "name"));
  assert(expr->type == KA_EXPR && *expr->children->number == 22);
  assert(block->type == KA_BLOCK && *block->children->number == 71);
  assert(list->type == KA_LIST);
  assert(*list->children->number == 1 && *list->children->next->number == 2);
  ka_free(result);
}

void test_logical() {
  KaNode *result;

  result = ka_and(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(*result->number == 2); ka_free(result);
  result = ka_and(NULL, ka_chain(ka_number(1), ka_new(KA_NONE), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_and(NULL, ka_chain(ka_new(KA_NONE), ka_new(KA_NONE), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_or(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(*result->number == 1); ka_free(result);
  result = ka_or(NULL, ka_chain(ka_new(KA_NONE), ka_number(2), NULL));
  assert(*result->number == 2); ka_free(result);
  result = ka_or(NULL, ka_chain(ka_new(KA_NONE), ka_new(KA_NONE), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_not(NULL, ka_number(1));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_not(NULL, NULL);
  assert(result->type == KA_TRUE); ka_free(result);
}

void test_comparison() {
  KaNode *result;

  result = ka_eq(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_eq(NULL, ka_chain(ka_new(KA_NONE), ka_number(2), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_eq(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_eq(NULL, ka_chain(ka_string("Hello"), ka_string("World"), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_eq(NULL, ka_chain(ka_string("Hello"), ka_string("Hello"), NULL));
  assert(result->type == KA_TRUE); ka_free(result);

  result = ka_neq(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_neq(NULL, ka_chain(ka_new(KA_NONE), ka_number(2), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_neq(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_neq(NULL, ka_chain(ka_string("Hello"), ka_string("World"), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_neq(NULL, ka_chain(ka_string("Hello"), ka_string("Hello"), NULL));
  assert(result->type == KA_FALSE); ka_free(result);

  result = ka_gt(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_gt(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_gt(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
  assert(result->type == KA_FALSE); ka_free(result);

  result = ka_lt(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_lt(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_lt(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
  assert(result->type == KA_FALSE); ka_free(result);

  result = ka_gte(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_gte(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_gte(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
  assert(result->type == KA_TRUE); ka_free(result);

  result = ka_lte(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
  assert(result->type == KA_FALSE); ka_free(result);
  result = ka_lte(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
  result = ka_lte(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
  assert(result->type == KA_TRUE); ka_free(result);
}

void test_arithmetic() {
  KaNode *result;

  result = ka_add(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
  assert(*result->number == 5); ka_free(result);
  result = ka_sub(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
  assert(*result->number == 1); ka_free(result);
  result = ka_mul(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
  assert(*result->number == 6); ka_free(result);
  result = ka_div(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
  assert(*result->number == 1.5); ka_free(result);
  result = ka_mod(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
  assert(*result->number == 1); ka_free(result);

  result = ka_add(NULL, ka_chain(ka_number(2), ka_string("Hello"), NULL));
  assert(result == NULL); ka_free(result);

  result = ka_add(NULL, ka_chain(ka_string("Hello"), ka_string("World"), NULL));
  assert(!strcmp(result->string, "HelloWorld")); ka_free(result);
}

void test_conditional() {
  KaNode *ctx = ka_ctx();
  KaNode *block = ka_block(ka_number(42), NULL);
  KaNode *else_block = ka_block(ka_number(27), NULL);
  KaNode *result;

  result = ka_if(&ctx, ka_chain(
        ka_lt(NULL, ka_chain(ka_number(1), ka_number(2), NULL)),
        ka_copy(block), ka_copy(else_block), NULL));
  assert(*result->number == 42);
  ka_free(result);

  result = ka_if(&ctx, ka_chain(
        ka_gt(NULL, ka_chain(ka_number(1), ka_number(2), NULL)),
        ka_copy(block), ka_copy(else_block), NULL));
  assert(*result->number == 27);
  ka_free(result);

  result = ka_if(&ctx, ka_chain(
        ka_gt(NULL, ka_chain(ka_number(1), ka_number(2), NULL)),
        ka_copy(block), NULL));
  assert(result == NULL);
  ka_free(result);

  ka_free(else_block), ka_free(block), ka_free(ctx);
}

void test_loop() {
  KaNode *ctx = ka_ctx();
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("i"), ka_number(0), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("lt"), ka_func(ka_lt), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("set"), ka_func(ka_set), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("add"), ka_func(ka_add), NULL)));

  KaNode *condition = ka_block(
      ka_symbol("lt"), ka_symbol("i"), ka_number(10), NULL);
  
  KaNode *block = ka_block(
      ka_symbol("set"), ka_symbol("i"),
      ka_expr(ka_symbol("add"), ka_symbol("i"), ka_number(1), NULL), NULL);

  ka_loop(&ctx, ka_chain(condition, block, NULL));
  KaNode *var = ka_get(&ctx, ka_symbol("i"));
  assert(*var->number == 10); ka_free(var);

  ka_free(ctx);
}

int main() {
  test_new();
  test_chain();
  test_ctx();
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
  test_parser();
  test_logical();
  test_comparison();
  test_arithmetic();
  test_conditional();
  test_loop();

  printf("\nAll tests passed!\n\n");
  return 0;
}
