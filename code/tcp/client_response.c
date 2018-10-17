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
	
	char recv_buf[1024] = {0};


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
        if((recv(cli_fd,recv_buf,sizeof(recv_buf),0))>0)
        {

            printf("recv: %s\n",recv_buf);
            char send_buf[35]={0x01,0x01,0xAA,0x23,0x80,0x81,0x0f,0xff\
            ,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff\
            ,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x01,0xfe,0x55};
            
            send( cli_fd, send_buf, 35, 0 );
            printf("send data:%s\n",send_buf);
            memset(send_buf,0,sizeof(send_buf));
        }
	}
    

	close(cli_fd);
 
	return 0;
}
