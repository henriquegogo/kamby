#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "kamby.h"

int print_level = -1;

void print_chain(KaNode *chain);

void print_node(KaNode *node)
{
    if (!node) return;

    const char *types[] = {
        "none", "ctx", "false", "true", "number", "string",
        "symbol", "func", "list", "expr", "block"
    };

    for (int i = 0; i < print_level; i++) {
        printf("  ");
    }

    if (node->key) {
        printf("(%s) %s ", types[node->type], node->key);
    } else {
        printf("(%s) ", types[node->type]);
    }

    if (node->type == KA_NUMBER) {
        printf("%.2Lf\n", *node->number);
    } else if (node->type == KA_STRING) {
        printf("\"%s\"\n", node->string);
    } else if (node->type == KA_SYMBOL) {
        printf("%s\n", node->symbol);
    } else if (node->type == KA_FUNC) {
        printf("%p\n", node->func);
    } else if (node->type >= KA_LIST) {
        printf("\n");
        print_chain(node->children);
    } else {
        printf("\n");
    }
}

void print_chain(KaNode *chain)
{
    print_level++;

    for (KaNode *curr = chain; curr; curr = curr->next) {
        print_node(curr);
    }

    print_level--;
}

void test_new()
{
    KaNode *node = ka_new(KA_NONE);

    assert(node->type == KA_NONE);
    assert(node->key == NULL);
    assert(node->value == NULL);
    assert(node->next == NULL);

    ka_free(node);
}

void test_chain()
{
    KaNode *node = ka_chain(
        ka_new(KA_NUMBER),
        ka_chain(
            ka_new(KA_SYMBOL),
            ka_new(KA_SYMBOL), NULL),
        ka_chain(
            ka_new(KA_NUMBER),
            ka_new(KA_STRING), NULL),
        ka_new(KA_SYMBOL), NULL
    );

    assert(node->type == KA_NUMBER);
    assert(node->next->type == KA_SYMBOL);
    assert(node->next->next->type == KA_SYMBOL);
    assert(node->next->next->next->type == KA_NUMBER);
    assert(node->next->next->next->next->type == KA_STRING);
    assert(node->next->next->next->next->next->type == KA_SYMBOL);
    assert(node->next->next->next->next->next->next == NULL);

    ka_free(node);
}

void test_ctx()
{
    KaNode *node = ka_new(KA_CTX);

    assert(node->type == KA_CTX);
    assert(node->key == NULL);
    assert(node->value == NULL);
    assert(node->next == NULL);

    ka_free(node);
}

void test_number()
{
    KaNode *node = ka_number(42);

    assert(node->type == KA_NUMBER);
    assert(node->key == NULL);
    assert(*node->number == 42);
    assert(node->next == NULL);

    ka_free(node);
}

void test_string()
{
    KaNode *node = ka_string("Hello");

    assert(node->type == KA_STRING);
    assert(node->key == NULL);
    assert(!strcmp(node->string, "Hello"));
    assert(node->next == NULL);

    ka_free(node);
}

void test_symbol()
{
    KaNode *node = ka_symbol("sum");

    assert(node->type == KA_SYMBOL);
    assert(node->key == NULL);
    assert(!strcmp(node->symbol, "sum"));
    assert(node->next == NULL);

    ka_free(node);
}

void test_func()
{
    KaNode *node = ka_func(ka_def);

    assert(node->type == KA_FUNC);
    assert(node->key == NULL);
    assert(node->func == ka_def);
    assert(node->next == NULL);

    ka_free(node);
}

void test_copy()
{
    KaNode *list = ka_list(
        ka_number(1), ka_string("a"), ka_symbol("b"), NULL
    );
    KaNode *first = list->children;
    KaNode *list_copy = ka_copy(list);
    KaNode *first_copy = ka_copy(first);
    KaNode *second_copy = ka_copy(list->children->next);
    KaNode *third_copy = ka_copy(list->children->next->next);

    assert(first_copy->type == first->type && first->type == KA_NUMBER);
    assert(*first_copy->number == *first->number);
    assert(first->next != NULL && first_copy->next == NULL);
    assert(!second_copy->next && !third_copy->next);

    assert(list_copy->type == list->type && list->type == KA_LIST);
    assert(list_copy->children != first_copy);
    assert(*list_copy->children->number == *first_copy->number);
    assert(list_copy->children->next != second_copy);
    assert(!strcmp(list_copy->children->next->string, second_copy->string));
    assert(list_copy->children->next->next != third_copy);
    assert(list_copy->children->next->next->type == third_copy->type);
    assert(third_copy->type == KA_SYMBOL);
    assert(
        !strcmp(list_copy->children->next->next->string, third_copy->string)
    );
    assert(!list_copy->children->next->next->next);

    ka_free(list_copy);
    ka_free(third_copy);
    ka_free(second_copy);
    ka_free(first_copy);
    ka_free(list);
}

void test_children()
{
    KaNode *list = ka_list(ka_number(1), ka_number(2), NULL);
    KaNode *expr = ka_expr(ka_number(1), ka_number(2), NULL);
    KaNode *block = ka_block(ka_number(1), ka_number(2), NULL);

    assert(list->type == KA_LIST);
    assert(*list->children->number == 1);
    assert(*list->children->next->number == 2);
    assert(expr->type == KA_EXPR);
    assert(*expr->children->number == 1);
    assert(*expr->children->next->number == 2);
    assert(block->type == KA_BLOCK);
    assert(*block->children->number == 1);
    assert(*block->children->next->number == 2);

    ka_free(block);
    ka_free(expr);
    ka_free(list);
}

void test_list()
{
    KaNode *node = ka_list(ka_number(42), ka_string("Hello"), NULL);

    assert(node->type == KA_LIST);
    assert(node->key == NULL);
    assert(node->children->type == KA_NUMBER);
    assert(node->children->next->type == KA_STRING);
    assert(node->children->next->next == NULL);
    assert(node->next == NULL);

    ka_free(node);
}

