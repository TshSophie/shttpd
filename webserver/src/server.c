/*
	EPOLL 反应堆版本HTTP SERVER
*/
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
#include <getopt.h>
#include "wrap.h"
#include "event.h"
#include "utils.h"

#define SERV_PORT 8888
#define SERV_IP "127.0.0.1"
#define VERSION "shttpd version: shttpd/1.0.0"


// 初始化socket和epoll
void initsocket(int epfd,int port,char *addr,int backlog);

// 找不到文件响应
void notfound_response(int cfd);

// 出错响应
void error_response(int cfd);

// 读回调函数 
void readcb(int fd,void *arg);

// 写回调
void writecb(int fd,void *arg);

// 连接客户端回调
void acceptcb(int lfd,void *arg);

void check_timeout(int epfd,struct myevent *events,int timeout);


int g_epfd;										// epoll 红黑树根
struct myevent g_events[EVENTS_SIZE + 1];		// 保存要监听的文件描述符的事件等信息


void initsocket(int epfd,int port,char *addr,int backlog){

	int lfd;
	struct sockaddr_in serv_addr;
	struct myevent *events = g_events;

	lfd = Socket(AF_INET,SOCK_STREAM,0);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET,addr,&serv_addr.sin_addr.s_addr);

	Bind(lfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

	Listen(lfd,backlog);

	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		
	// 将lfd挂到监听红黑树上，并且设置其事件为acceptcb回调
	event_init(&events[EVENTS_SIZE],lfd,acceptcb,&events[EVENTS_SIZE]);

	event_add(epfd,EPOLLIN,&events[EVENTS_SIZE]);

	return;
}


/*
	cfd: 客户端文件描述符
	no: 错误号
	disc: 错误描述
	type: 文件类型
	len: 文件长度
*/
void send_respond(int cfd,int no,const char *disc,const char *type,int len){
	char buf[1024] = {0};
	sprintf(buf,"HTTP/1.1 %d %s\r\n",no,disc);
	sprintf(buf+strlen(buf),"Content-Type: %s\r\n",type);
	sprintf(buf+strlen(buf),"Content-Length:%d\r\n",len);
	
	Write(cfd,buf,strlen(buf));
	Write(cfd,"\r\n",2);
}


// 发送文件
void send_file(int cfd,const char *file){
	char buf[4096]  = {0};
	int n,ret,have;
	// 打开服务器根目录文件
	int fd = open(file,O_RDONLY);
	if (fd == -1)
	{
		notfound_response(cfd);
		return;
	}
	have = 0;
	while((n = Read(fd,buf,sizeof(buf))) > 0 )
	{
		ret = Write(cfd,buf,n);
		have++;		
		if (ret < 0)
		{			
			error_response(cfd);
			return;
		}		
	}
	Close(fd);
}


// 发送目录数据
void send_dir(int cfd,const char *dirname){
	int i;
	char buf[4096] = {0};
	sprintf(buf,"<html><head><title>Index of  %s</title><style type='text/css'>td{padding-left:30px;}</style><head>",dirname);
	sprintf(buf + strlen(buf),"<body><h1>Index of  %s</h1><table>",dirname);
	sprintf(buf + strlen(buf),"<tr><th>Name</th><th>Last modified</th><th>Size</th></tr>");	
	sprintf(buf + strlen(buf),"<tr><th colspan='3'><hr></th></tr>");

	char enstr[1024] = {0};
	char path[1024] = {0};

	int tmp = strlen(dirname);
	// printf("dirname = %s , dirname[tmp-1] = %c\n",dirname, dirname[tmp-1]);
	
	struct dirent** ptr;

	int num = scandir(dirname, &ptr, NULL, alphasort);	

	for (i = 0; i < num; ++i)
	{
		char *name = ptr[i]->d_name;

		if (dirname[tmp-1] == '/')
		{
			sprintf(path,"%s%s",dirname,name);		
		}
		else
		{		
			sprintf(path,"%s/%s",dirname,name);		
		}
		
		struct stat st;
		stat(path,&st);
		
		time_t t;  
	    struct tm *p;  
	    t = st.st_mtime;  
	    p = gmtime(&t);  
	    char mtime[100];  
	    strftime(mtime, sizeof(mtime), "%Y-%m-%d %H:%M:%S", p);  	    
		
		// encode
		encode_str(enstr,sizeof(enstr),path);
				
		if (S_ISREG(st.st_mode))
		{			
			sprintf(buf+strlen(buf),"<tr><td><a href='/%s'>%s</a></td><td>%s</td><td>%ld</td></tr>",
						enstr,name,mtime,(long)st.st_size);						
		}
		else if (S_ISDIR(st.st_mode))
		{			
			sprintf(buf+strlen(buf),"<tr><td><a href='/%s/'>%s/</a></td><td>%s</td><td>-</td></tr>",
						enstr,name,mtime);	
		}

		Write(cfd,buf,strlen(buf));

		memset(buf,0,sizeof(buf));	
	}

	sprintf(buf + strlen(buf),"<tr><th colspan='3'><hr></th></tr>");
	sprintf(buf + strlen(buf),"<tr><th colspan='3'>shttpd/1.0.0 %s:%d</th></tr>",SERV_IP,SERV_PORT);

	sprintf(buf+strlen(buf),"</table><body></html>");
	
	Write(cfd,buf,strlen(buf));

	return;
}


