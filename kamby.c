#include <stdio.h>
#include <unistd.h>
#include "kamby.h"

int print_level = 0;
char *compile(KaNode *nodes) {
  print_level++;
  for (KaNode *node = nodes; node; node = node->next) {
    const char *types[] = { "none", "ctx", "false", "true", "number", "string",
      "symbol", "func", "list", "expr", "block" };
    for (int i = 0; i < print_level; i++) printf("  ");
    printf("ka_%s(", types[node->type]);
    if (node->type == KA_NUMBER) printf("%.2Lf),\n", *node->number);
    else if (node->type == KA_STRING) printf("\"%s\"),\n", node->string);
    else if (node->type == KA_SYMBOL) printf("\"%s\"),\n", node->symbol);
    else if (node->type >= KA_LIST) {
      printf("\n");
      compile(node->children);
      for (int i = 0; i < print_level; i++) printf("  ");
      printf("NULL),\n");
    }
    else printf("\n");
  }
  print_level--;
  return strdup("");
}

int main(int argc, char *argv[]) {
  KaNode *ctx = ka_init();
  ctx->key = strdup("ctx");
  int pos = 0;

  // Run script
  if (argc == 2) ka_load(&ctx, ka_string(argv[1]));
  // Compile
  else if (argc == 3) {
    char *source = ka_read(&ctx, ka_string(argv[1]))->string ?: "";
    KaNode *nodes = ka_parser(source, &pos);
    printf("ka_expr(\n");
    char *result = compile(nodes);
    printf("NULL);\n");
    free(result);
    ka_free(nodes);
  }
  // REPL
  else {
    if (isatty(fileno(stdin))) printf("Kamby 0.2.0\n> ");
    fflush(stdout);
    char input[8192];
    while (fgets(input, sizeof(input), stdin)) {
      KaNode *expr = ka_parser(input, (pos = 0, &pos));
      ka_free(ka_eval(&ctx, expr)), ka_free(expr);
      input[0] = '\0';
      if (isatty(fileno(stdin))) printf("> ");
      fflush(stdout);
    }
  }

  ka_free(ka_del(&ctx, ka_symbol("ctx")));
  ka_free(ctx);
  return 0;
}
