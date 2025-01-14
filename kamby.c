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

KaNode *builtin_print(KaNode **ctx, KaNode *arg) {
  printf("printing: %s\n", arg->string);
  ka_free(arg);
  return NULL;
}

int main() {
  printf("Kamby v0.0.2\n");

  KaNode *ctx = ka_new(KA_NONE);
  ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("Henrique"), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(40), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("newvar"), ka_string("This is not a value"), NULL));

  ka_def(&ctx, ka_chain(ka_symbol("seeds"), ka_list(
    ka_string("Wheat"),
    ka_string("Rye"),
    ka_string("Barley"), NULL), NULL));

  ka_def(&ctx, ka_chain(ka_symbol("fruits"), ka_list(
    ka_string("Apple"),
    ka_string("Banana"),
    ka_number(22),
    ka_get(&ctx, ka_symbol("seeds")),
    ka_get(&ctx, ka_symbol("name")),
    ka_string("Grape"), NULL), NULL));

  ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("Mr Soarrs"), NULL));

  ka_def(&ctx, ka_chain(ka_symbol("sum"), ka_expr(
    ka_symbol("name"),
    ka_symbol("age"), NULL), NULL));

  print_chain(ctx);
  printf("\n");
  
  KaNode *dupfruit = ka_copy(ka_get(&ctx, ka_symbol("fruits")));
  print_node(dupfruit);
  ka_free(dupfruit);

  KaNode *code_block = ka_expr(
      ka_symbol("name"),
      ka_number(42),
      ka_string("Hello"),
      ka_list(
        ka_symbol("name"),
        ka_string("endlist"),
        NULL),
      ka_symbol("var"),
      ka_symbol("name"),
      NULL);

  KaNode *print = ka_func(builtin_print);
  print->func(&ctx, ka_string("Hello, World!"));
  ka_free(print);

  printf("\n");
  KaNode *result = ka_eval(&ctx, code_block);
  print_chain(result);
  ka_free(result);
  
  ka_free(code_block);
  
  ka_free(ctx);

  return 0;
}
