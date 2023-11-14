#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

struct KaNode *builtin_exit(struct KaNode *node, struct KaNode **env) {
  exit(0);
  return 0;
}

struct KaNode *builtin_print(struct KaNode *node, struct KaNode **env) {
  while (node) {
    switch (node->type) {
      case KA_NUM:
        printf("%lld ", node->num);
        break;
      case KA_STR:
        printf("%s ", node->str);
        break;
      default:;
    }
    node = node->next;
  }
  printf("\b\n");
  return calloc(1, KANODE_SIZE);
}

char ident[256];
struct KaNode *builtin_debug(struct KaNode *node, struct KaNode **env) {
  if (!node) node = *env;
  if (!ident[0]) printf(",,,,,,,,,,,,,,,\n");
  while (node) {
    switch (node->type) {
      case KA_EXPR:
        printf("%sEXPR %s:\n", ident, node->key);
        strcat(ident, "..");
        builtin_debug(node->val, env);
        strcpy(ident, ident + 2);
        break;
      case KA_BLCK:
        printf("%sBLCK %s:\n", ident, node->key);
        strcat(ident, "..");
        builtin_debug(node->val, env);
        strcpy(ident, ident + 2);
        break;
      case KA_LIST:
        printf("%sLIST %s:\n", ident, node->key);
        strcat(ident, "..");
        builtin_debug(node->val, env);
        strcpy(ident, ident + 2);
        break;
      case KA_NUM:
        printf("%sNUM %s: %lld\n", ident, node->key, node->num);
        break;
      case KA_STR:
        printf("%sSTR %s: %s\n", ident, node->key, node->str);
        break;
      case KA_IDF:
        printf("%sIDF %s: %s\n", ident, node->key, node->str);
        break;
      default:
        printf("%sNONE %s: %s\n", ident, node->key, node->str);
    }
    node = node->next && node->next->type ? node->next : NULL;
  }
  if (!ident[0]) printf("'''''''''''''''\n");
  return calloc(1, KANODE_SIZE);
}

int main(int argc, char **argv) {
  struct KaNode *env = ka_init();
  struct KaNode *pos = calloc(1, KANODE_SIZE);

  ka_def(ka_lnk(ka_idf("exit"),  ka_fun(builtin_exit),  0), &env);
  ka_def(ka_lnk(ka_idf("print"), ka_fun(builtin_print), 0), &env);
  ka_def(ka_lnk(ka_idf("debug"), ka_fun(builtin_debug), 0), &env);

  if (argc == 1) {
    char input[1024];
    while (1) {
      printf("kamby> ");
      fflush(stdout);
      fgets(input, 1024, stdin);
      free(pos);
      pos = calloc(1, KANODE_SIZE);
      ka_eval(ka_parser(input, &pos), &env);
      input[0] = '\0';
    }
  } else if (argc == 2) {
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = calloc(1, size + 1);
    fread(text, size, 1, file);
    struct KaNode *ast = ka_parser(text, &pos);
    ka_eval(ast, &env);
    fclose(file);
    free(text);
  }

  free(pos);
  free(env);

  return 0;
}
