#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

#define KANODE_SIZE sizeof(struct KaNode)

// Atom constructors
struct KaNode *ka_num(long num) {
  struct KaNode *output = malloc(KANODE_SIZE);
  output->type = KA_NUM;
  output->num = num;
  return output;
}

struct KaNode *ka_str(char *str) {
  struct KaNode *output = malloc(KANODE_SIZE);
  output->type = KA_STR;
  output->str = malloc(strlen(str));
  strcpy(output->str, str);
  return output;
}

// Math and Logical operators
struct KaNode *ka_add(struct KaNode *node, struct KaNode **env) {
  if (node->type == KA_STR) {
    struct KaNode *output = ka_str(node->str);
    strcat(output->str, node->next->str);
    return output;
  }
  return ka_num(node->num + node->next->num);
}

struct KaNode *ka_sub(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num - node->next->num);
}

struct KaNode *ka_mul(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num * node->next->num);
}

struct KaNode *ka_div(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num / node->next->num);
}

struct KaNode *ka_and(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num && node->next->num);
}

struct KaNode *ka_or(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num || node->next->num);
}

struct KaNode *ka_eq(struct KaNode *node, struct KaNode **env) {
  if (node->type == KA_STR)
    return ka_num(strcmp(node->str, node->next->str) == 0);
  return ka_num(node->num == node->next->num);
}

struct KaNode *ka_not(struct KaNode *node, struct KaNode **env) {
  if (node->type == KA_STR)
    return ka_num(strcmp(node->str, node->next->str) != 0);
  return ka_num(node->num != node->next->num);
}

struct KaNode *ka_lt(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num < node->next->num);
}

struct KaNode *ka_lte(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num <= node->next->num);
}

struct KaNode *ka_gt(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num > node->next->num);
}

struct KaNode *ka_gte(struct KaNode *node, struct KaNode **env) {
  return ka_num(node->num >= node->next->num);
}

// Definitions and memory control
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
  return malloc(KANODE_SIZE);
}

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env) {
  ka_del(node, env); // If setting an existing register, delete and create new.
  struct KaNode *reg = malloc(KANODE_SIZE);
  memcpy(reg, node->next, KANODE_SIZE);
  // node->key exists if was returned from registers. If not, is a new register.
  reg->key = node->key ? node->key : node->str;
  reg->next = *env;
  *env = reg;
  return node->next;
}

struct KaNode *ka_get(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = *env;
  while (reg->next && strcmp(node->str, reg->key) != 0) reg = reg->next;
  struct KaNode *output = malloc(KANODE_SIZE);
  memcpy(output, reg, KANODE_SIZE);
  output->next = NULL;
  return output;
}

struct KaNode *ka_fn(char *key, struct KaNode *(*fn)(), struct KaNode **env) {
  struct KaNode *reg = malloc(KANODE_SIZE);
  reg->next = *env;
  reg->key = key;
  reg->fn = fn;
  return *env = reg;
}

struct KaNode *ka_if(struct KaNode *node, struct KaNode **env) {
  if (node->num) {
    return ka_eval(node->next->chld, env);
  } else if (node->next->next->num) {
    return ka_eval(node->next->next->next->chld, env);
  }
  return malloc(KANODE_SIZE);
}

// Parser and interpreter
struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env) {
  struct KaNode *head = malloc(KANODE_SIZE);
  struct KaNode *tail = head;
  
  while (node) {
    struct KaNode *value = malloc(KANODE_SIZE);
    switch (node->type) {
      case KA_EXPR:
        tail->next = ka_eval(node->chld, env);
        break;
      case KA_IDF:
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
  if (head && head->type == KA_IDF) {
    return ka_get(head, env)->fn(head->next, env);
  }

  return head;
}

struct KaNode *ka_parse(char *text, struct KaNode **pos) {
  int length = strlen(text);
  struct KaNode *head = malloc(KANODE_SIZE);
  struct KaNode *tail = head;

  // Trim initial breaklines
  while ((*pos)->num < length && text[(*pos)->num] == '\n') (*pos)->num++;

  // Every line will be wrapped by and expression
  while (!(*pos)->type) {
    if (!(*pos)->type) (*pos)->type = KA_EXPR;
    tail->next = malloc(KANODE_SIZE);
    tail->next->type = KA_EXPR;
    tail->next->chld = ka_parse(text, pos);
    tail = tail->next;
  }

  while ((*pos)->num < length) {
    int start = (*pos)->num;
    struct KaNode *node = malloc(KANODE_SIZE);

    switch (text[(*pos)->num]) {
      case ' ':
        break;
      case '#':
        while (text[(*pos)->num + 1] != '\n') (*pos)->num++;
        break;
      case '{':
        (*pos)->type = KA_NONE;
        node->type = KA_BLCK;
      case '(':
        if (!node->type) node->type = KA_EXPR;
      case '[':
        if (!node->type) node->type = KA_LIST;
        (*pos)->num++;
        node->chld = ka_parse(text, pos);
        break;
      case '\n':
      case ';':
        (*pos)->num++;
        (*pos)->type = KA_NONE;
      case ')': case ']': case '}':
        length = 0;
        continue;
      case '\'':
      case '"':
        while (text[++(*pos)->num] != text[start]);
        node->type = KA_STR;
        node->str = malloc((*pos)->num - start - 1);
        strncpy(node->str, text + start + 1, (*pos)->num - start - 1);
        break;
      default:
        if (isdigit(text[(*pos)->num])) {
          while (isdigit(text[(*pos)->num + 1])) (*pos)->num++;
          node->type = KA_NUM;
          node->num = atoi(text + start);
        } else if (isgraph(text[(*pos)->num])) {
          while (isgraph(text[(*pos)->num]) && text[(*pos)->num] != ';' &&
              text[(*pos)->num] != '(' && text[(*pos)->num] != ')' &&
              text[(*pos)->num] != '{' && text[(*pos)->num] != '}' &&
              text[(*pos)->num] != '[' && text[(*pos)->num] != ']')
            (*pos)->num++;
          node->type = KA_IDF;
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

struct KaNode *ka_init() {
  struct KaNode *env = malloc(KANODE_SIZE);
  ka_fn("+", ka_add, &env);
  ka_fn("-", ka_sub, &env);
  ka_fn("*", ka_mul, &env);
  ka_fn("/", ka_div, &env);
  ka_fn("&&", ka_and, &env);
  ka_fn("||", ka_or, &env);
  ka_fn("==", ka_eq, &env);
  ka_fn("!=", ka_not, &env);
  ka_fn("<", ka_lt, &env);
  ka_fn("<=", ka_lte, &env);
  ka_fn(">", ka_gt, &env);
  ka_fn(">=", ka_gte, &env);
  ka_fn("=", ka_set, &env);
  ka_fn("del", ka_del, &env);
  ka_fn("if", ka_if, &env);
  return env;
}