void test_expr()
{
    KaNode *node = ka_expr(
        ka_symbol("num"), ka_symbol("="), ka_number(7), NULL
    );

    assert(node->type == KA_EXPR);
    assert(node->key == NULL);
    assert(node->children->type == KA_SYMBOL);
    assert(!strcmp(node->children->symbol, "num"));
    assert(node->children->next->type == KA_SYMBOL);
    assert(!strcmp(node->children->next->symbol, "="));
    assert(node->children->next->next->type == KA_NUMBER);
    assert(*node->children->next->next->number == 7);
    assert(node->children->next->next->next == NULL);
    assert(node->next == NULL);

    ka_free(node);
}

void test_block()
{
    KaNode *node = ka_block(
        ka_symbol("num"), ka_symbol("="), ka_number(7), NULL
    );

    assert(node->type == KA_BLOCK);
    assert(node->key == NULL);
    assert(node->children->type == KA_SYMBOL);
    assert(!strcmp(node->children->symbol, "num"));
    assert(node->children->next->type == KA_SYMBOL);
    assert(!strcmp(node->children->next->symbol, "="));
    assert(node->children->next->next->type == KA_NUMBER);
    assert(*node->children->next->next->number == 7);
    assert(node->children->next->next->next == NULL);
    assert(node->next == NULL);

    ka_free(node);
}

void test_ref()
{
    KaNode *ctx = ka_new(KA_CTX);
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("key"), ka_string("name"), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("i"), ka_number(1), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));

    assert(*(ka_ref(&ctx, ka_symbol("age")))->number == 42);
    assert(!strcmp(ka_ref(&ctx, ka_symbol("name"))->string, "John"));
    assert(!(ka_ref(&ctx, ka_symbol("inexistent"))));
    assert(ka_ref(&ctx, ka_symbol("0"))->type == KA_NUMBER);
    assert(ka_ref(&ctx, ka_symbol("1"))->type == KA_STRING);

    ka_free(ctx);
}

void test_del()
{
    KaNode *ctx = ka_new(KA_CTX);
    ka_free(
        ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL))
    );
    ka_free(
        ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL))
    );
    ka_free(
        ka_def(&ctx, ka_chain(ka_symbol("message"), ka_string("Foo"), NULL))
    );

    ka_free(ka_del(&ctx, ka_symbol("unknown")));
    assert(!strcmp(ctx->key, "message"));
    assert(!strcmp(ctx->next->key, "age"));
    assert(!strcmp(ctx->next->next->key, "name"));

    ka_free(ka_del(&ctx, ka_symbol("message")));
    assert(!strcmp(ctx->key, "age"));
    assert(!strcmp(ctx->next->key, "name"));

    ka_free(ka_del(&ctx, ka_symbol("name")));
    assert(!strcmp(ctx->key, "age"));
    assert(ctx->next->type == KA_CTX);

    ka_free(ka_del(&ctx, NULL));
    assert(ctx->next->type == KA_CTX);

    ka_free(ctx);
}

void test_key()
{
    KaNode *ctx = ka_new(KA_CTX), *result;

    ka_free(ka_key(NULL, NULL));

    result = ka_key(&ctx, ka_chain(ka_symbol("two"), ka_number(2), NULL));

    assert(ctx->type == KA_CTX);
    assert(result->type == KA_NUMBER);
    assert(*result->number == 2);
    assert(!strcmp(result->key, "two"));

    ka_free(result);
    ka_free(ctx);
}

void test_get()
{
    KaNode *ctx = ka_new(KA_CTX), *result;
    ka_free(
        ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL))
    );
    ka_free(
        ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL))
    );

    assert(*(result = ka_get(&ctx, ka_symbol("age")))->number == 42);
    ka_free(result);
    assert(!strcmp((result = ka_get(&ctx, ka_symbol("name")))->string, "John"));
    ka_free(result);
    assert((result = ka_get(&ctx, ka_symbol("inexistent")))->type == KA_NONE);
    ka_free(result);

    ka_free(ka_def(&ctx, ka_chain(ka_symbol("(ctx)"), ka_new(KA_CTX), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(78), NULL)));

    assert(*(result = ka_get(&ctx, ka_symbol("age")))->number == 78);
    ka_free(result);
    assert(*(result = ka_get(&ctx, ka_symbol("0")))->number == 78);
    ka_free(result);
    assert(*(result = ka_get(&ctx, NULL))->number == 78);
    ka_free(result);
    assert((result = ka_get(&ctx, ka_symbol("1")))->type == KA_NONE);
    ka_free(result);

    ka_free(ctx);
}

void test_def()
{
    KaNode *ctx = ka_new(KA_CTX), *result;

    ka_free(ka_def(NULL, NULL));

    result = ka_def(&ctx, ka_chain(ka_symbol("block"), ka_block(NULL), NULL));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("John"), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("name"), ka_string("Doe"), NULL)));

    assert(!strcmp(ctx->key, "name"));
    assert(!strcmp(ctx->string, "Doe"));
    assert(!strcmp(ctx->next->key, "age"));
    assert(*ctx->next->number == 42);
    assert(!strcmp(ctx->next->next->key, "name"));
    assert(!strcmp(ctx->next->next->string, "John"));
    assert(result->type == KA_NONE && ctx->next->next->next->type == KA_BLOCK);

    ka_free(result);
    ka_free(ctx);
}

