#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "kamby.h"

unsigned long long uuid = 0;

// Constructors
struct KaNode *ka_new() {
  return calloc(1, sizeof(struct KaNode));
}

struct KaNode *ka_num(long long num) {
  struct KaNode *output = ka_new();
  output->type = KA_NUM;
  output->num = num;
  return output;
}

struct KaNode *ka_str(char *str) {
  struct KaNode *output = ka_new();
  output->type = KA_STR;
  output->str = str;
  return output;
}

struct KaNode *ka_key(char *key) {
  struct KaNode *output = ka_new();
  output->type = KA_KEY;
  output->key = key;
  output->str = key;
  return output;
}

struct KaNode *ka_fun(struct KaNode *(*fun)(struct KaNode *node, struct KaNode **env)) {
  struct KaNode *output = ka_new();
  output->fun = fun;
  return output;
}

struct KaNode *ka_lnk(struct KaNode *node, ...) {
  struct KaNode *tail = node;
  va_list args;
  va_start(args, node);
  while (tail) tail = tail->next = va_arg(args, struct KaNode *);
  va_end(args);
  return node;
}

struct KaNode *ka_cpy(struct KaNode *dest, struct KaNode *orig, struct KaNode *next) {
  dest->type = orig->type;
  dest->key = orig->key;
  dest->val = orig->val;
  dest->next = next;
  return dest;
}

// Definitions and memory control
struct KaNode *ka_def(struct KaNode *node, struct KaNode **env) {
  node->next->key = node->key ? node->key : node->str;
  (*env)->next = ka_cpy(ka_new(), node->next, (*env)->next);
  struct KaNode *output = node->next;
  free(node);
  return output;
}

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = *env;
  if (!node->key && node->type == KA_KEY) return ka_def(node, env);
  while (reg && strcmp(node->key, reg->key ? reg->key : "") != 0)
    reg = reg->next;
  node->next->key = reg->key;
  node->next->next = reg->next;
  ka_cpy(reg, node->next, node->next->next);
  node->next->next = NULL;
  return node->next;
}

struct KaNode *ka_get(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = *env;
  while (reg && strcmp(node->str, reg->key ? reg->key : "") != 0) {
    if (reg->type == KA_INIT) return ka_new();
    reg = reg->next;
  }
  return ka_cpy(ka_new(), reg, NULL);
}

struct KaNode *ka_del(struct KaNode *node, struct KaNode **env) {
  struct KaNode *reg = *env;
  if (node->key) {
    while (strcmp(node->key, reg->next->key ? reg->next->key : "") != 0)
      reg = reg->next;
    if (reg->next) ka_cpy(reg->next, reg->next->next, reg->next->next->next);
  }
  return node;
}

// Context methods
struct KaNode *ka_stack(struct KaNode *node, struct KaNode **env) {
  struct KaNode *output = ka_new();
  struct KaNode *reg = (*env)->next;
  for (int i = 0; reg->type && node && i < node->num - 1; i++) reg = reg->next;
  if (reg && (!node || node->num)) {
    if (!reg->key) sprintf(reg->key = calloc(1, sizeof(uuid)), "#%lld", uuid++);
    ka_cpy(output, reg, NULL);
  }
  free(node);
  return output;
}

struct KaNode *ka_call(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = ka_new();
  struct KaNode *tail = local->next = node->val;
  while (tail->next) tail = tail->next;
  tail->next = *env;
  struct KaNode *output = ka_eval(node->next, &local);
  tail->next = NULL;
  free(node);
  free(local);
  return output;
}

// Conditions and loops
struct KaNode *ka_if(struct KaNode *node, struct KaNode **env) {
  while (node) {
    if (node->num) {
      node = node->next;
      node->next = NULL;
      return node->type == KA_BLCK ? ka_eval(node->val, env) : node;
    }
    node = node->next->next;
  }
  return node;
}

struct KaNode *ka_while(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = ka_cpy(ka_new(), *env, (*env)->next);
  while (ka_eval(node->val, &local)->num)
    ka_eval(node->next->val, &local);
  free(local);
  return node;
}

struct KaNode *ka_for(struct KaNode *node, struct KaNode **env) {
  struct KaNode *local = ka_cpy(ka_new(), *env, (*env)->next);
  for (ka_eval(node->val, &local);
      ka_eval(node->next->val, &local)->num;
      ka_eval(node->next->next->val, &local)) {
    ka_eval(node->next->next->next->val, &local);
  }
  free(local);
  return node;
}

// Math and Logical operators
struct KaNode *ka_add(struct KaNode *node, struct KaNode **env) {
  if (node->type == KA_STR && node->next->type == KA_STR) {
    int size = strlen(node->str) + strlen(node->next->str);
    return ka_str(strcat(strcpy(calloc(1, size), node->str), node->next->str));
  } else if (node->type == KA_NUM && node->next->type == KA_NUM) {
    return ka_num(node->num + node->next->num);
  }
  return node;
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
    struct KaNode *tail = node->val;
    while (tail->next) tail = tail->next;
    tail->next = node->next->type == KA_LIST ? node->next->val : node->next;
    return node;
  }
  return ka_set(ka_lnk(node, ka_add(node, env), 0), env);
}

struct KaNode *ka_decr(struct KaNode *node, struct KaNode **env) {
  return ka_set(ka_lnk(node, ka_sub(node, env), 0), env);
}

