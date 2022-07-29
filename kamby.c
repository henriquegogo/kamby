#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { NONE, NUM, IDF, STR, EXPR } Type;

struct Node {
  Type type;
  char *key;
  union {
    long num;
    char *str;
    struct Node *chld;
  };
  struct Node *(*fn)(struct Node, struct Node);
  struct Node *next;
};

int pos = -1;
int bol = 1;

struct Node *builtin_set(struct Node *node, struct Node **env) {
  struct Node *reg = malloc(sizeof(struct Node));
  memcpy(reg, node->next, sizeof(struct Node));
  reg->key = node->str;
  reg->next = *env;
  return *env = reg;
}

struct Node *builtin_get(struct Node *node, struct Node **env) {
  struct Node *reg = *env;
  while (reg && strcmp(node->str, reg->key) != 0) reg = reg->next;
  return reg;
}

long builtin_sum(struct Node *node, struct Node **env) {
  return node->num + node->next->num;
}

void builtin_puts(struct Node *node, struct Node **env) {
  while (node) {
    struct Node *value = malloc(sizeof(struct Node));
    switch (node->type) {
      case NUM:
        printf("%lu ", node->num);
        break;
      case IDF:
        value = builtin_get(node, env);
        printf("%s ", value->str);
        break;
      case STR:
        printf("%s ", node->str);
        break;
      default:;
    }
    node = node->next;
  }
  printf("\b\n");
}

char ident[1024];
struct Node *eval(struct Node *node, struct Node **env) {
  char *fn = malloc(256);
  if (node && node->type == IDF) {
    fn = node->str;
    node = node->next;
  }

  if (strcmp(fn, "+") == 0) printf("%lu\n", builtin_sum(node, env));
  else if (strcmp(fn, "=") == 0) builtin_set(node, env);
  else if (strcmp(fn, "puts") == 0) builtin_puts(node, env);

  while (node) {
    switch (node->type) {
      case NUM:
        printf("%sNUM: %lu\n", ident, node->num);
        break;
      case IDF:
        printf("%sIDF: %s\n", ident, node->str);
        break;
      case STR:
        printf("%sSTR: %s\n", ident, node->str);
        break;
      case EXPR:
        printf("%sEXPR:\n", ident);
        strcat(ident, "..");
        eval(node->chld, env);
        strcpy(ident, ident + 2);
        break;
      default:
        printf("%sNONE\n", ident);
    }

    node = node->next;
  }

  return NULL;
}

struct Node *parse(char *text) {
  int length = strlen(text);
  struct Node *head = malloc(sizeof(struct Node));
  struct Node *tail = head;

  while (pos < 0 || bol) {
    if (pos < 0) pos++;     // First action is create an wrapper expression
    else if (bol) bol = 0;  // Beginning of line creates new expression
    tail->next = malloc(sizeof(struct Node));
    tail->next->type = EXPR;
    tail->next->chld = parse(text);
    tail = tail->next;
  }

  while (pos < length) {
    int start = pos;
    struct Node *node = malloc(sizeof(struct Node));

    switch (text[pos]) {
      case ' ':
        break;
      case '#':
        while (text[pos + 1] != '\n') pos++;
        break;
      case '(': case '[': case '{':
        pos++;
        node->type = EXPR;
        node->chld = parse(text);
        break;
      case '\n':
        pos++;
        bol = 1;
      case ')': case ']': case '}':
        length = 0;
        continue;
      case '\'':
      case '"':
        while (text[++pos] != text[start]);
        node->type = STR;
        node->str = malloc(pos - start - 1);
        strncpy(node->str, text + start + 1, pos - start - 1);
        break;
      default:
        if (isdigit(text[pos])) {
          while (isdigit(text[pos + 1])) pos++;
          node->type = NUM;
          node->num = atoi(text + start);
        } else if (isgraph(text[pos])) {
          while (isgraph(text[pos]) &&
              text[pos] != '(' && text[pos] != ')' &&
              text[pos] != '{' && text[pos] != '}' &&
              text[pos] != '[' && text[pos] != ']') pos++;
          node->type = IDF;
          node->str = malloc(pos - start);
          strncpy(node->str, text + start, pos - start);
          // (2 + 4) -> (+ 2 4) ... (true == true) -> (== true true)
          if (head->next == tail && !node->str[2] && ispunct(text[start])) {
            node->next = head->next;
            head->next = node;
            continue;
          }
          pos--;
        }
    };

    if (node->type) {
      tail->next = node;
      tail = node;
    }

    pos++;
  };
  
  return head->next;
}

int main(int argc, char **argv) {
  struct Node *env = malloc(sizeof(struct Node));

  if (argc == 1) {
    char input[1024];
    while (strcmp(input, "exit\n") != 0) {
      printf("kamby> ");
      fflush(stdout);
      fgets(input, 1024, stdin);
      eval(parse(input), &env);
      pos = -1;
      bol = 1;
    }
  } else if (argc == 2) {
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size);
    fread(text, size, 1, file);
    eval(parse(text), &env);
    fclose(file);
  }

  return 0;
}
