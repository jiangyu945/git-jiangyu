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
#include <pthread.h>

#include <signal.h>

#define MYPORT 8080
 
int sockfd, new_sock;
unsigned char send_buf[1024];
unsigned char recv_buf[1024];
unsigned char ipbuf[50];
struct sockaddr_in saddr, caddr;

pthread_mutex_t mutex; //互斥锁

void* handler(void* sock)
{
    int newsock = *((int*)sock);
    int n;
    while(1)
    {
        pthread_mutex_lock(&mutex); //上锁

        n = recv( newsock, recv_buf, sizeof(recv_buf), 0 );
        if(n<0)
        {
            perror("failed recv");	  
        }
        else if(n == 0)
        {
            memset( ipbuf, 0, sizeof(ipbuf) );					
            inet_ntop(AF_INET,(void*)&caddr.sin_addr.s_addr,ipbuf,50);					
            printf("the client:%s is quit\n",ipbuf);									
            close(newsock);
            break;
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
            send(newsock,recv_buf,n,0);	 
            memset( recv_buf, 0, sizeof(recv_buf) );       
        }

        pthread_mutex_unlock(&mutex); //解锁
    }
    close(newsock);   
}


int main( int argc , char ** argv )
{
    pthread_mutex_init(&mutex, NULL); //初始化互斥锁

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
 
	addr_len = sizeof( caddr );
	
	//循环接收 客户端的连入
	while(1)
	{   
        if((new_sock = accept( sockfd, (struct sockaddr*)&caddr, &addr_len ))>0)
		{
			printf("new client connect successful!\n");
		} 
		//打印新接入客户端IP、端口
		inet_ntop(AF_INET,(void*)&caddr.sin_addr.s_addr,ipbuf,50);  //地址格式转换为数值格式
		printf("the client:%s is in\n",ipbuf);

		//创建一个线程来接受新客户端的信息
        pthread_t id;
		pthread_create(&id,NULL,handler,(void*)&new_sock);
            
        /*pthread有两种状态:joinable状态和unjoinable状态
        一个线程默认的状态是joinable，如果线程是joinable状态，当线程函数自己返回退出时或pthread_exit时都不会释放线程所占用堆栈和线程描述符（总计8K多）。只有当你调用了pthread_join之后这些资源才会被释放。
        若是unjoinable状态的线程，这些资源在线程函数退出时或pthread_exit时自动会被释放。

        unjoinable属性可以在pthread_create时指定，或在线程创建后在线程中pthread_detach自己, 如：pthread_detach(当前线程ID)，将状态改为unjoinable状态，确保资源的释放。如果线程状态为 joinable,需要在之后适时调用pthread_join.
        */
        pthread_detach(id);	
	}

    pthread_mutex_destroy(&mutex); //销毁互斥锁
    close( sockfd );
	return 0;
}
