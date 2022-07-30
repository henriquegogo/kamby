typedef enum { NONE, NUM, IDF, STR, EXPR } KaType;

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

struct KaNode *ka_set(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_get(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_eval(struct KaNode *node, struct KaNode **env);

struct KaNode *ka_parse(char *text);

void ka_fn(char *key, struct KaNode *(*fn)(), struct KaNode **env);

void ka_init(struct KaNode **env);