void notfound_response(int cfd){

	char body[1024] = {0};
	char *str = "<!DOCTYPE html><html><head><title>404 NOT FOUND</title></head><body><h1 style='text-align: center;'>NOT FOUND</h1><hr/><p style='text-align: center;'>shttpd/1.0</p></body></html>";
	strcpy(body,str);
	send_respond(cfd,404,"OK","text/html; charset=utf-8",strlen(str));
	
	Write(cfd,body,strlen(str));
}


void error_response(int cfd){

	char body[1024] = {0};
	char *str = "<!DOCTYPE html><html><head><title>500 SERVER ERROR</title></head><body><h1 style='text-align: center;'>500 SERVER ERROR</h1><hr/><p style='text-align: center;'>shttpd/1.0</p></body></html>";
	strcpy(body,str);
	send_respond(cfd,500,"OK","text/heml; charset=utf-8",strlen(str));

	Write(cfd,body,strlen(str));
}


void readcb(int cfd,void *arg){
	int len;
	struct myevent *evp = (struct myevent *)arg;
		
	len = Readline(cfd,evp->buf,BUFSIZ); // 读取http请求行

	// 先从红黑树上取下来
	event_del(g_epfd,evp);
	
	if (len > 0)
	{
		// decode
		decode_str(evp->buf,evp->buf);
		len = strlen(evp->buf);

		evp->buf_len = len;
		evp->buf[len] = '\0';		

		char method[16],path[1024],protocol[16];
		sscanf(evp->buf,"%[^ ] %[^ ] %[^ ]",method,path,protocol);
		printf("Method = %s, Path = %s, Protocol = %s\n",method,path,protocol);
		while(1)
		{
			char buf[2048] = {0};
			len = Readline(cfd,buf,sizeof(buf));			
			 if (len == -1)
			{				
				break;
			}
		}

		// GET请求
		if (strncasecmp(method,"GET",3) == 0)
		{
			char *file = path; // 取出 客户端要访问的文件名
			if (strlen(path) > 1)
			{
				file = path + 1;
			}
			else
			{
				file = "./";
			}
						
			evp->file_len = strlen(file);
			strcpy(evp->file,file);

			// printf("evp->file %s,evp->file_len = %d\n", evp->file,evp->file_len);
		}
		event_init(evp, cfd, writecb, evp);                     //设置该 fd 对应的回调函数为 writecb
        event_add(g_epfd, EPOLLOUT, evp);                       //将fd加入红黑树g_efd中,监听其写事件	
	}
	else
	{
		Close(cfd);
		printf("-----------Client[%d] Closed----------\n", cfd);
	}
}


void writecb(int cfd,void *arg){		
	int ret;
	struct stat sbuf;
	struct myevent *evp = (struct myevent *)arg;	
	do{
		// 判断 请求的文件是否存在
		ret = stat(evp->file,&sbuf);
		if (ret == -1)
		{
			printf("------------------%s Not found file-------------------- \n", evp->file);
			// not found file
			notfound_response(cfd);
			break;
		}			
		const char *file_type = get_file_type(evp->file);
		if (S_ISREG(sbuf.st_mode))
		{	
			send_respond(cfd,200,"OK",file_type,sbuf.st_size);

			// printf("type = %s, filesize = %d\n", file_type ,(int)sbuf.st_size);

			send_file(cfd,evp->file);
		}
		else if (S_ISDIR(sbuf.st_mode)) // 目录
		{
			send_respond(cfd,200,"OK",get_file_type(".html"),-1);

			send_dir(cfd,evp->file);
		}
	}while(0);

	// 先从红黑树上取下来
	event_del(g_epfd,evp);	
	event_init(evp, cfd, readcb, evp);                     	//设置该 fd 对应的回调函数为 recv_data
    event_add(g_epfd, EPOLLIN, evp);                        //将fd加入红黑树g_efd中,监听其读事件	
}


