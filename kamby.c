#include <stdio.h>
#include "kamby.h"

KaNode *print(KaNode **ctx, KaNode *args) {
  for (KaNode *arg = args; arg != NULL; arg = arg->next) {
    switch (arg->type) {
      case KA_NUMBER:
        if (*arg->number == (long long)(*arg->number)) {
          printf("%lld", (long long)(*arg->number));
        } else {
          printf("%.2Lf", *arg->number);
        }
        break;
      case KA_STRING:
        printf("%s", arg->string);
        break;
      default:;
    }
  }
  printf("\n");
  ka_free(args);
  return ka_new(KA_NONE);
}

int main() {
  KaNode *ctx = ka_new(KA_CTX);
  int pos = 0;
  char input[1024];

  ka_free(ka_def(&ctx, ka_chain(ka_symbol("def"), ka_func(ka_def), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("print"), ka_func(print), NULL)));

  printf("Valid keywords: def, print\n");
  printf("Usage:\n");
  printf("  def <symbol> <number|string>\n");
  printf("  print <symbol>\n\n");

  while (1) {
    printf("kamby> ");
    fflush(stdout);
    fgets(input, 1024, stdin);
    KaNode *expr = ka_parser(input, (pos = 0, &pos));
    ka_free(ka_eval(&ctx, expr->children));
    ka_free(expr);
    input[0] = '\0';
  }

  ka_free(ctx);
  return 0;
}