void test_set()
{
    KaNode *ctx = ka_new(KA_CTX), *result;
    ka_free(ka_set(&ctx, NULL));

    ka_free(ka_set(&ctx, ka_chain(ka_symbol("num"), ka_number(1), NULL)));
    assert(*ctx->number == 1);
    ka_free(ka_set(&ctx, ka_chain(ka_symbol("num"), ka_new(KA_NONE), NULL)));
    assert(!ctx->number);

    ka_free(
        ka_set(&ctx, ka_chain(ka_symbol("block"), ka_string("NULL"), NULL))
    );
    result = ka_set(&ctx, ka_chain(ka_symbol("block"), ka_block(NULL), NULL));
    ka_free(
        ka_set(&ctx, ka_chain(
            ka_symbol("name"),
            ka_list(
                ka_string("John"), ka_string("Doe"), NULL
            ),
            NULL
        ))
    );
    ka_free(ka_set(&ctx, ka_chain(ka_symbol("age"), ka_number(42), NULL)));
    ka_free(ka_set(&ctx, ka_chain(ka_symbol("1"), ka_string("Foo"), NULL)));

    assert(!strcmp(ctx->key, "age"));
    assert(*ctx->number == 42);
    assert(ctx->next->type == KA_STRING);
    assert(!strcmp(ctx->next->key, "name"));
    assert(!strcmp(ctx->next->string, "Foo"));
    assert(result->type == KA_NONE && ctx->next->next->type == KA_BLOCK);

    ka_free(result);
    ka_free(ctx);
}

void test_bind()
{
    KaNode *ctx = ka_new(KA_CTX), *result;

    ka_free(ka_bind(NULL, NULL));

    KaNode *list = ka_list(
        ka_key(&ctx, ka_chain(ka_symbol("one"), ka_number(1), NULL)),
        ka_key(&ctx, ka_chain(ka_symbol("two"), ka_number(2), NULL)),
        NULL
    );

    result = ka_get(&ctx, ka_symbol("two"));
    assert(result->type == KA_NONE);
    ka_free(result);

    result = ka_bind(&ctx, ka_chain(ka_copy(list), ka_symbol("two"), NULL));
    assert(*result->number == 2);
    ka_free(result);

    ka_free(list);
    ka_free(ctx);
}

void test_return()
{
    KaNode *ctx = ka_init(), *block, *result;

    block = ka_block(
        ka_expr(ka_number(1), NULL),
        ka_expr(ka_number(7), NULL),
        NULL
    );
    result = ka_eval(&ctx, ka_chain(block, ka_new(KA_NONE), NULL));
    assert(*result->number == 7);
    ka_free(block);
    ka_free(result);

    block = ka_block(
        ka_expr(ka_symbol("return"), ka_number(1), NULL),
        ka_expr(ka_number(7), NULL),
        NULL
    );
    result = ka_eval(&ctx, ka_chain(block, ka_new(KA_NONE), NULL));
    assert(*result->number == 1);
    ka_free(block);
    ka_free(result);

    ka_free(ctx);
}

