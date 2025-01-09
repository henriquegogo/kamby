#include <stdio.h>

#include "kamby.h"

int print_level = 0;
void print_node(KaNode *node) {
  if (!node) return;
  KaNode *child;
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
      child = node->children;
      while (child) { print_node(child); child = child->next; }
      print_level--;
      break;
    case KA_EXPR:
      printf("(ref %d) expr %s:\n", *node->refcount, node->key);
      print_level++;
      child = node->children;
      while (child) { print_node(child); child = child->next; }
      print_level--;
      break;
    case KA_BLOCK:
      printf("(ref %d) block %s:\n", *node->refcount, node->key);
      print_level++;
      child = node->children;
      while (child) { print_node(child); child = child->next; }
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
  printf("Kamby v0.0.2\n");

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

  ka_def("sum", ka_block(
    ka_symbol("a"),
    ka_symbol("+"),
    ka_symbol("b"), NULL), &env);

  print_chain(env);
  printf("\n");
  
  KaNode *dupfruit = ka_copy(ka_get("fruits", &env));
  print_node(dupfruit);
  ka_free(dupfruit);

  ka_eval(ka_get("sum", &env), &env);
  
  ka_free(env);

  return 0;
}
