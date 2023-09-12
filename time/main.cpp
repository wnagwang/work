#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "rbt_timer.h"

#define MAX_MSG 512

void print(struct timer_node_s* node){
    printf("定时器处理 %d\n",node->node.key);
}

int main(){
    init_timer();
    int epfd = epoll_create(1);
    struct epoll_event evs[MAX_MSG] ={0};

    add_timer(1000,print);
    add_timer(3000,print);
    add_timer(4000,print);
    add_timer(11000,print);
    add_timer(15000,print);
    for(;;){
        int nearest = find_nearest_timer();
//        int n = epoll_wait(epfd,evs,MAX_MSG,nearest);
//        int i = 0;
//        for(; i < n ; ++ i){

//        }
        sleep(3);
        handle_timer();
        //printf("======\n");
    }



    return 0;
}
