/*
 ============================================================================
 Name        : TCPClient.c
 Author      : jiangyu
 Version     :
 date        : 2018-10-8
 Description : Simple Socket Client
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP  "192.168.31.91"
#define MYPORT        8080
 

int main( int argc, char** argv )
{
	
	char input[1024] = {0};

	int cli_fd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(MYPORT);
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
 
	if(connect(cli_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("connect fialed");
		exit(1);
	}
	printf("connect successful!\n");
    
	while(1)
	{
		printf("please input:\n");
		//gets(input); (linux下没有gets函数，会报警告，使用fgets代替)
		fgets(input,1024,stdin);    //读取少于size长度的字符，直到新的一行开始或是文件结束，最后会在读取的字符串最后一个字符后加一个结束字符’\0’
		send( cli_fd, input, strlen(input), 0 );
		printf("send data:%s\n",input);
		memset(input,0,sizeof(input));
	}
    

	close(cli_fd);
 
	return 0;
}
