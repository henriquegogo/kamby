#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

struct KaNode *ka_del(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = *env;
  if (!node->key); // Node is not a registered variable. Do nothing.
  else if (strcmp(node->key, reg->key) == 0) *env = (*env)->next;
  else {
    while (reg->next) {
      if (strcmp(node->key, reg->next->key) == 0) {
        reg->next = reg->next->next;
        break;
      }
      reg = reg->next;
    }
  }
  return malloc(sizeof(struct KaNode));
}

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env) {
  ka_del(node, env); // If setting an existing register, delete and create new.
  struct KaNode *reg = malloc(sizeof(struct KaNode));
  memcpy(reg, node->next, sizeof(struct KaNode));
  reg->key = node->key ? node->key : node->str;
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

struct KaNode *ka_parse(char *text, struct KaNode **pos) {
  int length = strlen(text);
  struct KaNode *head = malloc(sizeof(struct KaNode));
  struct KaNode *tail = head;

  while (!(*pos)->type) {
    if (!(*pos)->type) (*pos)->type = EXPR;
    tail->next = malloc(sizeof(struct KaNode));
    tail->next->type = EXPR;
    tail->next->chld = ka_parse(text, pos);
    tail = tail->next;
  }

  while ((*pos)->num < length) {
    int start = (*pos)->num;
    struct KaNode *node = malloc(sizeof(struct KaNode));

    switch (text[(*pos)->num]) {
      case ' ':
        break;
      case '#':
        while (text[(*pos)->num + 1] != '\n') (*pos)->num++;
        break;
      case '(': case '[': case '{':
        (*pos)->num++;
        node->type = EXPR;
        node->chld = ka_parse(text, pos);
        break;
      case '\n':
      case ';':
        (*pos)->num++;
        (*pos)->type = NONE;
      case ')': case ']': case '}':
        length = 0;
        continue;
      case '\'':
      case '"':
        while (text[++(*pos)->num] != text[start]);
        node->type = STR;
        node->str = malloc((*pos)->num - start - 1);
        strncpy(node->str, text + start + 1, (*pos)->num - start - 1);
        break;
      default:
        if (isdigit(text[(*pos)->num])) {
          while (isdigit(text[(*pos)->num + 1])) (*pos)->num++;
          node->type = NUM;
          node->num = atoi(text + start);
        } else if (isgraph(text[(*pos)->num])) {
          while (isgraph(text[(*pos)->num]) && text[(*pos)->num] != ';' &&
              text[(*pos)->num] != '(' && text[(*pos)->num] != ')' &&
              text[(*pos)->num] != '{' && text[(*pos)->num] != '}' &&
              text[(*pos)->num] != '[' && text[(*pos)->num] != ']')
            (*pos)->num++;
          node->type = IDF;
          node->str = malloc((*pos)->num - start);
          strncpy(node->str, text + start, (*pos)->num - start);
          // (2 + 4) -> (+ 2 4) ... (true == true) -> (== true true)
          if (head->next == tail && !node->str[2] && ispunct(text[start])) {
            node->next = head->next;
            head->next = node;
            continue;
          }
          (*pos)->num--;
        }
    };

    if (node->type) {
      tail->next = node;
      tail = tail->next;
    }

    (*pos)->num++;
  };
  
  return head->next;
}

void ka_init(struct KaNode **env) {
  ka_fn("=", ka_set, env);
  ka_fn("del", ka_del, env);
}
