#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "kamby.h"

struct KaNode **zeropos(struct KaNode **pos) {
  *pos = calloc(1, KANODE_SIZE);
  return pos;
}

void test_init() {
  printf("- Initialization\n");
  struct KaNode *env = ka_init();
  assert(env);
}

void test_parser() {
  printf("- Parser\n");
  struct KaNode *pos = calloc(1, KANODE_SIZE);
  struct KaNode *ast = ka_parser("4 + 5; puts 'message'", &pos);
  assert(ast->type == KA_EXPR);
  assert(ast->chld->type == KA_EXPR);
  assert(strcmp(ast->chld->chld->str, "+") == 0);
  assert(ast->chld->chld->next->num == 4);
  assert(ast->chld->chld->next->next->num == 5);
  assert(ast->next->type == KA_EXPR);
  assert(ast->next->chld->type == KA_IDF);
  assert(strcmp(ast->next->chld->str, "puts") == 0);
  assert(ast->next->chld->next->type == KA_STR);
  assert(strcmp(ast->next->chld->next->str, "message") == 0);
}

void test_evaluation() {
  printf("- Evaluation\n");
  struct KaNode *env = ka_init();
  struct KaNode *ast = calloc(1, KANODE_SIZE);
  ast->type = KA_EXPR;
  ast->chld = ka_lnk( ka_idf("+"), ka_num(4),  ka_num(5), 0);
  assert(ka_eval(ast, &env)->num == 9);
}

void test_constructors() {
  printf("- Constructors\n");
  struct KaNode *env = ka_init();
  assert(ka_num(5)->num == 5);
  assert(strcmp(ka_str("string")->str, "string") == 0);
  assert(strcmp(ka_idf("identifier")->str, "identifier") == 0);
  //assert(ka_fn(ka_fn)->fn == ka_fn);
  assert(ka_lnk(ka_num(5), ka_num(6), 0)->next->num == 6);
}

void test_definitions() {
  printf("- Definitions and memory control\n");
  struct KaNode *env = ka_init();
  assert(ka_def(ka_lnk( ka_str("number"), ka_num(987), 0), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 987);
  assert(ka_set(ka_lnk( ka_idf("number"), ka_num(789), 0), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 789);
  assert(ka_def(ka_lnk( ka_idf("number"), ka_num(456), 0), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 456);
  assert(ka_del(ka_idf("number"), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 789);
}

void test_stack() {
  printf("- Getting data from stack\n");
  struct KaNode *pos = calloc(1, KANODE_SIZE);
  struct KaNode *env = ka_init();
  ka_def(ka_lnk( ka_str("num1"), ka_num(987), 0), &env);
  ka_def(ka_lnk( ka_str("num2"), ka_num(789), 0), &env);
  assert(ka_eval(ka_parser(".", zeropos(&pos)), &env)->num == 789);
  assert(ka_eval(ka_parser(". 1", zeropos(&pos)), &env)->num == 789);
  assert(ka_eval(ka_parser(". 2", zeropos(&pos)), &env)->num == 987);
  assert(!ka_eval(ka_parser(". 3", zeropos(&pos)), &env)->num);
}

void test_call() {
  printf("- Call block passing context\n");
  struct KaNode *pos = calloc(1, KANODE_SIZE);
  struct KaNode *env = ka_init();
  ka_def(ka_lnk( ka_str("num1"), ka_num(987), 0), &env);
  ka_eval(ka_parser("list = [12 23 34]", &pos), &env);
  ka_eval(ka_parser("obj = [name := 'me'; age := 20]", zeropos(&pos)), &env);
  ka_def(ka_lnk( ka_str("num2"), ka_num(789), 0), &env);
  assert(ka_eval(ka_parser("list :: {.}", zeropos(&pos)), &env)->num == 12);
  assert(ka_eval(ka_parser("list :: {. 1}", zeropos(&pos)), &env)->num == 12);
  ka_eval(ka_parser("list :: {(. 1) = 99}", zeropos(&pos)), &env);
  assert(ka_eval(ka_parser("list :: {. 1}", zeropos(&pos)), &env)->num == 99);
  assert(ka_eval(ka_parser("list :: {. 2}", zeropos(&pos)), &env)->num == 23);
  assert(ka_eval(ka_parser("list :: {. 3}", zeropos(&pos)), &env)->num == 34);
  assert(!ka_eval(ka_parser("list :: {. 4}", zeropos(&pos)), &env)->num);
  assert(ka_eval(ka_parser("obj :: {age}", zeropos(&pos)), &env)->num == 20);
  ka_eval(ka_parser("obj :: {age = 40}", zeropos(&pos)), &env);
  assert(ka_eval(ka_parser("obj :: {age}", zeropos(&pos)), &env)->num == 40);
  assert(ka_eval(ka_parser(". 1", zeropos(&pos)), &env)->num == 789);
  assert(ka_eval(ka_parser(". 4", zeropos(&pos)), &env)->num == 987);
}

void test_call_change_values() {
  printf("- Remove values inside a contextualized block\n");
  struct KaNode *pos = calloc(1, KANODE_SIZE);
  struct KaNode *env = ka_init();
  ka_eval(ka_parser("list = [12 23 34]", &pos), &env);
  //ERROR: ka_eval(ka_parser("list :: {del (. 3)}", &pos), &env);
  //ERROR: ka_eval(ka_parser("list :: {del (. 2)}", &pos), &env);
  ka_eval(ka_parser("obj = [name := 'me'; age := 20]", zeropos(&pos)), &env);
  ka_eval(ka_parser("obj :: {del name}", zeropos(&pos)), &env);
  ka_eval(ka_parser("obj :: {del age}", zeropos(&pos)), &env);
  //ERROR: assert(!ka_eval(ka_parser("obj :: {name}", zeropos(&pos)), &env));
  //ERROR: assert(!ka_eval(ka_parser("obj :: {age}", zeropos(&pos)), &env));
}

int main() {
  printf("TESTING...\n");

  test_init();
  test_parser();
  test_evaluation();
  test_constructors();
  test_definitions();
  test_stack();
  test_call();
  test_call_change_values();

  printf("ALL TESTS PASSED!\n");

  return 0;
}
