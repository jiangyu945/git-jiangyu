/*
 ============================================================================
 Name        : TCPServer.c
 Author      : jiangyu
 Version     :
 date        : 2018-10-8
 Description : Simple Socket Server
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
 
#include <arpa/inet.h>
#include <sys/socket.h>

#include <signal.h>

#define MYPORT 8080
 
int main( int argc , char ** argv )
{
	signal(SIGCHLD,SIG_IGN);//把子进程的僵尸进程给init进程处理

	struct sockaddr_in saddr, caddr;
 
	unsigned char send_buf[1024];
	unsigned char recv_buf[1024];
	unsigned char ipbuf[50];
	int sockfd, connfd;
	int addr_len;
	pid_t pid;
 
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
 
	memset(&saddr, 0, sizeof(saddr) );
	memset( send_buf, 0, sizeof(send_buf) );
	memset( recv_buf, 0, sizeof(recv_buf) );
	memset( ipbuf, 0, sizeof(ipbuf) );
	saddr.sin_family = AF_INET;
    saddr.sin_port = htons(MYPORT); 
	saddr.sin_addr.s_addr = htonl( INADDR_ANY ); //any address
 
	bind( sockfd, (struct sockaddr *)&saddr, 16 );
 
	listen( sockfd, 20 );
 
	printf( "Accepting connections ... \n" );
 
	int n;
	addr_len = sizeof( caddr );
	
	//循环接收 客户端的连入
	while(1)
	{   
        if((connfd = accept( sockfd, (struct sockaddr*)&caddr, &addr_len ))>0)
		{
			printf("new client connect successful!\n");
		} 
		//打印新接入客户端IP、端口
		inet_ntop(AF_INET,(void*)&caddr.sin_addr.s_addr,ipbuf,50);  //地址格式转换为数值格式
		printf("the client:%s is in\n",ipbuf);

		//创建一个子进程来接受新客户端的信息
		pid = fork();

		if(pid<0)   //fork失败
		{
			perror("failed fork()");
			return -1;
		}
 
		else if(pid == 0)	//子进程
		{
			close(sockfd);//关闭从父进程进程来的监听套接字，因为在子进程用不到，关闭不需要的套接字可节省系统资源，同时可避免父子进程共享这些套接字可能带来的不可预计的后果           			
            
			//这个子进程循环接收客户端信息
			while(1)
			{
				n = recv( connfd, recv_buf, sizeof(recv_buf), 0 );
				if(n<0)
				{
					perror("failed recv");	  
				}
                else if(n == 0)
				{
					bzero(ipbuf,50);					
					inet_ntop(AF_INET,(void*)&caddr.sin_addr.s_addr,ipbuf,50);					
					printf("the client:%s is out\n",ipbuf);				
					exit(0);					
					close(connfd);
				}
				else
				{
					inet_ntop(AF_INET,(void*)&caddr.sin_addr.s_addr,ipbuf,50);
					printf("ip:%s,port:%d\n",ipbuf,ntohs(caddr.sin_port));
					printf("Recived %d bytes\n",n);
					int i;
					for(i=0;i<n;i++)
					{
						printf("%02x", recv_buf[i]);
					}
						printf("\n");
						//printf("Recived %d bytes: %s \n",n, recv_buf);
						send(connfd,recv_buf,n,0);	 
						memset( recv_buf, 0, sizeof(recv_buf) );
						
				}
			}
		}
		else if(pid>0)  //父进程，用于继续监测有没有新的客户端连入
		{
			close(connfd);//关闭新客户端返回的套接字，因为在父进程中用不到			
			continue;
		}
        /*
		printf("please input:\n");
		gets(send_buf);
		send(connfd,send_buf,strlen(send_buf),0);	 
		printf("send :%s\n",send_buf);
		memset( send_buf, 0, sizeof(send_buf) );
		
		
		n = recv( connfd, recv_buf, sizeof(recv_buf), 0 );
		if(n>0)
		{
			printf("Recived %d bytes: %s \n",n, recv_buf);
			send(connfd,recv_buf,n,0);	 
			memset( recv_buf, 0, sizeof(recv_buf) );  
		}
		*/
		
	}

    close( sockfd );
    close( connfd );
	return 0;
}
