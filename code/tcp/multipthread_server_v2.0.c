/*
 ============================================================================
 Name        : multipthread_server.c
 Author      : Greein-jy
 Version     : v0.1
 date        : 2018-10-10
 Description : Multipthread Socket Server (replace strcat use strncpy )
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
 
#define REQ_FRAME_SIZE    9      //数据请求帧长度

//传感器数据请求格式
#define HEAD    0xAA        
#define LEN     0x09
#define OPTION  0x00
#define CMD     0X01
#define DATA1   0x0f      //数据域，表示需要请求哪些数据
#define DATA2   0xff      //数据域，表示需要请求哪些数据
#define TAIL    0x55

unsigned short CRC_Compute(unsigned char*,unsigned char);
void req_data_frame(void);
int get_one_frame(int);
void* pthread_handler(void*);


int sockfd, new_sock;
char req_buf[REQ_FRAME_SIZE] = {0};
unsigned char ipbuf[50];
struct sockaddr_in saddr, caddr;


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
 
	listen( sockfd, 30 );
 
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
    int newsock = *((int*)sock);
    int nByte;
    int ret;

    req_data_frame();
    while(1)                        
    { 
        fd_set fds; //定义文件描述符集
        struct timeval timeout;
        timeout.tv_sec = 10;  //设置超时时间为10s
        timeout.tv_usec = 0;
        FD_ZERO(&fds);//清除文件描述符集
        FD_SET(newsock, &fds);//将文件描述符加入文件描述符集中
    	  
        //发送数据请求  
        send(newsock,req_buf,REQ_FRAME_SIZE,0);
        printf("send req success!\n");   

        //监听socket，超时时间为10s
        if(select(newsock+1,&fds,NULL,NULL,&timeout) > 0 )  
        {  
            printf("start to receive data...\n");

            //接收数据  
            ret = get_one_frame(newsock);
            if(ret == -1 )
                break;
        
            sleep(5);   //延时10s，每10s发送一次数据请求
        }    
    }
    close(newsock);   
}


//接收一帧数据
int get_one_frame(int sockfd)
{
    unsigned char recv_buf[100] = {0};
    unsigned char recv_tmp[100] = {0};
	int nByte,get_complete_flag=0,get_head_flag=0,count=0;
	int i,len;

    while(1)
    {
        //接收数据  
        nByte = recv( sockfd, recv_tmp, sizeof(recv_buf), 0 );
        //printf("nByte:%d\n",nByte);
        if(nByte>0)           
        {
            for(i=0;i<nByte;i++)
            { 
                //帧头检测，并取出数据长度
                if( (recv_tmp[i] == 0x01) && (recv_tmp[i+1] < 0x0a) && (recv_tmp[i+2] == 0xaa) && (i < (nByte-3) ) )
                {
                    char tmp[5] = {0};
                    tmp[0]=recv_tmp[i];
                    //strcat(recv_buf,tmp);     //!!!不能用字符串拼接函数strcat/strncat，否则接收数据中的字符0会被遗弃，造成接收数据出错
                    strncpy(&recv_buf[count],tmp,1);
                    len = recv_tmp[i+3];    //计算帧总长
                    get_head_flag = 1;      //检查到头部标志
                    count++;                //count用于计数后面还需取出的字节数
                }
                else if(get_head_flag == 1)
                { 
                    char tmp[5]={0};
                    tmp[0]=recv_tmp[i]; 
                    strncpy(&recv_buf[count],tmp,1); 
                    count++;
                    if(count == len)  
                    {   
                        get_complete_flag = 1;     //标志位置1，表明已取够一帧完整数据
                        break;    
                    }       
                }
            }

            if(get_complete_flag == 1)                         												         	  
            {
                unsigned short result = CRC_Compute(&recv_buf[2],(unsigned char)(len-5));
                unsigned char result_1 = (unsigned char)result;
                unsigned char result_2 = (unsigned char)(result>>8);
                
                // printf("start to check...\n");
                //数据校验：校验码、帧尾
                //printf("recv_buf[len-3]:%d, result_1:%d,recv_buf[len-2]:%d,result_2:%d,recv_buf[len-1]:%d\n",recv_buf[len-3],result_1,recv_buf[len-2],result_2,recv_buf[len-1]);
                if( (recv_buf[len-3] == result_1) && (recv_buf[len-2] == result_2) && (recv_buf[len-1] == TAIL) )
                {																					    		
                    int k;
                    for(k=0;k<len;k++)
                    {
                        printf("%02x", recv_buf[k]);
                    }
                    printf("\n");
                    memset(recv_buf,0,sizeof(recv_buf));
                    return 0;
                }
                else
                { 
                    //校验失败，丢弃本帧数据
                    printf("Check failed!\n");
                    memset(recv_buf,0,sizeof(recv_buf));
                    return 0;
                }
                
            }    
        }
        
        else 
        {					
            printf("this client has exited!\n");									
            close(sockfd);
            break;
        }
    }
    return -1;
}