void acceptcb(int lfd,void *arg){
	int cfd,i;
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	char client_ip[100];
	cfd = Accept(lfd,(struct sockaddr *)&client_addr,&addr_len);
	printf("New Client IP:%s ", inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,client_ip,sizeof(client_ip)));
	printf("PORT: %d\n", ntohs(client_addr.sin_port));

	do{
		// 将cfd加入监听红黑树
		for (i = 0; i < EVENTS_SIZE; i++)
		{
			if (g_events[i].status == 0)
			{
				break;
			}
		}

		if (i == EVENTS_SIZE) {
	            printf("%s: Max connect limit[%d]\n", __func__, EVENTS_SIZE);
	            break;                                              //跳出do while(0) 不执行后续代码
	    }

	    int flag = 0;
	    if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) {        //将cfd也设置为非阻塞
	        printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
	        break;
	    }

	    event_init(&g_events[i],cfd,readcb,&g_events[i]);
		event_add(g_epfd,EPOLLIN,&g_events[i]);
		
	}while(0);
	
	return;
}

void show_help(){

	char *str = "Usage: shttpd [options] [-- <shttpdapp> [shttpd app arguments]]\n" \
		VERSION \
		"\n" \
		"Options:\n" \
		" -?,-h 		this help\n" \
		" -v             show version\n" \
		" -d <directory(required option)> chdir to directory before using shttpd\n" \
		" -a <address>   bind to IPv4/IPv6 address (defaults to 127.0.0.1)\n" \
		" -p <port>      bind to TCP-port\n";

	printf("%s\n", str);
	
	return;
}


void show_version(){

	printf("%s - Welcome use shttp web server !\n",VERSION);
	return;
}


int main(int argc, char **argv)
{	
	if (argc < 2)
	{		
		show_help();
		return -1;
	}

	int i,nready,port,o;
	struct epoll_event clients[EVENTS_SIZE + 1];	
	char *rootdir = NULL;
	port = SERV_PORT;
	char *addr = SERV_IP;

	opterr = 0;
	while (-1 != (o = getopt(argc, argv, "d:p:av?h"))) {
		
		switch(o) {		
		case 'd': rootdir = optarg; 
				  break;		
		case 'p': 
			port = atoi(optarg);
			break;		
		case 'a':
			addr = optarg;
			break;
		case 'v': show_version(); return 0;
		case '?':
		case 'h': show_help(); return 0;
		default:
			show_help();
			break;
		}
	}

	if (rootdir == NULL) {
		fprintf(stderr, "shttpd: no root dir given (use -d)\n");
		return -1;
	}	

	// 改变当前工作目录	
	if (rootdir && -1 == chdir(rootdir)) {
		fprintf(stderr, "shttpd: chdir('%s') failed: %s\n", rootdir, strerror(errno));
		return -1;
	}

	g_epfd = epoll_create(EVENTS_SIZE + 1);
	if (g_epfd <= 0)
        printf("create efd in %s err : %s\n", __func__, strerror(errno));
    
	initsocket(g_epfd,port,addr,128);

	printf("http server: [%s:%d]\n", SERV_IP,port);

	/*进入循环体，使用epoll_wait监听客户端事件*/
    while(1)
    {
    	/* 这里可以检测一下客户端连接活跃情况，将不活跃的客户端踢掉 */
    	check_timeout(g_epfd,g_events,60);

    	nready = epoll_wait(g_epfd,clients,EVENTS_SIZE + 1, 1000);
    	
    	if (nready < 0)
    	{
        	printf("epoll_wait in %s err %s\n", __func__, strerror(errno));

    		exit(1);
    	}

    	/*处理epoll_wait监听到的客户端事件*/
    	for (i = 0; i < nready; ++i)
    	{
    		struct myevent *evp = (struct myevent *)clients[i].data.ptr;

    		if ((clients[i].events & EPOLLIN) && (evp->events & EPOLLIN))
    		{
    			evp->callback(evp->fd,evp->arg);    			
    		}
    		if((clients[i].events & EPOLLOUT) && (evp->events & EPOLLOUT)){
    			
    			evp->callback(evp->fd,evp->arg);     			
    		}
    	}
    }	

	return 0;
}