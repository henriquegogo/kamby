#ifndef KAMBY_H
#define KAMBY_H

typedef enum {
  KA_NONE, KA_INIT, KA_NUM, KA_KEY, KA_STR, KA_EXPR, KA_BLCK, KA_LIST
} KaType;

struct KaNode {
  KaType type;
  char *key;
  union {
    long long num;
    char *str;
    struct KaNode *val;
    struct KaNode *(*fun)(struct KaNode *node, struct KaNode **env);
  };
  struct KaNode *next;
};

struct KaNode *ka_new();

struct KaNode *ka_num(long long num);

struct KaNode *ka_str(char *str);

struct KaNode *ka_key(char *key);

struct KaNode *ka_fun(struct KaNode *(*fun)(struct KaNode *node, struct KaNode **env));

struct KaNode *ka_lnk(struct KaNode *node, ...);

struct KaNode *ka_cpy(struct KaNode *dest, struct KaNode *orig, struct KaNode *next);

struct KaNode *ka_def(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_get(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_del(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_parser(char *text, struct KaNode **pos);

struct KaNode *ka_init();

void ka_free(struct KaNode **node);

#endif
