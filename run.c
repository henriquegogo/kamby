#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kamby.h"

char *ka_run(char *text) {
  struct KaNode *env = ka_init();
  struct KaNode *pos = ka_new();
  struct KaNode *res = ka_eval(ka_parser(text, &pos), &env);
  while (res->next) {
    struct KaNode *prev = res;
    res = res->next;
    free(prev);
  }
  char *output = calloc(1, sizeof(long long));
  if (res->type == KA_NUM) {
    sprintf(output, "%lld", res->num);
  } else if (res->type == KA_STR) {
    strcpy(output, res->str);
  }
  ka_free(&env);
  ka_free(&pos);
  return output;
}
