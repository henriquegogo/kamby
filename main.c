#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

struct KaNode *builtin_puts(struct KaNode *node, struct KaNode **env) {
  while (node) {
    switch (node->type) {
      case KA_NUM:
        printf("%lu ", node->num);
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

int main(int argc, char **argv) {
  struct KaNode *env = ka_init();
  struct KaNode *pos = malloc(sizeof(struct KaNode));

  ka_fn("puts", builtin_puts, &env);

  if (argc == 1) {
    char input[1024];
    while (strcmp(input, "exit\n") != 0) {
      printf("kamby> ");
      fflush(stdout);
      fgets(input, 1024, stdin);
      pos = malloc(sizeof(struct KaNode));
      ka_eval(ka_parse(input, &pos), &env);
    }
  } else if (argc == 2) {
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size);
    fread(text, size, 1, file);
    ka_eval(ka_parse(text, &pos), &env);
    fclose(file);
  }

  return 0;
}
