#include <stdio.h>
#include <unistd.h>
#include "kamby.h"

int print_level = 1;
void transpile_tree(KaNode *nodes) {
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
      transpile_tree(node->children);
      for (int i = 0; i < print_level; i++) printf("  ");
      printf("NULL),\n");
    }
    else printf("\n");
  }
  print_level--;
}

char *transpile(KaNode **ctx, char *path) {
  int pos = 0;
  KaNode *source = ka_read(ctx, ka_string(path));
  KaNode *nodes = ka_parser(source->string, &pos);
  printf("#include \"kamby.h\"\n\n");
  printf("int main() {\n");
  printf("  KaNode *nodes = ka_expr(\n");
  transpile_tree(nodes);
  printf("  NULL);\n\n");
  printf("  KaNode *ctx = ka_init();\n");
  printf("  ka_free(ka_eval(&ctx, nodes));\n");
  printf("  ka_free(ctx);\n");
  printf("  return 0;\n");
  printf("}\n");
  ka_free(nodes), ka_free(source);
  return NULL;
}

int main(int argc, char *argv[]) {
  KaNode *ctx = ka_init();
  ctx->key = strdup("ctx");
  int pos = 0;

  if (argc > 1 && !strcmp(argv[1], "--help")) {
    printf("Usage: kamby [options] [file]\n");
    printf("Options:\n");
    printf("  --help          Display this help message\n");
    printf("  --version       Display version information\n");
    printf("  -c              Transpile a file to C\n");
  }
  else if (argc > 1 && !strcmp(argv[1], "--version")) printf("Kamby 0.2.0\n");
  else if (argc > 2 && !strcmp(argv[1], "-c")) transpile(&ctx, argv[2]);
  else if (argc > 1) ka_free(ka_load(&ctx, ka_string(argv[1])));
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
