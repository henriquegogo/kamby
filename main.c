#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

struct KaNode *builtin_puts(struct KaNode *node, struct KaNode **env) {
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
  return malloc(sizeof(struct KaNode));
}

char ident[256];
void ka_tree(struct KaNode *node) {
  while (node) {
    switch (node->type) {
      case KA_EXPR:
        printf("%sEXPR:\n", ident);
        strcat(ident, "..");
        ka_tree(node->chld);
        strcpy(ident, ident + 2);
        break;
      case KA_BLCK:
        printf("%sBLCK:\n", ident);
        strcat(ident, "..");
        ka_tree(node->chld);
        strcpy(ident, ident + 2);
        break;
      case KA_LIST:
        printf("%sLIST:\n", ident);
        strcat(ident, "..");
        ka_tree(node->chld);
        strcpy(ident, ident + 2);
        break;
      case KA_NUM:
        printf("%sNUM: %lld\n", ident, node->num);
        break;
      case KA_STR:
        printf("%sSTR: %s\n", ident, node->str);
        break;
      case KA_IDF:
        printf("%sIDF: %s\n", ident, node->str);
        break;
      default:
        printf("%sNONE\n", ident);
    }
    node = node->next;
  }
}

int main(int argc, char **argv) {
  struct KaNode *env = ka_init();
  struct KaNode *pos = malloc(sizeof(struct KaNode));

  ka_def(ka_link(ka_idf("puts"), ka_fn(builtin_puts), 0), &env);

  if (argc == 1) {
    char input[1024];
    while (1) {
      printf("kamby> ");
      fflush(stdout);
      fgets(input, 1024, stdin);
      if (input[0] == '\n') continue;
      else if (strcmp(input, "exit\n") == 0) break;
      pos = malloc(sizeof(struct KaNode));
      ka_eval(ka_parse(input, &pos), &env);
      input[0] = '\0';
    }
  } else if (argc == 2) {
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size);
    fread(text, size, 1, file);
    struct KaNode *ast = ka_parse(text, &pos);
    //ka_tree(ast);
    ka_eval(ast, &env);
    fclose(file);
  }

  return 0;
}
