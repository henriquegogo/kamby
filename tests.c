#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "kamby.h"

void test_init() {
  printf("- Initialization\n");
  assert(ka_init());
}

void test_constructors() {
  printf("- Constructors\n");
  assert(ka_num(5)->num == 5);
  assert(strcmp(ka_str("string")->str, "string") == 0);
  assert(strcmp(ka_idf("identifier")->str, "identifier") == 0);
  assert(ka_fn(ka_fn)->fn == ka_fn);
  assert(ka_link(ka_num(5), ka_num(6), 0)->next->num == 6);
}

void test_definitions() {
  printf("- Definitions and memory control\n");
  struct KaNode *env = ka_init();
  assert(ka_def(ka_link(
    ka_str("number"), ka_num(987),
  0), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 987);
  assert(ka_set(ka_link(
    ka_idf("number"), ka_num(789),
  0), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 789);
  assert(ka_def(ka_link(
    ka_idf("number"), ka_num(456),
  0), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 456);
  assert(ka_del(ka_idf("number"), &env));
  assert(ka_get(ka_idf("number"), &env)->num == 789);
}

void test_math() {
  printf("- TODO: Mathematics\n");
  assert(1);
}

void test_logical_operators() {
  printf("- TODO: Logical operators\n");
  assert(1);
}

int main() {
  printf("TESTING...\n");

  test_init();
  test_constructors();
  test_definitions();
  test_math();
  test_logical_operators();

  printf("ALL TESTS PASSED!\n");

  return 0;
}
