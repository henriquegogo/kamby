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
  struct Node *(*fn)();
  struct Node *next;
};

int pos = 0;
int bol = 1;

struct Node *builtin_set(struct Node *node, struct Node **env) {
  struct Node *reg = malloc(sizeof(struct Node));
  memcpy(reg, node->next, sizeof(struct Node));
  reg->key = node->str;
  reg->next = *env;
  *env = reg;
  return node->next;
}

struct Node *builtin_get(struct Node *node, struct Node **env) {
  struct Node *reg = *env;
  while (reg && strcmp(node->str, reg->key) != 0) reg = reg->next;
  return reg;
}

struct Node *builtin_sum(struct Node *node, struct Node **env) {
  struct Node *value = malloc(sizeof(struct Node));
  value->type = node->type;
  value->num = node->num + node->next->num;
  return value;
}

struct Node *builtin_puts(struct Node *node, struct Node **env) {
  while (node) {
    struct Node *value = malloc(sizeof(struct Node));
    switch (node->type) {
      case NUM:
        printf("%lu ", node->num);
        break;
      case IDF:
        printf("%s (PRINT INTERNAL) ", builtin_get(node, env)->str);
        break;
      case STR:
        printf("%s ", node->str);
        break;
      default:;
    }
    node = node->next;
  }
  printf("\b\n");
  return malloc(sizeof(struct Node));
}

struct Node *eval(struct Node *node, struct Node **env) {
  struct Node *head = malloc(sizeof(struct Node));
  struct Node *tail = head;

  while (node) {
    if (node->type == EXPR) {
      tail->next = eval(node->chld, env);
    } else if (node->type == IDF) {
      tail->next = node;
    } else {
      tail->next = node;
    }
    tail = tail->next;
    node = node->next;
  }

  head = head->next;
  if (head && head->type == IDF) {
    return builtin_get(head, env)->fn(head->next, env);
  }

  return head;
}

struct Node *parse(char *text) {
  int length = strlen(text);
  struct Node *head = malloc(sizeof(struct Node));
  struct Node *tail = head;

  while (bol) {
    if (bol) bol = 0;  // Beginning of line creates new expression
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
      case ';':
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
          while (isgraph(text[pos]) && text[pos] != ';' &&
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
      tail = tail->next;
    }

    pos++;
  };
  
  return head->next;
}

int main(int argc, char **argv) {
  struct Node *env = malloc(sizeof(struct Node));

  struct Node *def = malloc(sizeof(struct Node));
  def->next = env;
  def->key = "=";
  def->fn = builtin_set;
  env = def;

  def = malloc(sizeof(struct Node));
  def->next = env;
  def->key = "+";
  def->fn = builtin_sum;
  env = def;

  def = malloc(sizeof(struct Node));
  def->next = env;
  def->key = "puts";
  def->fn = builtin_puts;
  env = def;

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
