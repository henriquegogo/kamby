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

struct KaNode *ka_idf(char *str, struct KaNode *next) {
  struct KaNode *output = malloc(KANODE_SIZE);
  output->type = KA_IDF;
  output->str = malloc(strlen(str));
  strcpy(output->str, str);
  output->next = next;
  return output;
}

struct KaNode *ka_fn(struct KaNode *(*fn)()) {
  struct KaNode *output = malloc(KANODE_SIZE);
  output->fn = fn;
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
struct KaNode *ka_def(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = malloc(KANODE_SIZE);
  memcpy(reg, node->next, KANODE_SIZE);
  // node->key exists if was returned from registers. If not, is a new register.
  reg->key = node->key ? node->key : node->str;
  reg->next = *env;
  *env = reg;
  return node->next;
}

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = *env;
  if (!node->key) return ka_def(node, env);
  while (reg) {
    if (strcmp(node->key, reg->key) == 0) {
      reg->type = node->next->type;
      switch (reg->type) {
        case KA_EXPR: case KA_BLCK: case KA_LIST:
          memcpy(reg->chld, node->next->chld, KANODE_SIZE);
          break;
        case KA_STR: case KA_IDF:
          strcpy(reg->str, node->next->str);
          break;
        default:
          reg->num = node->next->num;
      }
      break;
    }
    reg = reg->next;
  }
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

// Conditions and loops
struct KaNode *ka_if(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = *env;
  while (node) {
    if (node->num) {
      node = node->next;
      return node->type == KA_BLCK ? ka_eval(node->chld, &local) : node;
    }
    node = node->next->next;
  }
  return malloc(KANODE_SIZE);
}

struct KaNode *ka_while(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = *env;
  while (ka_eval(node->chld, &local)->num) {
    ka_eval(node->next->chld, &local);
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
        memcpy(value, node->chld, KANODE_SIZE);
        value = ka_eval(value, env);
        break;
      case KA_IDF:
        value = ka_get(node, env);
        if (!value->type) memcpy(value, node, KANODE_SIZE);
        break;
      default:
        memcpy(value, node, KANODE_SIZE);
    }
    tail->next = value;
    tail = tail->next;
    node = node->next;
  }

  head = head->next;
  switch (head->type) {
    case KA_IDF:
      return ka_get(head, env)->fn(head->next, env);
    case KA_BLCK:
      return ka_eval(head->chld, env);
    default:
      return head;
  }
}

struct KaNode *ka_parse(char *text, struct KaNode **pos) {
  int length = strlen(text);
  struct KaNode *head = malloc(KANODE_SIZE);
  struct KaNode *tail = head;

  // Every line will be wrapped by an expression
  while (!(*pos)->type) {
    if (!(*pos)->type) (*pos)->type = KA_EXPR;
    struct KaNode *expr = malloc(KANODE_SIZE);
    expr->chld = ka_parse(text, pos);
    if (expr->chld) {
      expr->type = KA_EXPR;
      tail->next = expr;
      tail = tail->next;
    }
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

  ka_def(ka_idf("+", ka_fn(ka_add)), &env);
  ka_def(ka_idf("-", ka_fn(ka_sub)), &env);
  ka_def(ka_idf("*", ka_fn(ka_mul)), &env);
  ka_def(ka_idf("/", ka_fn(ka_div)), &env);
  ka_def(ka_idf("&&", ka_fn(ka_and)), &env);
  ka_def(ka_idf("||", ka_fn(ka_or)), &env);
  ka_def(ka_idf("==", ka_fn(ka_eq)), &env);
  ka_def(ka_idf("!=", ka_fn(ka_not)), &env);
  ka_def(ka_idf("<", ka_fn(ka_lt)), &env);
  ka_def(ka_idf("<=", ka_fn(ka_lte)), &env);
  ka_def(ka_idf(">", ka_fn(ka_gt)), &env);
  ka_def(ka_idf(">=", ka_fn(ka_gte)), &env);
  ka_def(ka_idf("def", ka_fn(ka_def)), &env);
  ka_def(ka_idf(":=", ka_fn(ka_def)), &env);
  ka_def(ka_idf("=", ka_fn(ka_set)), &env);
  ka_def(ka_idf("del", ka_fn(ka_del)), &env);
  ka_def(ka_idf("if", ka_fn(ka_if)), &env);
  ka_def(ka_idf("?", ka_fn(ka_if)), &env);
  ka_def(ka_idf("while", ka_fn(ka_while)), &env);

  ka_def(ka_idf("false", ka_num(0)), &env);

  return env;
}
