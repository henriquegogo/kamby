#include <stdio.h>
#include "kamby.h"

KaNode *_print_(KaNode **ctx, KaNode *args) {
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

KaNode *_exit_(KaNode **ctx, KaNode *args) {
  ka_free(args);
  exit(0);
  return ka_new(KA_NONE);
}

int main() {
  KaNode *ctx = ka_ctx();
  int pos = 0;
  char input[8192];

  // Variables
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("get"), ka_func(ka_get), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("def"), ka_func(ka_def), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("set"), ka_func(ka_set), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("del"), ka_func(ka_del), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol(":="),  ka_func(ka_def), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("="),   ka_func(ka_set), NULL)));

  // Logical operators
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("&&"), ka_func(ka_and), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("||"), ka_func(ka_or),  NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("!"),  ka_func(ka_not), NULL)));

  // Comparison operators
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("=="), ka_func(ka_eq),  NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("!="), ka_func(ka_neq), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol(">"),  ka_func(ka_gt),  NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("<"),  ka_func(ka_lt),  NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol(">="), ka_func(ka_gte), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("<="), ka_func(ka_lte), NULL)));

  // Arithmetic operators
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("+"), ka_func(ka_add), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("-"), ka_func(ka_sub), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("*"), ka_func(ka_mul), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("/"), ka_func(ka_div), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("%"), ka_func(ka_mod), NULL)));

  // Conditional and loops
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("if"),   ka_func(ka_if),   NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("loop"), ka_func(ka_loop), NULL)));

  // Standard functions
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("print"), ka_func(_print_), NULL)));
  ka_free(ka_def(&ctx, ka_chain(ka_symbol("exit"),  ka_func(_exit_), NULL)));

  // Context separator
  ka_free(ka_def(&ctx, ka_chain(ka_symbol(""),  ka_ctx(), NULL)));

  printf("Valid keywords:\n  ");
  int cols = 0;
  for (KaNode *curr = ctx; curr->key && curr->next; curr = curr->next) {
    printf("%s ", curr->key);
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