// Parser and interpreter
struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env) {
  struct KaNode *head = ka_new();
  struct KaNode *tail = head;
  struct KaNode *local = ka_cpy(ka_new(), *env, (*env)->next);
  
  // Eval expressions and get variables
  while (node) {
    struct KaNode *value = ka_new();
    switch (node->type) {
      case KA_EXPR:
        value = ka_eval(node->val, env);
        break;
      case KA_LIST:
        node->val = ka_eval(node->val, &local);
      case KA_KEY:
        value = ka_get(node, env);
        if (value->type) break;
      default:
        ka_cpy(value, node, node->next);
    }
    tail->next = value;
    tail = tail->next;
    node = node->next;
  }

  // Check first item from expression and do some action based on this
  head = head->next;
  switch (head->type) {
    case KA_KEY:
      node = ka_get(head, env);
      if (node->fun) {
        free(local);
        return node->fun(head->next, env);
      } else break;
    case KA_BLCK:
      while (head->next) {
        ka_def(ka_lnk(ka_key(""), ka_eval(head->next, &local), 0), &local);
        head->next = head->next->next;
      }
      tail = ka_eval(head->val, &local);
      while (tail->next) tail = tail->next;
      head = tail;
    default:;
  }
  free(node);
  free(local);
  return head;
}

struct KaNode *ka_parser(char *text, struct KaNode **pos) {
  int length = strlen(text);
  struct KaNode *head = ka_new();
  struct KaNode *tail = head;

  // Every line will be wrapped by an expression
  while (!(*pos)->type) {
    if (!(*pos)->type) (*pos)->type = KA_EXPR;
    struct KaNode *expr = ka_new();
    expr->val = ka_parser(text, pos);
    if (expr->val) {
      expr->type = KA_EXPR;
      tail->next = expr;
      tail = tail->next;
    } else {
      free(expr);
    }
  }

  while ((*pos)->num < length) {
    int start = (*pos)->num;
    struct KaNode *node = ka_new();

    switch (text[(*pos)->num]) {
      case ' ': break;
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
        node->val = ka_parser(text, pos);
        break;
      case '\n': case ';':
        (*pos)->num++;
        (*pos)->type = KA_NONE;
      case ')': case ']': case '}':
        length = 0;
        continue;
      case '\'': case '"':
        while (text[++(*pos)->num] != text[start]);
        node->type = KA_STR;
        node->str = calloc(1, (*pos)->num - start);
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
          node->type = KA_KEY;
          node->str = calloc(1, (*pos)->num - start + 1);
          strncpy(node->str, text + start, (*pos)->num - start);
          (*pos)->num--;
        }
    };

    if (node->type) {
      tail->next = node;
      tail = tail->next;
    } else {
      free(node);
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
    if (op->type == KA_KEY && strlen(op->str) < 3 && ispunct(op->str[0])) {
      tail->next = ka_new();
      tail->next->type = KA_EXPR;
      tail->next->val = ka_lnk(op, a, b, 0);
      tail->next->next = next;
      continue;
    }
    tail = tail->next;
  }
  
  return head->next;
}

struct KaNode *ka_init() {
  struct KaNode *env = ka_new();
  env->next = ka_new();
  env->next->type = KA_INIT;

  ka_def(ka_lnk(ka_key("def"),   ka_fun(ka_def),   0), &env);
  ka_def(ka_lnk(ka_key(":="),    ka_fun(ka_def),   0), &env);
  ka_def(ka_lnk(ka_key("="),     ka_fun(ka_set),   0), &env);
  ka_def(ka_lnk(ka_key("del"),   ka_fun(ka_del),   0), &env);
  ka_def(ka_lnk(ka_key("if"),    ka_fun(ka_if),    0), &env);
  ka_def(ka_lnk(ka_key("while"), ka_fun(ka_while), 0), &env);
  ka_def(ka_lnk(ka_key("for"),   ka_fun(ka_for),   0), &env);
  ka_def(ka_lnk(ka_key("."),     ka_fun(ka_stack), 0), &env);
  ka_def(ka_lnk(ka_key("::"),    ka_fun(ka_call),  0), &env);

  ka_def(ka_lnk(ka_key("+"),  ka_fun(ka_add),  0), &env);
  ka_def(ka_lnk(ka_key("-"),  ka_fun(ka_sub),  0), &env);
  ka_def(ka_lnk(ka_key("*"),  ka_fun(ka_mul),  0), &env);
  ka_def(ka_lnk(ka_key("/"),  ka_fun(ka_div),  0), &env);
  ka_def(ka_lnk(ka_key("&&"), ka_fun(ka_and),  0), &env);
  ka_def(ka_lnk(ka_key("||"), ka_fun(ka_or),   0), &env);
  ka_def(ka_lnk(ka_key("=="), ka_fun(ka_eq),   0), &env);
  ka_def(ka_lnk(ka_key("!="), ka_fun(ka_not),  0), &env);
  ka_def(ka_lnk(ka_key("<"),  ka_fun(ka_lt),   0), &env);
  ka_def(ka_lnk(ka_key("<="), ka_fun(ka_lte),  0), &env);
  ka_def(ka_lnk(ka_key(">"),  ka_fun(ka_gt),   0), &env);
  ka_def(ka_lnk(ka_key(">="), ka_fun(ka_gte),  0), &env);
  ka_def(ka_lnk(ka_key("+="), ka_fun(ka_incr), 0), &env);
  ka_def(ka_lnk(ka_key("-="), ka_fun(ka_decr), 0), &env);

  ka_def(ka_lnk(ka_key("true"),  ka_num(1), 0), &env);
  ka_def(ka_lnk(ka_key("false"), ka_num(0), 0), &env);

  return env;
}

void ka_free(struct KaNode **node) {
  while (*node) {
    struct KaNode *prev = *node;
    *node = (*node)->next;
    free(prev);
  }
  free(*node);
}
