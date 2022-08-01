typedef enum { KA_NONE, KA_NUM, KA_IDF, KA_STR, KA_EXPR, KA_LIST } KaType;

struct KaNode {
  KaType type;
  char *key;
  union {
    long num;
    char *str;
    struct KaNode *chld;
  };
  struct KaNode *(*fn)();
  struct KaNode *next;
};

struct KaNode *ka_fn(char *key, struct KaNode *(*fn)(), struct KaNode **env);

struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_parse(char *text, struct KaNode **pos);

struct KaNode *ka_init();
