#ifndef RBT_TIMER_H
#define RBT_TIMER_H

#include <memory>

#include "ngx_rbtree.h"

struct timer_node_s;

typedef void (*timer_handle_ptr)(struct timer_node_s*);

typedef struct timer_node_s
{
    ngx_rbtree_node_t node;
    timer_handle_ptr cb;
} timer_node_t;

ngx_rbtree_t timer;

static ngx_rbtree_node_t sentinel;

void init_timer()
{
    ngx_rbtree_init(&timer, &sentinel, ngx_rbtree_insert_timer_value);
}

unsigned int current_time()
{
    unsigned int now;
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    now = t.tv_sec * 1000;
    now += t.tv_nsec / 1000000;
    return now;
}
timer_node_t *add_timer(int expire, timer_handle_ptr func)
{
    timer_node_t* t(new timer_node_t);

    t->node.key = expire + current_time();
    t->cb = func;
    ngx_rbtree_insert(&timer, &t->node);
    return t;
}

bool del_timer(timer_node_t *node)
{
    ngx_rbtree_delete(&timer, &node->node);
}


int find_nearest_timer(){
    if(timer.root == timer.sentinel) return -1;   

    ngx_rbtree_node_t* node;
    node = ngx_rbtree_min(timer.root,timer.sentinel);

    int diff = (int)node->key - (int)current_time();
    return diff > 0? diff:0;
}

void handle_timer()
{
    ngx_rbtree_node_t *root, *node;
    for (;;)
    {
        root = timer.root;
        if (root == timer.sentinel)
            break;

        node = ngx_rbtree_min(root, timer.sentinel);
        
        if (node->key > current_time()) break;
        timer_node_t *tn = (timer_node_t *)(char *)node;
        printf("do : %d  ",current_time());
        tn->cb(tn);
        
        ngx_rbtree_delete(&timer, &tn->node);
        free(tn);
    }
}

#endif