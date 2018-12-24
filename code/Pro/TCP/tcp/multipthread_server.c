/*
 ============================================================================
 Name        : multipthread_server.c
 Author      : Greein-jy
 Version     : v1.0
 date        : 2018-10-10
 Description : Multipthread Socket Server
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
#include <sys/select.h>

#define MYPORT 8080
 
#define REQ_FRAME_SIZE    9      //传感器信息协议帧总长

//传感器数据请求格式
#define HEAD    0xAA        
#define LEN     0x09
#define OPTION  0x00
#define CMD     0X01
#define DATA1   0x0f      //数据域，表示需要请求哪些数据
#define DATA2   0xff      //数据域，表示需要请求哪些数据
#define TAIL    0x55

unsigned short CRC_Compute(unsigned char*,unsigned char);

int sockfd, new_sock;
char req_buf[REQ_FRAME_SIZE] = {0};

unsigned char ipbuf[50];
struct sockaddr_in saddr, caddr;


//请求帧构造函数
void req_data_frame()
{	
    req_buf[0]=HEAD;
	req_buf[1]=LEN;
	req_buf[2]=OPTION;
	req_buf[3]=CMD;
	req_buf[4]=DATA1;
	req_buf[5]=DATA2;

	unsigned short crc16 = CRC_Compute(req_buf,(unsigned char)6);
	req_buf[6] = (char)crc16;
    req_buf[7] = (char)(crc16>>8);
	req_buf[8]=TAIL;
}


//线程处理函数
void* pthread_handler(void* sock)
{
    unsigned char recv_buf[1024] = {0};
    int newsock = *((int*)sock);
    int n;
    
    req_data_frame();
    while(1)                        
    { 
        fd_set fds; //定义文件描述符集
        struct timeval timeout;
        timeout.tv_sec = 10;  //设置超时时间为10s
        timeout.tv_usec = 0;
        FD_ZERO(&fds);//清除文件描述符集
        FD_SET(newsock, &fds);//将文件描述符加入文件描述符集中；
    	  
        //发送数据请求  
        send(newsock,req_buf,REQ_FRAME_SIZE,0);
        printf("send req success!\n");   

        if(select(newsock+1,&fds,NULL,NULL,&timeout) > 0 )  //监听socket，超时时间为10s
        {  
            printf("start to receive data...\n");

            //接收数据，处理数据   
            n = recv( newsock, recv_buf, REQ_FRAME_SIZE, 0 );
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
                printf("ip:%s,port:%d\n",ipbuf,ntohs(caddr.sin_port));   
                printf("Recived %d bytes\n",n);
                int i;
                for(i=0;i<n;i++)
                {
                    printf("%02x", recv_buf[i]);
                }
                printf("\n");
                printf("Recived %d bytes: %s \n",n, recv_buf);
                //send(newsock,recv_buf,n,0);	 
                memset( recv_buf, 0, sizeof(recv_buf) );      
            }

            sleep(5);   //延时5s，每5s发送一次数据请求
        }
    }
    close(newsock);   
}


int main( int argc , char ** argv )
{
	int addr_len;
	pid_t pid;
 
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
 
	memset(&saddr, 0, sizeof(saddr) );
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

            //打印新接入客户端IP、端口
            inet_ntop(AF_INET,(void*)&caddr.sin_addr.s_addr,ipbuf,50);  //地址格式转换为数值格式
            printf("the client:%s is in\n",ipbuf);
		} 
		

		//创建一个线程来接收新客户端的信息
        pthread_t id;
		pthread_create(&id,NULL,pthread_handler,(void*)&new_sock);
            
        /*pthread有两种状态:joinable状态和unjoinable状态
        一个线程默认的状态是joinable，如果线程是joinable状态，当线程函数自己返回退出时或pthread_exit时都不会释放线程所占用堆栈和线程描述符（总计8K多）。只有当你调用了pthread_join之后这些资源才会被释放。
        若是unjoinable状态的线程，这些资源在线程函数退出时或pthread_exit时自动会被释放。

        unjoinable属性可以在pthread_create时指定，或在线程创建后在线程中pthread_detach自己, 如：pthread_detach(当前线程ID)，将状态改为unjoinable状态，确保资源的释放。如果线程状态为 joinable,需要在之后适时调用pthread_join.
        */
        pthread_detach(id);	
	}

    close( sockfd );
	return 0;
}

//CRC校验函数
unsigned short  CRC_Compute(unsigned char* snd,unsigned char len)
{
	auto int i, j;
	auto unsigned short  c,crc=0xFFFF;
	for(i = 0;i < len; i ++)
	{
		c = snd[i] & 0x00FF;
		crc ^= c;
		for(j = 0;j < 8; j ++)
		{
			if (crc & 0x0001)
			{
				crc>>=1;
				crc^=0xA001;
			}else crc>>=1;
		}
	}
	return (crc);
}
