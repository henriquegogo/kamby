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
      printf("number %s: %.2Lf\n", node->key ?: "", *node->number);
      break;
    case KA_STRING:
      printf("string %s: %s\n", node->key ?: "", node->string);
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
    case KA_CTX:
      printf("ctx %s\n", node->key);
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
  KaNode *node = ka_new(KA_CTX);

  assert(node->type == KA_CTX);
  assert(node->key == NULL);
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

void test_ref() {
  KaNode *ctx = ka_new(KA_CTX);
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("key"), ka_string("name"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));

  assert(*(ka_ref(&ctx, ka_symbol("age")))->number == 42);
  assert(!strcmp(ka_ref(&ctx, ka_symbol("name"))->string, "John"));
  assert(!(ka_ref(&ctx, ka_symbol("inexistent"))));
  assert(ka_ref(&ctx, ka_symbol("$0"))->type == KA_NUMBER);
  assert(ka_ref(&ctx, ka_symbol("$1"))->type == KA_STRING);
  assert(ka_ref(&ctx, ka_symbol("$key"))->type == KA_STRING);
  assert(!strcmp(ka_ref(&ctx, ka_symbol("$key"))->string, "John"));

  ka_free(ctx);
}

void test_get() {
  KaNode *ctx = ka_new(KA_CTX), *result;
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));

  assert(*(result = ka_get(&ctx, ka_symbol("age")))->number == 42);
  ka_free(result);
  assert(!strcmp((result = ka_get(&ctx, ka_symbol("name")))->string, "John"));
  ka_free(result);
  assert((result = ka_get(&ctx, ka_symbol("inexistent")))->type == KA_NONE);
  ka_free(result);

  ka_free(ka_def(&ctx, ka_chain(ka_symbol("ctx"), ka_new(KA_CTX), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(78), NULL)));

  assert(*(ka_ref(&ctx, ka_symbol("age")))->number == 78);
  assert(*(ka_ref(&ctx, ka_symbol("$0")))->number == 78);
  assert(ka_ref(&ctx, ka_symbol("$1")) == NULL);

  ka_free(ka_del(&ctx, ka_symbol("ctx")));
  ka_free(ctx);
}

void test_key() {
  KaNode *ctx = ka_new(KA_CTX), *result;
  result = ka_key(&ctx, ka_chain(ka_symbol("two"), ka_number(2), NULL));

  assert(ctx->type == KA_CTX);
  assert(result->type == KA_NUMBER);
  assert(*result->number == 2);
  assert(!strcmp(result->key, "two"));

  ka_free(result);
  ka_free(ctx);
}

void test_def() {
  KaNode *ctx = ka_new(KA_CTX), *result;
  result = ka_def(&ctx, ka_chain(ka_symbol("block"), ka_block(NULL), NULL));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("Doe"), NULL)));

  assert(!strcmp(ctx->key, "name"));
  assert(!strcmp(ctx->string, "Doe"));
  assert(!strcmp(ctx->next->key, "age"));
  assert(*ctx->next->number == 42);
  assert(!strcmp(ctx->next->next->key, "name"));
  assert(!strcmp(ctx->next->next->string, "John"));
  assert(result->type == KA_NONE && ctx->next->next->next->type == KA_BLOCK);

  ka_free(result);
  ka_free(ctx);
}

void test_set() {
  KaNode *ctx = ka_new(KA_CTX), *result;
  ka_free(ka_set(&ctx, ka_chain(ka_symbol("block"), ka_string("NULL"), NULL)));
  result = ka_set(&ctx, ka_chain(ka_symbol("block"), ka_block(NULL), NULL));
  ka_free(ka_set(&ctx, ka_chain(ka_symbol("name"),
      ka_list(ka_string("John"), ka_string("Doe"), NULL), NULL)));
  ka_free(ka_set(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
  ka_free(ka_set(&ctx, ka_chain(ka_symbol("$1"), ka_string("Foo"), NULL)));

  assert(!strcmp(ctx->key, "age"));
  assert(*ctx->number == 42);
  assert(ctx->next->type == KA_STRING);
  assert(!strcmp(ctx->next->key, "name"));
  assert(!strcmp(ctx->next->string, "Foo"));
  assert(result->type == KA_NONE && ctx->next->next->type == KA_BLOCK);

  ka_free(result);
  ka_free(ctx);
}

void test_del() {
  KaNode *ctx = ka_new(KA_CTX);
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("message"), ka_string("Foo"), NULL)));

  ka_free(ka_del(&ctx, ka_symbol("unknown")));
  assert(!strcmp(ctx->key, "message"));
  assert(!strcmp(ctx->next->key, "age"));
  assert(!strcmp(ctx->next->next->key, "name"));

  ka_free(ka_del(&ctx, ka_symbol("message")));
  assert(!strcmp(ctx->key, "age"));
  assert(!strcmp(ctx->next->key, "name"));

  ka_free(ka_del(&ctx, ka_symbol("name")));
  assert(!strcmp(ctx->key, "age"));
  assert(ctx->next->type == KA_CTX);

  ka_free(ka_del(&ctx, ka_symbol("$0")));
  assert(ctx->type == KA_CTX);

  ka_free(ctx);
}

