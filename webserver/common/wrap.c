#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

void perr_exit(const char *s){
	perror(s);
	exit(1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr){

	int ret;
agin:
	ret = accept(fd,sa,salenptr);
	if(ret == -1){
		if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
			goto agin;

		}else{			
			perr_exit("accept error");
		}
	}
	return ret;
}


int Bind(int fd, const struct sockaddr *sa, socklen_t salen){
	int ret;
	ret = bind(fd,sa,salen);
	if(ret < 0){
		perr_exit("bind error");		
	}
	return ret;
}



int Connect(int fd, const struct sockaddr *sa, socklen_t salen){
	int ret;
	ret = connect(fd,sa,salen);
	if(ret < 0){
		perr_exit("connect error");		
	}
	return ret;
}



int Listen(int fd, int backlog){
	int ret;
	ret = listen(fd,backlog);
	if(ret < 0){
		perr_exit("listen error");		
	}
	return ret;
}


int Socket(int family, int type, int protocol){
	int ret;
	ret = socket(family,type,protocol);
	if(ret < 0){
		perr_exit("socket error");
		
	}
	return ret;
}


ssize_t Read(int fd, void *ptr, size_t nbytes){
	ssize_t ret;
agin:
	ret = read(fd,ptr,nbytes);
	if (ret == -1){
		if (errno == EINTR){
			goto agin;			
		}else{
			return -1;
		}
	}
	return ret;
}


ssize_t Write(int fd, const void *ptr, size_t nbytes){
	ssize_t ret;
agin:
	ret = write(fd,ptr,nbytes);
	if (ret == -1){
		if (errno == EINTR || errno == EAGAIN){
			goto agin;			
		}else{
			return -1;
		}
	}
	return ret;
}
int Close(int fd){
	int ret;
	ret = close(fd);
	if (ret < 0){
		perr_exit("close error");		
	}
	return ret;
}

ssize_t Readn(int fd, void *vptr, size_t n){
	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while(nleft > 0){
		if ((nread = read(fd,vptr,n)) < 0){
			if (errno == EINTR){
				nread = 0;
			}
			else{
				return -1;
			}
		}else if (nread == 0){

			break;
		}

		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n){
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;

	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}


ssize_t my_read(int fd, char *ptr){
	static int read_cnt;
	static char *read_ptr;
	static char read_buf[100];

	if (read_cnt <= 0) {
again:
		if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return -1;	
		} else if (read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}


ssize_t Readline(int fd, void *vptr, size_t maxlen){
	ssize_t n, rc;
	char c, *ptr;
	ptr = vptr;

	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			*ptr = 0;
			return n - 1;
		} else
			return -1;
	}
	*ptr = 0;
	return n;
}

