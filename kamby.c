#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

#define KANODE_SIZE sizeof(struct KaNode)

// Constructors
struct KaNode *ka_num(long long num) {
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

struct KaNode *ka_idf(char *str) {
  struct KaNode *output = malloc(KANODE_SIZE);
  output->type = KA_IDF;
  output->key = malloc(strlen(str));
  output->str = malloc(strlen(str));
  strcpy(output->key, str);
  strcpy(output->str, str);
  return output;
}

struct KaNode *ka_fn(struct KaNode *(*fn)()) {
  struct KaNode *output = malloc(KANODE_SIZE);
  output->fn = fn;
  return output;
}

struct KaNode *ka_link(struct KaNode *node, ...) {
  struct KaNode *tail = node;
  va_list args;
  va_start(args, node);
  while (tail) {
    tail->next = va_arg(args, struct KaNode *);
    tail = tail->next;
  }
  va_end(args);
  return node;
}

struct KaNode *ka_expr(struct KaNode *node) {
  struct KaNode *output = malloc(KANODE_SIZE);
  output->type = KA_EXPR;
  output->chld = node;
  return output;
}

// Definitions and memory control
struct KaNode *ka_def(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = malloc(KANODE_SIZE);
  char *key = node->key ? node->key : node->str;
  node->next->key = malloc(sizeof(key));
  strcpy(node->next->key, key);
  memcpy(reg, node->next, KANODE_SIZE);
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
  node->next->key = malloc(sizeof(node->key));
  strcpy(node->next->key, node->key);
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

// List methods
struct KaNode *ka_len(struct KaNode *node, struct KaNode **env) {
  int i = 0;
  switch (node->type) {
    case KA_NUM:
      return node;
    case KA_STR:
      return ka_num(strlen(node->str));
    default:
      for (; node->chld; i++) node->chld = node->chld->next;
      return ka_num(i);
  }
}

struct KaNode *ka_item(struct KaNode *node, struct KaNode **env) {
  struct KaNode *output = malloc(KANODE_SIZE);
  // String
  if (node->type == KA_STR) {
    output = ka_str(&node->str[node->next->num - 1]);
    output->str[1] = '\0';
    return output;
  }
  // List
  struct KaNode *chld = node->chld;
  for (int i = 0; chld->next && i < node->next->num - 1; i++) chld = chld->next;
  memcpy(output, chld, KANODE_SIZE);
  output->next = NULL;
  return output;
}

struct KaNode *ka_call(struct KaNode *node, struct KaNode **env) {
  struct KaNode *tail = node->chld;
  while (tail->next) tail = tail->next;
  tail->next = *env;
  struct KaNode *output = ka_eval(node->next, &node->chld);
  tail->next = NULL;
  return output;
}

// Conditions and loops
struct KaNode *ka_if(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = *env;
  while (node) {
    if (node->num) {
      node = node->next;
      node->next = NULL;
      return node->type == KA_BLCK ? ka_eval(node->chld, &local) : node;
    }
    node = node->next->next;
  }
  return malloc(KANODE_SIZE);
}

struct KaNode *ka_while(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = *env;
  while (ka_eval(node->chld, &local)->num)
    ka_eval(node->next->chld, &local);
  return malloc(KANODE_SIZE);
}

struct KaNode *ka_for(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = *env;
  for (ka_eval(node->chld, &local);
      ka_eval(node->next->chld, &local)->num;
      ka_eval(node->next->next->chld, &local)) {
    ka_eval(node->next->next->next->chld, &local);
  }
  return malloc(KANODE_SIZE);
}

// Math and Logical operators
struct KaNode *ka_add(struct KaNode *node, struct KaNode **env) {
  if (node->type == KA_STR && node->next->type == KA_STR) {
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
  if (node->type != KA_NUM && ka_num(node->num && node->next->num))
    return node->next;
  return ka_num(node->num && node->next->num);
}

struct KaNode *ka_or(struct KaNode *node, struct KaNode **env) {
  if (node->type != KA_NUM && ka_num(node->num || node->next->num))
    return node->num ? node : node->next;
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

struct KaNode *ka_incr(struct KaNode *node, struct KaNode **env) {
  if (node->type == KA_LIST) {
    struct KaNode *tail = node->chld;
    while (tail->next) tail = tail->next;
    tail->next = node->next->type == KA_LIST ? node->next->chld : node->next;
    return node;
  }
  return ka_set(ka_link(node, ka_add(node, env), 0), env);
}

struct KaNode *ka_decr(struct KaNode *node, struct KaNode **env) {
  return ka_set(ka_link(node, ka_sub(node, env), 0), env);
}

// Parser and interpreter
struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env) {
  struct KaNode *head = malloc(KANODE_SIZE);
  struct KaNode *tail = head;
  struct KaNode *local = *env;
  
  // Eval expressions and get variables
  while (node) {
    struct KaNode *value = malloc(KANODE_SIZE);
    switch (node->type) {
      case KA_EXPR:
        memcpy(value, node->chld, KANODE_SIZE);
        value = ka_eval(value, env);
        break;
      case KA_LIST:
        node->chld = ka_eval(node->chld, &local);
        memcpy(value, node, KANODE_SIZE);
        break;
      case KA_IDF:
        value = ka_get(node, env);
        if (value->type) break;
      default:
        memcpy(value, node, KANODE_SIZE);
    }
    tail->next = value;
    tail = tail->next;
    node = node->next;
  }

  // Check first item from expression and do some action based on this
  head = head->next;
  switch (head->type) {
    case KA_IDF:
      node = ka_get(head, env);
      if (node->fn) return node->fn(head->next, env);
      else break;
    case KA_BLCK:
      if (head->next) {
        if (head->next->type == KA_BLCK) head->next = head->next->chld;
        ka_def(ka_link(ka_idf("arg"), ka_eval(head->next, &local), 0), &local);
      }
      tail = ka_eval(head->chld, &local);
      while (tail->next) tail = tail->next;
      return tail;
    default:;
  }
  return head;
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
        node->type = KA_BLCK;
      case '[':
        if (!node->type) node->type = KA_LIST;
        (*pos)->type = KA_NONE;
      case '(':
        if (!node->type) node->type = KA_EXPR;
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
          (*pos)->num--;
        }
    };

    if (node->type) {
      tail->next = node;
      tail = tail->next;
    }

    (*pos)->num++;
  };