void test_eval() {
  KaNode *ctx = ka_new(KA_CTX), *expr, *result;
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

  result = ka_parser("42.21 age // This is a comment", (pos = 0, &pos));
  assert(fabsl(*result->children->number - 42.21) < 1e-10);
  assert(!strcmp(result->children->next->symbol, "age"));
  assert(!result->children->next->next);
  ka_free(result);

  result = ka_parser("age\n// This is a comment\n42", (pos = 0, &pos));
  assert(!strcmp(result->children->symbol, "age"));
  assert(*result->children->next->number == 42);
  ka_free(result);

  result = ka_parser("age\n/* Multiline\ncomment */\n42", (pos = 0, &pos));
  assert(!strcmp(result->children->symbol, "age"));
  assert(*result->children->next->number == 42);
  ka_free(result);

  result = ka_parser("'It\\'s John Doe. Backslash: \\\\ OK'", (pos = 0, &pos));
  assert(!strcmp(result->children->string, "It\'s John Doe. Backslash: \\ OK"));
  ka_free(result);

  result = ka_parser("age; 42 'John Doe' 21;name", (pos = 0, &pos));
  assert(!strcmp(result->children->symbol, "age"));
  assert(*result->next->children->number == 42);
  assert(!strcmp(result->next->children->next->string, "John Doe"));
  assert(*result->next->children->next->next->number == 21);
  assert(!strcmp(result->next->next->children->symbol, "name"));
  ka_free(result);

  result = ka_parser("42 'John Doe'; name (22) [1 2] {71; 72}", (pos = 0, &pos));
  KaNode *number = result->children,
         *string = result->children->next,
         *symbol = result->next->children,
         *expr = result->next->children->next->children,
         *list = result->next->children->next->next,
         *block = result->next->children->next->next->next;
  assert(number->type == KA_NUMBER && *number->number == 42);
  assert(string->type == KA_STRING && !strcmp(string->string, "John Doe"));
  assert(symbol->type == KA_SYMBOL && !strcmp(symbol->symbol, "name"));
  assert(expr->type == KA_EXPR && *expr->children->number == 22);
  assert(list->type == KA_LIST);
  assert(*list->children->children->number == 1);
  assert(*list->children->children->next->number == 2);
  assert(block->type == KA_BLOCK && *block->children->children->number == 71);
  assert(block->type == KA_BLOCK && *block->children->next->children->number == 72);
  ka_free(result);

  result = ka_parser("i = 1 * 2; 5 + 6 - 7", (pos = 0, &pos));
  KaNode *expr_set = result->children,
         *expr_mul = result->children->next->next->children,
         *expr_sub = result->next->children->children,
         *expr_sum = result->next->children->children->next->children;
  assert(expr_set->type == KA_SYMBOL && !strcmp(expr_set->symbol, "="));
  assert(!strcmp(expr_set->next->symbol, "i"));
  assert(expr_mul->type == KA_SYMBOL && !strcmp(expr_mul->symbol, "*"));
  assert(*expr_mul->next->number == 1 && *expr_mul->next->next->number == 2);
  assert(expr_sub->type == KA_SYMBOL && !strcmp(expr_sub->symbol, "-"));
  assert(*expr_sub->next->next->number == 7);
  assert(expr_sum->type == KA_SYMBOL && !strcmp(expr_sum->symbol, "+"));
  assert(*expr_sum->next->number == 5 && *expr_sum->next->next->number == 6);
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
  KaNode *ctx = ka_new(KA_CTX);
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
  assert(result->type == KA_NONE);
  ka_free(result);

  ka_free(else_block), ka_free(block), ka_free(ctx);
}

void test_loop() {
  KaNode *ctx = ka_new(KA_CTX);
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("i"), ka_number(0), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("lt"), ka_func(ka_lt), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("set"), ka_func(ka_set), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("add"), ka_func(ka_add), NULL)));

  KaNode *condition = ka_block(
      ka_symbol("lt"), ka_symbol("i"), ka_number(10), NULL);
  
  KaNode *block = ka_block(
      ka_symbol("set"), ka_symbol("i"),
      ka_expr(ka_symbol("add"), ka_symbol("i"), ka_number(1), NULL), NULL);

  ka_free(ka_loop(&ctx, ka_chain(condition, block, NULL)));
  KaNode *var = ka_get(&ctx, ka_symbol("i"));
  assert(*var->number == 10); ka_free(var);

  ka_free(ctx);
}

void test_init() {
  KaNode *ctx = ka_init();
  ctx->key = strdup("ctx");

  assert(!strcmp(ctx->next->key, "get"));
  assert(!strcmp(ctx->next->next->key, "def"));
  assert(!strcmp(ctx->next->next->next->key, "set"));
  assert(!strcmp(ctx->next->next->next->next->key, "del"));

  ka_free(ka_del(&ctx, ka_symbol("ctx")));
  ka_free(ctx);
}

void test_code() {
  KaNode *ctx = ka_init();
  ctx->key = strdup("ctx");
  int pos = 0;

  char *code = "\
    def hello { print \"1st: \" $1 \", 2nd: \" first }\n\
    def i 2;\n\
    list = [\
      1,\
      i,\
      third : 3,\
      4\
    ];\n\
    hello(first : 33, age : 34);\n\
    print first age;\n\
    print i;\n\
    key = 'i';\n\
    print 'two: ' $key;\n\
  ";

  KaNode *expr = ka_parser(code, &pos);
//  print_chain(expr);
  ka_free(ka_eval(&ctx, expr));
//  print_chain(ctx);
  
  ka_free(expr);
  ka_free(ka_del(&ctx, ka_symbol("ctx")));
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
  test_ref();
  test_get();
  test_key();
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
  test_init();
  test_code();

  printf("\nAll tests passed!\n\n");
  return 0;
}
