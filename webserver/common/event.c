#include <stdio.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include "wrap.h"
#include "utils.h"
#include "event.h"


void event_del(int epfd,struct myevent *evtp){
	struct epoll_event ev;
	if (evtp->status != 1)
	{
		return;
	}

	ev.data.ptr	= evtp;
	evtp->status = 0;
	epoll_ctl(epfd,EPOLL_CTL_DEL,evtp->fd,&ev);
}


void event_add(int epfd,int events,struct myevent *evtp){
	struct epoll_event ev;
	int op,ret;	
	ev.data.ptr = evtp;
	ev.events = evtp->events = events; 
	if (evtp->status == 1) {                                          
        op = EPOLL_CTL_MOD;                                         
    } else{                                
        op = EPOLL_CTL_ADD;                 
        evtp->status = 1;
    }

	ret = epoll_ctl(epfd,op,evtp->fd,&ev);
	if (ret == -1)
	{
		printf("epoll_ctl error:%s\n",strerror(errno));
		exit(1);
	}

	return;
}




void event_init(struct myevent *evtp, int fd, void (*callback)(int, void *),void *arg){
	evtp->fd = fd;
	evtp->events = 0;
	evtp->arg = arg;
	evtp->callback = callback;
	evtp->status = 0;
	evtp->last_active_time = time(NULL);

	return;
}


// 超时检测
void check_timeout(int epfd,struct myevent *events,int timeout){

	int checkpos = 0,i = 0;	
	long now = time(NULL);                          				//当前时间
    for (i = 0; i < 100; i++, checkpos++) {         				//一次循环检测100个,使用checkpos控制检测对象        
        if (checkpos == EVENTS_SIZE)
            checkpos = 0;
        if (events[checkpos].status != 1)         					//不在红黑树 g_efd 上
            continue;

        long duration = now - events[checkpos].last_active_time;    //客户端不活跃的事件

        if (duration >= timeout) {
            Close(events[checkpos].fd);                           	//关闭与该客户端链接
            printf("[fd=%d] timeout\n", events[checkpos].fd);
            event_del(epfd, &events[checkpos]);                   	//将该客户端 从红黑树 g_efd移除
        }
    }    	
}


