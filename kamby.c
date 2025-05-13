#include <stdio.h>
#include <unistd.h>
#include "kamby.h"

int main() {
  KaNode *ctx = ka_init();
  ctx->key = strdup("ctx");
  int pos = 0;
  char input[8192];

  if (isatty(fileno(stdin))) {
    printf("Valid keywords:\n  ");
    int cols = 0;
    for (KaNode *curr = ctx->next; curr->key && curr->next; curr = curr->next) {
      if (strlen(curr->key) > 0) printf("%s ", curr->key);
      if ((cols += strlen(curr->key) + 1) > 50) { printf("\n  "); cols = 0; }
    }
    printf("\n\n");
    printf("Usage:\n");
    printf("  <symbol> = <number|string>\n");
    printf("  print <symbol>\n\n");
    printf("kamby> ");
  }

  while (fgets(input, 8192, stdin)) {
    KaNode *expr = ka_parser(input, (pos = 0, &pos));
    KaNode *result = ka_eval(&ctx, expr);
    ka_free(result), ka_free(expr);
    input[0] = '\0';
    if (isatty(fileno(stdin))) {
      printf("kamby> ");
      fflush(stdout);
    }
  }

  ka_free(ka_del(&ctx, ka_symbol("ctx")));
  ka_free(ctx);
  return 0;
}
