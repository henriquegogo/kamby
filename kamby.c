#include <stdio.h>

#include "kamby.h"

KaNode *print(KaNode **ctx, KaNode *args) {
  switch (args->type) {
    case KA_NUMBER:
      printf("%.2Lf\n", *args->number);
      break;
    case KA_STRING:
      printf("%s\n", args->string);
      break;
    default:;
  }
  ka_free(args);
  return NULL;
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
    KaNode *result = ka_eval(&ctx, expr->children);
    ka_free(result), ka_free(expr);
    input[0] = '\0';
  }

  ka_free(ctx);
  return 0;
}
