#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

struct KaNode *builtin_sum(struct KaNode *node, struct KaNode **env) {
  struct KaNode *value = malloc(sizeof(struct KaNode));
  value->type = node->type;
  value->num = node->num + node->next->num;
  return value;
}

struct KaNode *builtin_puts(struct KaNode *node, struct KaNode **env) {
  while (node) {
    switch (node->type) {
      case NUM:
        printf("%lu ", node->num);
        break;
      case STR:
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
  struct KaNode *pos = malloc(sizeof(struct KaNode));
  struct KaNode *env = malloc(sizeof(struct KaNode));

  ka_init(&env);

  ka_fn("+", builtin_sum, &env);
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
