#ifndef __EVENT_H_
#define __EVENT_H_

#define EVENTS_SIZE 1024

struct myevent{
	int fd;	
	int events;
	void *arg;
	void (*callback)(int fd,void *arg);
	int status;
	char file[BUFSIZ];
	int file_len;
	char buf[BUFSIZ];	
	int buf_len;
	int last_active_time;
};


void event_add(int epfd,int events,struct myevent *evtp);

// 添加文件描述符到监听红黑树
void event_init(struct myevent *evtp, int fd, void (*callback)(int , void *),void *arg);

// 从监听红黑树上删除文件描述符
void event_del(int epfd,struct myevent *evtp);


#endif