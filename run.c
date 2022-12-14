#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kamby.h"

char *ka_run(char *text) {
  struct KaNode *env = ka_init();
  struct KaNode *pos = calloc(1, KANODE_SIZE);
  struct KaNode *res = ka_eval(ka_parser(text, &pos), &env);
  while (res->next) res = res->next;
  char *output = calloc(1, sizeof(long long));
  if (res->type == KA_NUM) sprintf(output, "%lld", res->num);
  else strcpy(output, res->str);
  return output;
}