  // Change a->punct->b to punct->a->b and wrap in an expression
  tail = head;
  while (tail && tail->next && tail->next->next && tail->next->next->next) {
    struct KaNode *a = tail->next;
    struct KaNode *op = tail->next->next;
    struct KaNode *b = tail->next->next->next;
    struct KaNode *next = tail->next->next->next->next;
    if (op->type == KA_IDF && !op->str[2] && ispunct(op->str[0])) {
      tail->next = ka_expr(ka_link(op, a, b, 0));
      tail->next->next = next;
      continue;
    }
    tail = tail->next;
  }
  
  return head->next;
}

struct KaNode *ka_init() {
  struct KaNode *env = malloc(KANODE_SIZE);

  ka_def(ka_link(ka_idf("def"),ka_fn(ka_def),0),&env);
  ka_def(ka_link(ka_idf(":="), ka_fn(ka_def),0),&env);
  ka_def(ka_link(ka_idf("="),  ka_fn(ka_set),0),&env);
  ka_def(ka_link(ka_idf("del"),ka_fn(ka_del),0),&env);
  ka_def(ka_link(ka_idf("if"), ka_fn(ka_if), 0),&env);
  ka_def(ka_link(ka_idf("while"), ka_fn(ka_while), 0), &env);
  ka_def(ka_link(ka_idf("for"),ka_fn(ka_for),0),&env);
  ka_def(ka_link(ka_idf("len"),ka_fn(ka_len),0),&env);
  ka_def(ka_link(ka_idf("."),  ka_fn(ka_item), 0),&env);
  ka_def(ka_link(ka_idf("::"), ka_fn(ka_call), 0),&env);

  ka_def(ka_link(ka_idf("+"), ka_fn(ka_add),0), &env);
  ka_def(ka_link(ka_idf("-"), ka_fn(ka_sub),0), &env);
  ka_def(ka_link(ka_idf("*"), ka_fn(ka_mul),0), &env);
  ka_def(ka_link(ka_idf("/"), ka_fn(ka_div),0), &env);
  ka_def(ka_link(ka_idf("&&"),ka_fn(ka_and),0), &env);
  ka_def(ka_link(ka_idf("||"),ka_fn(ka_or), 0), &env);
  ka_def(ka_link(ka_idf("=="),ka_fn(ka_eq), 0), &env);
  ka_def(ka_link(ka_idf("!="),ka_fn(ka_not),0), &env);
  ka_def(ka_link(ka_idf("<"), ka_fn(ka_lt), 0), &env);
  ka_def(ka_link(ka_idf("<="),ka_fn(ka_lte),0), &env);
  ka_def(ka_link(ka_idf(">"), ka_fn(ka_gt), 0), &env);
  ka_def(ka_link(ka_idf(">="),ka_fn(ka_gte),0), &env);
  ka_def(ka_link(ka_idf("+="),ka_fn(ka_incr),0),&env);
  ka_def(ka_link(ka_idf("-="),ka_fn(ka_decr),0),&env);

  ka_def(ka_link(ka_idf("true"), ka_num(1), 0),&env);
  ka_def(ka_link(ka_idf("false"),ka_num(0), 0),&env);

  return env;
}
