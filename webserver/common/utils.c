// 工具库

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"


const char *get_file_type(const char *name){

	char* dot; 
    dot = strrchr(name, '.');	//自右向左查找‘.’字符;如不存在返回NULL
	if (dot == NULL)
		return "text/plain; charset=utf-8";
	if (strcmp(dot,".html") == 0 || strcmp(dot,".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
		return "image/gif";
    if (strcmp(dot, ".png") == 0)
		return "image/png";
    if (strcmp(dot, ".css") == 0)	
		return "text/css";
    if (strcmp(dot, ".au") == 0)	
		return "audio/basic";
    if (strcmp( dot, ".wav") == 0)
		return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)	
		return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)	
		return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)	
		return "application/ogg";
    if (strcmp(dot, ".pac") == 0)	
		return "application/x-ns-proxy-autoconfig";
 
	return "text/plain; charset=iso-8859-1";
	
}


//16进制数转化为10进制, return 0不会出现 
int hexit(char c)
{
    if (c >= '0' && c <= '9')
		return c - '0';
    if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
 
    return 0;		
}


/*
 * 这里的内容是处理%20之类的东西！是"解码"过程。
 * %20 URL编码中的‘ ’(space)
 * %21 '!' %22 '"' %23 '#' %24 '$'
 * %25 '%' %26 '&' %27 ''' %28 '('......
 * 相关知识html中的‘ ’(space)是 
 */
void decode_str(char *to, char *from)
{
    for ( ; *from != '\0'; ++to, ++from) {
	
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) { //依次判断from中 %20 三个字符
	    
	    	*to = hexit(from[1])*16 + hexit(from[2]);
	    	from += 2;                      //移过已经处理的两个字符(%21指针指向1),表达式3的++from还会再向后移一个字符
	    } else
	    	*to = *from;
	}
    *to = '\0';
}

//"编码"，用作回写浏览器的时候，将除字母数字及/_.-~以外的字符转义后回写。
//strencode(encoded_name, sizeof(encoded_name), name);
void encode_str(char* to, size_t tosize, const char* from)
{
    int tolen;
 
    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {
		if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) {
			*to = *from;
			++to;
			++tolen;
		} else {
			sprintf(to, "%%%02x", (int) *from & 0xff);	
			to += 3;
			tolen += 3;
		}
	}
    *to = '\0';
}
