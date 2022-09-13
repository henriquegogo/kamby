#ifndef KAMBY_H
#define KAMBY_H

#define KANODE_SIZE sizeof(struct KaNode)

typedef enum {
  KA_NONE, KA_NUM, KA_IDF, KA_STR, KA_EXPR, KA_BLCK, KA_LIST
} KaType;

struct KaNode {
  KaType type;
  char *key;
  union {
    long long num;
    char *str;
    struct KaNode *chld;
    struct KaNode *(*fn)();
  };
  struct KaNode *next;
};

struct KaNode *ka_num(long long num);

struct KaNode *ka_str(char *str);

struct KaNode *ka_idf(char *str);

struct KaNode *ka_fn(struct KaNode *(*fn)());

struct KaNode *ka_link(struct KaNode *node, ...);

struct KaNode *ka_def(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_get(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_del(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_parser(char *text, struct KaNode **pos);

struct KaNode *ka_init();

void ka_free(struct KaNode *node);

#endif
