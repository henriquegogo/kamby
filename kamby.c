#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

int pos = 0;
int bol = 1;

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = malloc(sizeof(struct KaNode));
  memcpy(reg, node->next, sizeof(struct KaNode));
  reg->key = node->str;
  reg->next = *env;
  *env = reg;
  return node->next;
}

struct KaNode *ka_get(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = *env;
  while (reg->next && strcmp(node->str, reg->key) != 0) reg = reg->next;
  struct KaNode *result = malloc(sizeof(struct KaNode));
  memcpy(result, reg, sizeof(struct KaNode));
  result->next = NULL;
  return result;
}

struct KaNode *ka_fn(char *key, struct KaNode *(*fn)(), struct KaNode **env) {
  struct KaNode *reg = malloc(sizeof(struct KaNode));
  reg->next = *env;
  reg->key = key;
  reg->fn = fn;
  return *env = reg;
}

struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env) {
  struct KaNode *head = malloc(sizeof(struct KaNode));
  struct KaNode *tail = head;
  
  while (node) {
    struct KaNode *value = malloc(sizeof(struct KaNode));
    switch (node->type) {
      case EXPR:
        tail->next = ka_eval(node->chld, env);
        break;
      case IDF:
        value = ka_get(node, env);
        tail->next = value->type ? value : node;
        break;
      default:
        tail->next = node;
    }
    tail = tail->next;
    node = node->next;
  }

  head = head->next;
  if (head && head->type == IDF) {
    return ka_get(head, env)->fn(head->next, env);
  }

  return head;
}

struct KaNode *ka_parse(char *text) {
  int length = strlen(text);
  struct KaNode *head = malloc(sizeof(struct KaNode));
  struct KaNode *tail = head;

  while (bol) {
    if (bol) bol = 0;  // Beginning of line creates new expression
    tail->next = malloc(sizeof(struct KaNode));
    tail->next->type = EXPR;
    tail->next->chld = ka_parse(text);
    tail = tail->next;
  }

  while (pos < length) {
    int start = pos;
    struct KaNode *node = malloc(sizeof(struct KaNode));

    switch (text[pos]) {
      case ' ':
        break;
      case '#':
        while (text[pos + 1] != '\n') pos++;
        break;
      case '(': case '[': case '{':
        pos++;
        node->type = EXPR;
        node->chld = ka_parse(text);
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

void ka_init(struct KaNode **env) {
  ka_fn("=", ka_set, env);
}
