#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { NONE, NUM, IDF, STR, EXPR } Type;

struct Node {
  Type type;
  union {
    long num;
    char *str;
    struct Node *chld;
  };
  struct Node *next;
};

int bol = 1;
int pos = 0;

int isbracket(char ch) {
  return ch == '(' || ch == ')' ||
    ch == '{' || ch == '}' ||
    ch == '[' || ch == ']';
}

char ident[1024];
void eval(struct Node *node) {
  if (node->type == IDF) {
    printf("%s%s\n", ident, node->str);
    node = node->next;
  }

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
        eval(node->chld);
        strcpy(ident, ident + 2);
        break;
      default:
        printf("%sNONE\n", ident);
    }

    node = node->next;
  }
}

struct Node *parse(char *text) {
  int length = strlen(text);
  struct Node *head = malloc(sizeof(struct Node));
  struct Node *tail = head;

  while (bol) {
    bol = 0;
    tail->next = malloc(sizeof(struct Node));
    tail = tail->next;
    tail->type = EXPR;
    tail->chld = parse(text);
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
      case '(':
      case '[':
      case '{':
        pos++;
        node->type = EXPR;
        node->chld = parse(text);
        break;
      case ')':
      case ']':
      case '}':
        length = 0;
        continue;
      case '\n':
        pos++;
        bol = 1;
        length = 0;
        continue;
      case '"':
        while (text[++pos] != '"');
        node->type = STR;
        node->str = malloc(pos - start - 1);
        strncpy(node->str, text + start + 1, pos - start - 1);
        break;
      default:
        if (isdigit(text[pos])) {
          while (isdigit(text[pos + 1])) pos++;
          node->type = NUM;
          node->num = atoi(text + start);
        } else if (isgraph(text[pos]) && !isbracket(text[pos])) {
          while (isgraph(text[pos]) && !isbracket(text[pos])) pos++;
          node->type = IDF;
          node->str = malloc(pos - start);
          strncpy(node->str, text + start, pos - start);
          // Convention: Identifiers with 1 or 2 chars initialized by punct
          // and in second place of list, will be moved to beginning
          // (2 + 4) -> (+ 2 4)
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
  char *code = ""
    "puts \"Hello, World!\" # Comments\n"
    "go (/ (2 + 2) 9)\n"
    "sum ((12 + 34) * 4)\n"
    "\"Simple Atomic\"\n"
    "42 # Atomic number\n"
    "car cdr\n"
    "[4 3 2 5]\n"
    "endify \"The end\"";

  eval(parse(code));
}
