#include <stdio.h>
#include "kamby.h"

int main() {
  KaNode *ctx = ka_init();
  int pos = 0;
  char input[8192];

  // Context separator
  ka_free(ka_def(&ctx, ka_chain(ka_symbol(""),  ka_new(KA_CTX), NULL)));

  printf("Valid keywords:\n  ");
  int cols = 0;
  for (KaNode *curr = ctx; curr->key && curr->next; curr = curr->next) {
    if (strlen(curr->key) > 0) printf("%s ", curr->key);
    if ((cols += strlen(curr->key) + 1) > 50) { printf("\n  "); cols = 0; }
  }
  printf("\n\n");

  printf("Usage:\n");
  printf("  def <symbol> <number|string>\n");
  printf("  print <symbol>\n\n");

  while (1) {
    printf("kamby> ");
    fflush(stdout);
    fgets(input, 8192, stdin);
    KaNode *expr = ka_parser(input, (pos = 0, &pos));
    ka_free(ka_eval(&ctx, expr));
    ka_free(expr);
    input[0] = '\0';
  }

  ka_free(ctx);
  return 0;
}
