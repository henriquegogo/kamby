#include <stdio.h>
#include <unistd.h>
#include "kamby.h"

int main(int argc, char *argv[]) {
  KaNode *ctx = ka_init();
  ctx->key = strdup("ctx");
  int pos = 0;

  // Read file
  if (argc > 1) {
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *source = calloc(1, size + 1);
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);
    KaNode *expr = ka_parser(source, &pos);
    ka_free(ka_eval(&ctx, expr)), ka_free(expr);
    free(source);
  }

  // REPL
  else {
    if (isatty(fileno(stdin))) printf("Kamby 0.2.0\n> ");
    char input[8192];
    while (fgets(input, sizeof(input), stdin)) {
      KaNode *expr = ka_parser(input, (pos = 0, &pos));
      ka_free(ka_eval(&ctx, expr)), ka_free(expr);
      input[0] = '\0';
      if (isatty(fileno(stdin))) printf("> ");
    }
  }

  ka_free(ka_del(&ctx, ka_symbol("ctx")));
  ka_free(ctx);
  return 0;
}