void test_logical()
{
    KaNode *result;

    ka_free(ka_and(NULL, NULL));

    result = ka_and(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(*result->number == 2);
    ka_free(result);

    result = ka_and(NULL, ka_chain(ka_number(1), ka_new(KA_NONE), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_and(NULL, ka_chain(ka_new(KA_NONE), ka_new(KA_NONE), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    ka_free(ka_or(NULL, NULL));

    result = ka_or(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(*result->number == 1);
    ka_free(result);

    result = ka_or(NULL, ka_chain(ka_new(KA_NONE), ka_number(2), NULL));
    assert(*result->number == 2);
    ka_free(result);

    result = ka_or(NULL, ka_chain(ka_new(KA_NONE), ka_new(KA_NONE), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_not(NULL, ka_number(1));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_not(NULL, NULL);
    assert(result->type == KA_TRUE);
    ka_free(result);
}

void test_comparison()
{
    KaNode *result;

    ka_free(ka_eq(NULL, NULL));

    result = ka_eq(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_eq(NULL, ka_chain(ka_new(KA_NONE), ka_number(2), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_eq(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_eq(NULL,
        ka_chain(ka_string("Hello"), ka_string("World"), NULL)
    );
    assert(result->type == KA_FALSE);
    ka_free(result);
    
    result = ka_eq(NULL, ka_chain(
        ka_string("Hello"), ka_string("Hello"), NULL)
    );
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_neq(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_neq(NULL, ka_chain(ka_new(KA_NONE), ka_number(2), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_neq(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_neq(NULL,
        ka_chain(ka_string("Hello"), ka_string("World"), NULL)
    );
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_neq(NULL,
        ka_chain(ka_string("Hello"), ka_string("Hello"), NULL)
    );
    assert(result->type == KA_FALSE);
    ka_free(result);

    ka_free(ka_gt(NULL, NULL));

    result = ka_gt(NULL, ka_chain(ka_string("2"), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_gt(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_gt(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_gt(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    ka_free(ka_lt(NULL, NULL));

    result = ka_lt(NULL, ka_chain(ka_string("2"), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_lt(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_lt(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_lt(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    ka_free(ka_gte(NULL, NULL));

    result = ka_gte(NULL, ka_chain(ka_string("2"), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_gte(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_gte(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_gte(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    ka_free(ka_lte(NULL, NULL));

    result = ka_lte(NULL, ka_chain(ka_string("2"), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_lte(NULL, ka_chain(ka_number(2), ka_number(1), NULL));
    assert(result->type == KA_FALSE);
    ka_free(result);

    result = ka_lte(NULL, ka_chain(ka_number(1), ka_number(2), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);

    result = ka_lte(NULL, ka_chain(ka_number(1), ka_number(1), NULL));
    assert(result->type == KA_TRUE);
    ka_free(result);
}

void test_if()
{
    KaNode *ctx = ka_new(KA_CTX);
    KaNode *block = ka_block(ka_number(42), NULL);
    KaNode *elseif_block = ka_block(ka_number(71), NULL);
    KaNode *else_block = ka_block(ka_number(27), NULL);
    KaNode *result;

    ka_free(ka_if(NULL, NULL));

    result = ka_if(&ctx, ka_chain(
        ka_lt(&ctx, ka_chain(
            ka_number(1), ka_number(2), NULL
        )),
        ka_copy(block),
        ka_copy(else_block),
        NULL
    ));
    assert(*result->number == 42);
    ka_free(result);

    result = ka_if(&ctx, ka_chain(
        ka_gt(&ctx, ka_chain(
            ka_number(1), ka_number(2), NULL
        )),
        ka_copy(block),
        ka_copy(else_block),
        NULL
    ));
    assert(*result->number == 27);
    ka_free(result);

    result = ka_if(&ctx, ka_chain(
        ka_gt(&ctx, ka_chain(
            ka_number(1), ka_number(2), NULL
        )),
        ka_copy(block),
        NULL
    ));
    assert(result->type == KA_NONE);
    ka_free(result);

    result = ka_if(&ctx, ka_chain(
        ka_gt(&ctx, ka_chain(
            ka_number(1), ka_number(2), NULL
        )),
        ka_copy(block),
        ka_gt(&ctx, ka_chain(
            ka_number(2), ka_number(1), NULL
        )),
        ka_copy(elseif_block),
        ka_copy(else_block),
        NULL
    ));
    assert(*result->number == 71);
    ka_free(result);

    result = ka_if(&ctx, ka_chain(
        ka_gt(&ctx, ka_chain(
            ka_number(1), ka_number(2), NULL
        )),
        ka_copy(block),
        ka_gt(&ctx, ka_chain(
            ka_number(2), ka_number(3), NULL
        )),
        ka_copy(elseif_block),
        ka_copy(else_block),
        NULL
    ));
    assert(*result->number == 27);
    ka_free(result);

    ka_free(elseif_block);
    ka_free(else_block);
    ka_free(block);
    ka_free(ctx);
}

void test_while()
{
    KaNode *ctx = ka_init();

    ka_free(ka_while(NULL, NULL));

    ka_free(ka_def(&ctx, ka_chain(ka_symbol("i"), ka_number(0), NULL)));

    KaNode *cond = ka_expr(
        ka_symbol("<"), ka_symbol("i"), ka_number(10), NULL
    );

    KaNode *block = ka_block(
        ka_symbol("="), ka_symbol("i"), ka_expr(
            ka_symbol("+"), ka_symbol("i"), ka_number(1), NULL
        ), NULL
    );

    ka_free(ka_while(&ctx, ka_chain(cond, block, NULL)));

    KaNode *result = ka_get(&ctx, ka_symbol("i"));
    assert(*result->number == 10);
    ka_free(result);

    ka_free(ctx);
}

void test_for()
{
    KaNode *ctx = ka_new(KA_CTX);

    ka_free(ka_for(NULL, NULL));

    KaNode *list = ka_list(ka_number(1), ka_number(2), NULL);
    KaNode *block = ka_block(ka_symbol("0"), NULL);

    KaNode *result = ka_for(&ctx, ka_chain(list, block, NULL));
    assert(*result->children->number == 1);
    assert(*result->children->next->number == 2);
    ka_free(result);

    ka_free(ctx);
}

void test_range()
{
    KaNode *result;

    ka_free(ka_range(NULL, NULL));

    result = ka_range(NULL, ka_chain(ka_number(1), ka_number(3), NULL));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 1);
    assert(*result->children->next->number == 2);
    assert(*result->children->next->next->number == 3);
    assert(!result->children->next->next->next);
    ka_free(result);

    result = ka_range(NULL, ka_chain(ka_number(2), ka_number(0), NULL));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 2);
    assert(*result->children->next->number == 1);
    assert(*result->children->next->next->number == 0);
    assert(!result->children->next->next->next);
    ka_free(result);
}

void test_merge()
{
    KaNode *result;

    ka_free(ka_merge(NULL, NULL));

    result = ka_merge(NULL, ka_chain(
        ka_list(ka_number(1), NULL),
        ka_list(ka_number(2), NULL),
        NULL
    ));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 1);
    assert(*result->children->next->number == 2);
    ka_free(result);

    result = ka_merge(NULL, ka_chain(
        ka_list(ka_number(3), NULL),
        ka_number(4),
        NULL
    ));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 3);
    assert(*result->children->next->number == 4);
    ka_free(result);

    result = ka_merge(NULL, ka_chain(
        ka_number(4),
        ka_list(ka_number(3), NULL),
        NULL
    ));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 4);
    assert(*result->children->next->number == 3);
    ka_free(result);
}

void test_cat()
{
    KaNode *result;

    ka_free(ka_cat(NULL, NULL));

    result = ka_cat(NULL, ka_chain(
        ka_number(2), ka_string("Message"), NULL
    ));
    assert(!strcmp(result->string, "2Message"));
    ka_free(result);

    result = ka_cat(NULL, ka_chain(
        ka_string("Hello"), ka_string("World"), NULL
    ));
    assert(!strcmp(result->string, "HelloWorld"));
    ka_free(result);

    result = ka_cat(NULL, ka_chain(
        ka_string("Ten"), ka_number(10), NULL
    ));
    assert(!strcmp(result->string, "Ten10"));
    ka_free(result);

    result = ka_cat(NULL, ka_chain(
        ka_string("Float"), ka_number(10.123), NULL
    ));
    assert(!strcmp(result->string, "Float10.12"));
    ka_free(result);
}

void test_split()
{
    KaNode *result;

    ka_free(ka_split(NULL, NULL));

    result = ka_split(NULL, ka_chain(
        ka_string("J. Doe"), ka_string(" "), NULL
    ));
    assert(result->type == KA_LIST);
    assert(!strcmp(result->children->string, "J."));
    assert(!strcmp(result->children->next->string, "Doe"));
    assert(!result->children->next->next);
    ka_free(result);

    result = ka_split(NULL, ka_chain(
        ka_string("Doe"), ka_string(""), NULL
    ));
    assert(result->type == KA_LIST);
    assert(!strcmp(result->children->string, "D"));
    assert(!strcmp(result->children->next->string, "o"));
    assert(!strcmp(result->children->next->next->string, "e"));
    assert(!result->children->next->next->next);
    ka_free(result);
}

void test_join()
{
    KaNode *result;

    ka_free(ka_join(NULL, NULL));

    result = ka_join(NULL, ka_chain(
        ka_list(ka_string("John"), ka_string("Doe"), NULL),
        ka_string("-"),
        NULL
    ));
    assert(!strcmp(result->string, "John-Doe"));
    ka_free(result);

    result = ka_join(NULL, ka_chain(
        ka_list(ka_string("John"), ka_number(12), ka_string("Doe"), NULL),
        ka_string("-"),
        NULL
    ));
    assert(!strcmp(result->string, "John-Doe"));
    ka_free(result);

    result = ka_join(NULL, ka_chain(
        ka_string("John"),
        ka_string("Doe"),
        NULL
    ));
    assert(result->type == KA_NONE);
    ka_free(result);
}

void test_length()
{
    KaNode *ctx = ka_new(KA_CTX), *result;

    result = ka_length(&ctx, ka_string(""));
    assert(*result->number == 0);
    ka_free(result);

    result = ka_length(&ctx, ka_string("John Doe"));
    assert(*result->number == 8);
    ka_free(result);

    result = ka_length(&ctx, ka_list(NULL));
    assert(*result->number == 0);
    ka_free(result);

    result = ka_length(&ctx, ka_list(ka_number(1), ka_string("John"), NULL));
    assert(*result->number == 2);
    ka_free(result);

    result = ka_length(&ctx, ka_block(ka_number(1), ka_string("John"), NULL));
    assert(*result->number == 0);
    ka_free(result);

    result = ka_length(&ctx, ka_number(1));
    assert(*result->number == 0);
    ka_free(result);

    ka_free(ctx);
}

void test_upperlower()
{
    KaNode *ctx = ka_new(KA_CTX), *result;

    ka_free(ka_upper(&ctx, NULL));
    result = ka_upper(&ctx, ka_string("John Doe"));
    assert(!strcmp(result->string, "JOHN DOE"));
    ka_free(result);

    ka_free(ka_lower(&ctx, NULL));
    result = ka_lower(&ctx, ka_string("John Doe"));
    assert(!strcmp(result->string, "john doe"));
    ka_free(result);

    ka_free(ctx);
}

void test_arithmetic()
{
    KaNode *ctx = ka_new(KA_CTX), *result;

    ka_free(ka_add(&ctx, NULL));
    result = ka_add(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
    assert(*result->number == 5);
    ka_free(result);

    ka_free(ka_sub(&ctx, NULL));
    result = ka_sub(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
    assert(*result->number == 1);
    ka_free(result);

    ka_free(ka_mul(&ctx, NULL));
    result = ka_mul(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
    assert(*result->number == 6);
    ka_free(result);

    ka_free(ka_div(&ctx, NULL));
    result = ka_div(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
    assert(*result->number == 1.5);
    ka_free(result);

    ka_free(ka_mod(&ctx, NULL));
    result = ka_mod(NULL, ka_chain(ka_number(3), ka_number(2), NULL));
    assert(*result->number == 1);
    ka_free(result);

    result = ka_add(NULL, ka_chain(
        ka_number(2), ka_string("Message"), NULL
    ));
    assert(!strcmp(result->string, "2Message"));
    ka_free(result);

    result = ka_add(NULL, ka_chain(
        ka_string("Hello"), ka_string("World"), NULL
    ));
    assert(!strcmp(result->string, "HelloWorld"));
    ka_free(result);

    result = ka_add(NULL, ka_chain(
        ka_string("Ten"), ka_number(10), NULL
    ));
    assert(!strcmp(result->string, "Ten10"));
    ka_free(result);

    result = ka_add(NULL, ka_chain(
        ka_string("Float"), ka_number(10.123), NULL
    ));
    assert(!strcmp(result->string, "Float10.12"));
    ka_free(result);

    result = ka_add(NULL, ka_chain(
        ka_list(ka_number(1), NULL),
        ka_list(ka_number(2), NULL),
        NULL
    ));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 1);
    assert(*result->children->next->number == 2);
    ka_free(result);

    result = ka_add(NULL, ka_chain(
        ka_list(ka_number(3), NULL),
        ka_number(4),
        NULL
    ));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 3);
    assert(*result->children->next->number == 4);
    ka_free(result);

    result = ka_add(NULL, ka_chain(
        ka_number(4),
        ka_list(ka_number(3), NULL),
        NULL
    ));
    assert(result->type == KA_LIST);
    assert(*result->children->number == 4);
    assert(*result->children->next->number == 3);
    ka_free(result);

    result = ka_mul(NULL, ka_chain(ka_string("John"), ka_number(2), NULL));
    assert(result->type == KA_NONE);
    ka_free(result);

    KaNode *list = ka_list(ka_number(1), ka_number(2), NULL);
    KaNode *block = ka_block(ka_symbol("0"), NULL);

    result = ka_mul(&ctx, ka_chain(list, block, NULL));
    assert(*result->children->number == 1);
    assert(*result->children->next->number == 2);
    ka_free(result);

    result = ka_mul(NULL, ka_chain(
        ka_list(ka_string("John"), ka_string("Doe"), NULL),
        ka_string("-"),
        NULL
    ));
    assert(result->type == KA_STRING);
    assert(!strcmp(result->string, "John-Doe"));
    ka_free(result);

    result = ka_div(NULL, ka_chain(ka_string("John Doe"), ka_number(2), NULL));
    assert(result->type == KA_NONE);
    ka_free(result);

    result = ka_div(NULL, ka_chain(
        ka_string("John Doe"), ka_string(" "), NULL
    ));
    assert(result->type == KA_LIST);
    assert(!strcmp(result->children->string, "John"));
    assert(!strcmp(result->children->next->string, "Doe"));
    assert(!result->children->next->next);
    ka_free(result);

    result = ka_div(NULL, ka_chain(ka_string("Doe"), ka_string(""), NULL));
    assert(result->type == KA_LIST);
    assert(!strcmp(result->children->string, "D"));
    assert(!strcmp(result->children->next->string, "o"));
    assert(!strcmp(result->children->next->next->string, "e"));
    assert(!result->children->next->next->next);
    ka_free(result);

    result = ka_def(&ctx, ka_chain(ka_symbol("i"), ka_number(2), NULL));

    ka_free(ka_addset(&ctx, NULL));
    result = ka_addset(&ctx, ka_chain(result, ka_number(3), NULL));
    assert(*result->number == 5);

    ka_free(ka_subset(&ctx, NULL));
    result = ka_subset(&ctx, ka_chain(result, ka_number(1), NULL));
    assert(*result->number == 4);

    ka_free(ka_mulset(&ctx, NULL));
    result = ka_mulset(&ctx, ka_chain(result, ka_number(2), NULL));
    assert(*result->number == 8);

    ka_free(ka_divset(&ctx, NULL));
    result = ka_divset(&ctx, ka_chain(result, ka_number(2), NULL));
    assert(*result->number == 4);

    ka_free(ka_modset(&ctx, NULL));
    result = ka_modset(&ctx, ka_chain(result, ka_number(3), NULL));
    assert(*result->number == 1);

    ka_free(result);

    ka_free(ctx);
}

void test_eval()
{
    KaNode *ctx = ka_new(KA_CTX), *expr, *result;
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("def"), ka_func(ka_def), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("add"), ka_func(ka_add), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("lt"), ka_func(ka_lt), NULL)));
    ka_free(ka_def(&ctx, ka_chain(ka_symbol("i"), ka_number(5), NULL)));

    // Define a variable into the context
    expr = ka_expr(
        ka_symbol("def"), ka_symbol("name"), ka_string("John"), NULL
    );
    result = ka_eval(&ctx, expr);
    KaNode *var = ka_get(&ctx, ka_symbol("name"));
    assert(!strcmp(var->string, result->string));
    assert(!strcmp(result->string, "John"));
    ka_free(var);
    ka_free(result);
    ka_free(expr);

    // Define and recover a variable inside a block context
    expr = ka_expr(
        ka_block(
            ka_expr(
                ka_symbol("def"), ka_symbol("age"), ka_number(42), NULL
            ),
            ka_expr(
                ka_symbol("def"), ka_symbol("name"), ka_string("Doe"), NULL
            ),
            ka_expr(
                ka_symbol("name"), NULL
            ),
            NULL
        ),
        ka_new(KA_NONE),
        NULL
    );
    result = ka_eval(&ctx, expr);
    assert(!strcmp(result->string, "Doe"));
    ka_free(result);
    ka_free(expr);

    // Recover a global variable using a new block
    expr = ka_expr(ka_symbol("name"), NULL);
    result = ka_eval(&ctx, expr);
    assert(!strcmp(result->string, "John"));
    ka_free(result);
    ka_free(expr);

    // Use other kind of functions
    expr = ka_chain(ka_symbol("lt"), ka_number(5), ka_number(10), NULL);
    result = ka_eval(&ctx, expr);
    assert(result->type == KA_TRUE);
    ka_free(result);
    ka_free(expr);

    expr = ka_chain(ka_symbol("lt"), ka_symbol("i"), ka_number(10), NULL);
    result = ka_eval(&ctx, expr);
    assert(result->type == KA_TRUE);
    ka_free(result);
    ka_free(expr);

    expr = ka_chain(ka_symbol("add"), ka_number(5), ka_number(10), NULL);
    result = ka_eval(&ctx, expr);
    assert(*result->number == 15);
    ka_free(result);
    ka_free(expr);

    ka_free(ctx);
}

void test_parser()
{
    KaNode *result;
    int pos;

    pos = 0;
    result = ka_parser("# This is a comment", &pos);
    assert(!result);

    pos = 0;
    result = ka_parser("42.21 # This is a comment", &pos);
    assert(fabsl(*result->children->number - 42.21) < 1e-10);
    assert(!result->children->next);
    ka_free(result);

    pos = 0;
    result = ka_parser("42.21 age // This is a comment", &pos);
    assert(fabsl(*result->children->number - 42.21) < 1e-10);
    assert(!strcmp(result->children->next->symbol, "age"));
    assert(!result->children->next->next);
    ka_free(result);

    pos = 0;
    result = ka_parser("age\n// This is a comment\n42", &pos);
    assert(!strcmp(result->children->symbol, "age"));
    assert(*result->next->children->number == 42);
    ka_free(result);

    pos = 0;
    result = ka_parser("age\n/* Multiline\ncomment */\n42", &pos);
    assert(!strcmp(result->children->symbol, "age"));
    assert(*result->next->children->number == 42);
    ka_free(result);

    pos = 0;
    result = ka_parser("'It\\'s John Doe. Backslash: \\\\ OK'", &pos);
    assert(
        !strcmp(result->children->string, "It\'s John Doe. Backslash: \\ OK")
    );
    ka_free(result);

    pos = 0;
    result = ka_parser("age; 42 'John Doe' 21;name", &pos);
    assert(!strcmp(result->children->symbol, "age"));
    assert(*result->next->children->number == 42);
    assert(!strcmp(result->next->children->next->string, "John Doe"));
    assert(*result->next->children->next->next->number == 21);
    assert(!strcmp(result->next->next->children->symbol, "name"));
    ka_free(result);

    pos = 0;
    result = ka_parser("42 'John Doe';name (22) [1 2] {71; 72}", &pos);
    KaNode *number = result->children;
    KaNode *string = result->children->next;
    KaNode *symbol = result->next->children;
    KaNode *expr = result->next->children->next->children;
    KaNode *list = result->next->children->next->next;
    KaNode *block = result->next->children->next->next->next;
    assert(number->type == KA_NUMBER && *number->number == 42);
    assert(string->type == KA_STRING && !strcmp(string->string, "John Doe"));
    assert(symbol->type == KA_SYMBOL && !strcmp(symbol->symbol, "name"));
    assert(expr->type == KA_EXPR && *expr->children->number == 22);
    assert(list->type == KA_LIST);
    assert(*list->children->children->number == 1);
    assert(*list->children->children->next->number == 2);
    assert(block->type == KA_BLOCK && *block->children->children->number == 71);
    assert(*block->children->next->children->number == 72);
    ka_free(result);

    pos = 0;
    result = ka_parser("i = 1 * 2; 5 + 6 - 7", &pos);
    KaNode *expr_set = result->children->children;
    KaNode *expr_mul = result->children->children->next->next->children;
    KaNode *expr_sub = result->next->children->children;
    KaNode *expr_sum = result->next->children->children->next->children;
    assert(expr_set->type == KA_SYMBOL && !strcmp(expr_set->symbol, "="));
    assert(!strcmp(expr_set->next->symbol, "i"));
    assert(expr_mul->type == KA_SYMBOL && !strcmp(expr_mul->symbol, "*"));
    assert(*expr_mul->next->number == 1 && *expr_mul->next->next->number == 2);
    assert(expr_sub->type == KA_SYMBOL && !strcmp(expr_sub->symbol, "-"));
    assert(*expr_sub->next->next->number == 7);
    assert(expr_sum->type == KA_SYMBOL && !strcmp(expr_sum->symbol, "+"));
    assert(*expr_sum->next->number == 5 && *expr_sum->next->next->number == 6);
    ka_free(result);

    pos = 0;
    result = ka_parser("!1 && 2", &pos);
    KaNode *expr_and = result->children->children;
    KaNode *expr_not = result->children->children->next->children;
    assert(expr_and->type == KA_SYMBOL && !strcmp(expr_and->symbol, "&&"));
    assert(expr_not->type == KA_SYMBOL && !strcmp(expr_not->symbol, "!"));
    assert(*expr_not->next->number == 1);
    assert(*expr_and->next->next->number == 2);
    ka_free(result);

    pos = 0;
    result = ka_parser("+; 2", &pos);
    assert(*result->children->number == 2);
    ka_free(result);

    pos = 0;
    result = ka_parser("?; 2", &pos);
    assert(*result->next->children->number == 2);
    ka_free(result);
}

void test_input()
{
    KaNode *ctx = ka_new(KA_CTX);

    KaNode *result = ka_input(&ctx, ka_new(KA_NONE));
    assert(*result->number == 99);
    ka_free(result);

    ka_free(ctx);
}

void test_read()
{
    KaNode *ctx = ka_new(KA_CTX);

    KaNode *result = ka_read(&ctx, ka_string("README.md"));
    assert(!strncmp(result->string, "Kamby Language", 14));
    ka_free(result);

    ka_free(ctx);
}

void test_write()
{
    KaNode *ctx = ka_new(KA_CTX);

    ka_free(
        ka_write(&ctx, ka_chain(
            ka_string("tests.out"), ka_string("content"), NULL
        ))
    );
    KaNode *result = ka_read(&ctx, ka_string("tests.out"));
    assert(!strncmp(result->string, "content", 7));
    ka_free(result);

    ka_free(ctx);
}

void test_load()
{
    KaNode *ctx = ka_init(), *result;

    ka_free(
        ka_write(&ctx, ka_chain(
            ka_string("tests.out"), ka_string("a = 12"), NULL
        ))
    );
    result = ka_load(&ctx, ka_string("tests.out"));
    assert(*result->number == 12);
    assert(*ctx->number == 12);
    ka_free(result);

    result = ka_load(&ctx, ka_string("invalidlib.so"));
    assert(result->type == KA_NONE);
    ka_free(result);

    result = ka_load(&ctx, ka_string("testslib.so"));
    assert(result->type == KA_NONE);
    ka_free(result);

    ka_free(ctx);
}

void test_init()
{
    KaNode *ctx = ka_init(), *last, *prev;

    for (last = ctx; last->next; last = last->next) {
        prev = last;
    }

    assert(ctx->type == KA_CTX);
    assert(ctx->next->type == KA_FUNC);
    assert(strlen(ctx->next->key) > 0);
    assert(last->type == KA_CTX);
    assert(prev->type == KA_TRUE);
    assert(!strcmp(prev->key, "true"));

    ka_free(ctx);
}

KaNode *eval_code(KaNode **ctx, const char *code)
{
    int pos = 0;
    KaNode *expr = ka_parser((char *) code, &pos);
    KaNode *result = ka_eval(ctx, expr);

    ka_free(expr);

    return result;
}

void test_code_print()
{
    KaNode *ctx = ka_init();

    ka_free(eval_code(&ctx, "print [12, 3.45, '... testing']"));

    ka_free(ctx);
}

void test_code_variables()
{
    KaNode *ctx = ka_init(), *result;

    result = eval_code(&ctx, "i: 1");
    assert(!strcmp(result->key, "i") && *result->number == 1);
    assert(strcmp(ctx->key, "i") && ctx->type != KA_NUMBER);
    ka_free(result);

    result = eval_code(&ctx, "i := 1");
    assert(!strcmp(result->key, "i") && *result->number == 1);
    assert(!strcmp(ctx->key, "i") && ctx->type == KA_NUMBER);
    ka_free(result);

    result = eval_code(&ctx, "i := 2");
    assert(!strcmp(result->key, "i") && *result->number == 2);
    assert(!strcmp(ctx->key, "i") && *ctx->number == 2);
    assert(!strcmp(ctx->next->key, "i") && *ctx->next->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "i = 3");
    assert(!strcmp(result->key, "i") && *result->number == 3);
    assert(!strcmp(ctx->key, "i") && *ctx->number == 3);
    assert(!strcmp(ctx->next->key, "i") && *ctx->next->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "$0");
    assert(!strcmp(result->key, "i") && *result->number == 3);
    ka_free(result);

    result = eval_code(&ctx, "$1");
    assert(!strcmp(result->key, "i") && *result->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "i");
    assert(!strcmp(result->key, "i") && *result->number == 3);
    ka_free(result);

    ka_free(eval_code(&ctx, "del i"));

    result = eval_code(&ctx, "i");
    assert(!strcmp(result->key, "i") && *result->number == 1);
    assert(!strcmp(ctx->key, "i") && *ctx->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "$0");
    assert(!strcmp(result->key, "i") && *result->number == 1);
    ka_free(result);

    ka_free(eval_code(&ctx, "def test { 0=arg:$0; arg }"));
    result = eval_code(&ctx, "test 99");
    assert(!strcmp(result->key, "arg") && *result->number == 99);
    ka_free(result);

    ka_free(ctx);
}

void test_code_lists()
{
    KaNode *ctx = ka_init(), *result;

    ka_free(eval_code(&ctx, "\
        i := 3;\
        items := [1, 2, third: 3, 4, double: { $0 * items.$1 }]\
    "));

    result = eval_code(&ctx, "items.$0");
    assert(*result->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "items.$i");
    assert(*result->number == 4);
    ka_free(result);

    result = eval_code(&ctx, "items.third");
    assert(*result->number == 3);
    ka_free(result);

    result = eval_code(&ctx, "items.double(7)");
    assert(*result->number == 14);
    ka_free(result);

    ka_free(eval_code(&ctx, "items.{ 2 = 33 }"));
    result = eval_code(&ctx, "items.third");
    assert(*result->number == 33);
    ka_free(result);

    ka_free(eval_code(&ctx, "\
        items.{\
            del double\n\
            0 = 11\n\
        }\n\
        items = items * { $0 * 2 }\
    "));
    result = eval_code(&ctx, "items");
    assert(*result->children->number == 22);
    assert(*result->children->next->number == 4);
    assert(*result->children->next->next->number == 66);
    assert(!strcmp(result->children->next->next->key, "third"));
    assert(*result->children->next->next->next->number == 8);
    assert(!result->children->next->next->next->next);
    ka_free(result);

    ka_free(ctx);
}

void test_code_blocks()
{
    KaNode *ctx = ka_init(), *result;

    ka_free(eval_code(&ctx, "def test { $1 / first }"));
    result = eval_code(&ctx, "test(first: 2, 8)");
    assert(*result->number == 4);
    ka_free(result);

    ka_free(ctx);
}

void test_code_if()
{
    KaNode *ctx = ka_init(), *result;

    ka_free(eval_code(&ctx, "i := 10"));

    result = eval_code(&ctx, "if i { 1 } { 0 }");
    assert(*result->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "if a { 1 } { 0 }");
    assert(*result->number == 0);
    ka_free(result);

    result = eval_code(&ctx, "if !a { 1 } { 0 }");
    assert(*result->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "1 == 1 ? 1 'two' == 'two' { 2 } { 1 + 2 }");
    assert(*result->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "(1 != 1) ? 1 ('two' == 'two') { 2 } { 1 + 2 }");
    assert(*result->number == 2);
    ka_free(result);

    result = eval_code(&ctx, "(1 != 1) ? 1 ('two' != 'two') { 2 } { 1 + 2 }");
    assert(*result->number == 3);
    ka_free(result);

    result = eval_code(&ctx, "if false 1 false { 2 } else { 2 + 2 }");
    assert(*result->number == 4);
    ka_free(result);

    ka_free(ctx);
}

void test_code_return()
{
    KaNode *ctx = ka_init(), *result;

    ka_free(eval_code(&ctx, "i := 0"));
    ka_free(eval_code(&ctx, "def test { 99; i += 1 }"));
    ka_free(eval_code(&ctx, "def test_return { return 99; i += 1 }"));

    result = eval_code(&ctx, "test()");
    assert(*result->number == 1);
    ka_free(result);

    result = eval_code(&ctx, "test_return()");
    assert(*result->number == 99);
    ka_free(result);

    result = eval_code(&ctx, "i");
    assert(*result->number == 1);
    ka_free(result);

    ka_free(ctx);
}

void test_code_while()
{
    KaNode *ctx = ka_init(), *result;

    ka_free(eval_code(&ctx, "i := 0; while (i < 10) { i += 1 }"));
    assert(*ctx->number == 10);

    ka_free(ctx);
}

int main()
{
    printf("\nRunning tests...\n");
    test_new();
    test_chain();
    test_ctx();
    test_number();
    test_string();
    test_symbol();
    test_func();
    test_copy();
    test_children();
    test_list();
    test_expr();
    test_block();
    test_ref();
    test_del();
    test_key();
    test_get();
    test_def();
    test_set();
    test_bind();
    test_return();
    test_logical();
    test_comparison();
    test_if();
    test_while();
    test_for();
    test_range();
    test_merge();
    test_cat();
    test_split();
    test_join();
    test_length();
    test_upperlower();
    test_arithmetic();
    test_eval();
    test_parser();
    test_input();
    test_read();
    test_write();
    test_load();
    test_init();
    test_code_print();
    test_code_variables();
    test_code_lists();
    test_code_blocks();
    test_code_if();
    test_code_return();
    test_code_while();

    printf("All tests passed!\n");
    return 0;
}
