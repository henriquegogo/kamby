#include <stdio.h>

#include "kamby.h"

int print_level = 0;
void print_node(KaNode *node) {
  if (!node) return;
  KaNode *child;
  for (int i = 0; i < print_level; i++) printf("  ");
  switch (node->type) {
    case KA_NUMBER:
      printf("number %s: %.Lf\n", node->key ? node->key : "", *node->number);
      break;
    case KA_STRING:
      printf("string %s: %s\n", node->key ? node->key : "", node->string);
      break;
    case KA_SYMBOL:
      printf("symbol %s\n", node->key);
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

KaNode *builtin_print(KaNode **ctx, KaNode *arg) {
  printf("printing: %s\n", arg->string);
  ka_free(arg);
  return NULL;
}

int main() {
  printf("Kamby v0.0.2\n");

  KaNode *ctx = ka_new(KA_CTX);
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

  KaNode *dupfruit = ka_copy(ka_get(&ctx, ka_symbol("fruits")));
  print_node(dupfruit);
  ka_free(dupfruit);

  printf("\n");

  ka_def(&ctx, ka_chain(ka_symbol("def"), ka_func(ka_def), NULL));
  ka_def(&ctx, ka_chain(ka_symbol("print"), ka_func(builtin_print), NULL));

  print_chain(ctx);
  printf("\n");

  KaNode *code_block = ka_expr(
      ka_symbol("name"),
      ka_number(42),
      ka_string("Hello"),
      ka_expr(ka_symbol("def"), ka_symbol("text"), ka_string("My Text"), NULL),
      ka_expr(ka_symbol("print"), ka_symbol("text"), NULL),
      ka_list(
        ka_symbol("name"),
        ka_string("endlist"),
        NULL),
      ka_symbol("var"),
      ka_expr(ka_symbol("def"), ka_symbol("say"),
        ka_block(
          ka_expr(ka_symbol("print"), ka_string("This is a block"), NULL),
        NULL),
      NULL),
      ka_expr(ka_symbol("say"), NULL),
      ka_symbol("name"),
      ka_symbol("age"),
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
