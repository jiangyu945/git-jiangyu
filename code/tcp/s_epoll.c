 /*
 ============================================================================
 Name        : multipthread_server.c
 Author      : Greein-jy
 Version     : v2.2
 date        : 2018-10-31
 Description : Multipthread Socket Server (epoll)

 Question    : 不能做到定时发送请求！
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
#include <time.h>

#include <sys/epoll.h>
#include <errno.h>


#define MYPORT 8080

//远端服务器
#define SERVER_IP  "192.168.31.41"
#define PORT_S        8887
 
#define REQ_FRAME_SIZE    9      //数据请求帧长度

//传感器数据请求格式
#define HEAD    0xAA        
#define LEN     0x09
#define OPTION  0x00
#define CMD     0X01
#define DATA1   0x0f      //数据域，表示需要请求哪些数据
#define DATA2   0xff      //数据域，表示需要请求哪些数据
#define TAIL    0x55

typedef  unsigned char  uchar;


int listenfd,c_fd,epfd;
uchar req_buf[REQ_FRAME_SIZE] = {0};
struct sockaddr_in saddr, caddr;

uchar send_buf[100] = {0};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  //初始化锁，静态分配

//CRC校验函数
unsigned short  CRC_Compute(uchar* snd,uchar len)
{
	auto int i, j;
	auto unsigned short  c,crc=0xFFFF;
	for(i = 0;i < len; i ++){
		c = snd[i] & 0x00FF;
		crc ^= c;
		for(j = 0;j < 8; j ++){
			if (crc & 0x0001){
				crc>>=1;
				crc^=0xA001;
			}else crc>>=1;
		}
	}
	return (crc);
}

uchar hexToStr(uchar tmp)
{
    if (tmp < 10){
        tmp = tmp + '0';
    }else{
        tmp = tmp -10 +'A';
    }
    return tmp;
}

int sock_init()
{   
    //socket init（As a client）
    c_fd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT_S);
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if(connect(c_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("connect fialed");
		return -1;
	}
	printf("connected server!\n");

    //socket init（As a server）
	listenfd = socket( AF_INET, SOCK_STREAM, 0 );
   
	memset(&saddr, 0, sizeof(saddr) );
	saddr.sin_family = AF_INET;
    saddr.sin_port = htons(MYPORT); 
	saddr.sin_addr.s_addr = htonl( INADDR_ANY ); //any address
 
	bind( listenfd, (struct sockaddr *)&saddr, 16 );
	listen( listenfd, 30 );

    //设置fd可重用  
    int opt=1;  
    if(setsockopt(listenfd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&opt,sizeof(opt)) != 0)  
    {  
        printf("setsockopt error in reuseaddr[%d]\n", listenfd);  
        return ;  
    }  

	printf( "Accepting client connections ... \n" );
 
    return 0;
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

	unsigned short crc16 = CRC_Compute(req_buf,(uchar)6);
	req_buf[6] = (uchar)crc16;
    req_buf[7] = (uchar)(crc16>>8);
	req_buf[8]=TAIL;
}

//接收一帧数据
int get_one_frame(int connfd)
{
    uchar recv_buf[100] = {0};
    uchar recv_tmp[100] = {0};
	int nByte,get_complete_flag=0,get_head_flag=0,count=0;
	int i,len;

    while(1)
    {
        //接收数据  
        nByte = recv( connfd, recv_tmp, sizeof(recv_buf), MSG_DONTWAIT );
        //printf("nByte:%d\n",nByte);
        if(nByte>0)           
        {
            for(i=0;i<nByte;i++)
            { 
                //帧头检测，并取出数据长度
                if( (get_head_flag == 0) && (recv_tmp[i] == 0xaa) && (i < (nByte-1) ) )
                {
                    uchar tmp[5] = {0};
                    tmp[0]=recv_tmp[i];
                    //strcat(recv_buf,tmp);     //!!!不能用字符串拼接函数strcat/strncat，否则接收数据中的字符0会被遗弃，造成接收数据出错
                    strncpy(&recv_buf[count],tmp,1);
                    len = recv_tmp[i+1];        //计算帧总长
                    if(len < 80){
                        get_head_flag = 1;      //检查到头部标志
                        count++;                //count用于计数后面还需取出的字节数
                    }
                    else{
                        memset(recv_buf,0,sizeof(recv_buf));
                    }
                }
                
                else if(get_head_flag == 1)
                { 
                    uchar tmp[5]={0};
                    tmp[0]=recv_tmp[i]; 
                    strncpy(&recv_buf[count],tmp,1); 
                    count++;
                    if(count == len){   
                        get_complete_flag = 1;     //标志位置1，表明已取够一帧完整数据
                        break;    
                    }       
                }
            }

            if(get_complete_flag == 1)                         												         	  
            {
                unsigned short result = CRC_Compute(recv_buf,(uchar)(len-3));
                uchar result_1 = (uchar)result;
                uchar result_2 = (uchar)(result>>8);
                
                //数据校验：校验码、帧尾
                //printf("recv_buf[len-3]:%d, result_1:%d,recv_buf[len-2]:%d,result_2:%d,recv_buf[len-1]:%d\n",recv_buf[len-3],result_1,recv_buf[len-2],result_2,recv_buf[len-1]);
                if( (recv_buf[len-3] == result_1) && (recv_buf[len-2] == result_2) && (recv_buf[len-1] == TAIL) )
                {	
                    //校验通过，数据处理																				    		

                    //获取当前时间
                    struct tm *ptr;
                    time_t t;
                    uchar strtime[20];
                    time(&t);
                    ptr = localtime(&t);
                    strftime(strtime,sizeof(strtime),"%Y-%m-%d %H:%M:%S",ptr);
                    
    
                    //加（互斥）锁
                    pthread_mutex_lock(&mutex);

                    memcpy(send_buf,strtime,sizeof(strtime)-1);
                    int k;
                    for(k=0;k<len;k++){
                        hexToStr(recv_buf[k]);
                    }
                    memcpy(&send_buf[sizeof(strtime)-1],recv_buf,len);
                    printf("%s", send_buf);
                
                    //发送到服务端
                    send(c_fd,send_buf,len+sizeof(strtime)-1,MSG_DONTWAIT);
                    memset(send_buf,0,sizeof(send_buf));
                    
                    //解锁
                    pthread_mutex_unlock(&mutex);

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
            else{
                memset(recv_buf,0,sizeof(recv_buf));
                printf("The data length is failed!\n");
                return -2;
            }    
        }
        
        else if(nByte == 0)
        {	
            printf("Connect close!!!\n");												
            close(connfd);
            break;
        }
    }
    return -1;
}

int main( int argc , char ** argv )
{
    int i,ret;
    int connfd;
	int addr_len;

    sock_init();

    struct epoll_event ev,events[100];

    epfd = epoll_create(512);

    ev.data.fd = listenfd;       
    ev.events = EPOLLIN | EPOLLET; 
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev); 

	
    req_data_frame();
	//循环接收 客户端的连入
	while(1)
	{   
        int nfds = epoll_wait(epfd,events,50,-1);
        if(nfds > 0)
        {
            for(i=0;i<nfds;++i)
            {
                if(!(events[i].events & EPOLLIN)){
                    continue;
                }
                if(events[i].data.fd == listenfd){   //有新的连接

                    addr_len = sizeof( caddr );
                    if( (connfd=accept(listenfd,(struct sockaddr*)&caddr,&addr_len)) > 0 )
                    {
                        printf("%s:%d connect successfully!\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
                    } 

                    /* set NONBLOCK */  
                    int flags = fcntl(connfd, F_GETFL, 0);  
                    fcntl(connfd, F_SETFL, flags | O_NONBLOCK); 

                    //监听读事件
                    ev.events = EPOLLIN|EPOLLET;
                    ev.data.fd = connfd;

                    //添加至监听
                    epoll_ctl(epfd,EPOLL_CTL_ADD, connfd, &ev);

                    //发送数据请求
                    ret = send(connfd,req_buf,REQ_FRAME_SIZE,MSG_DONTWAIT);  //flags设置为MSG_DONTWAIT，以非阻塞方式发送
                    if(ret == -1){   //客户端连接关闭
                        printf("%s:%d connect closed!\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

                        //从监听中移除
				        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        close(events[i].data.fd);   
                        break;   
                    }

                }
                else{   //读取客户端消息 
                    printf("start to receive data...\n");
                    ret = get_one_frame(events[i].data.fd);
                    if(ret == -1 )
                        break;

                    ret = send(events[i].data.fd,req_buf,REQ_FRAME_SIZE,MSG_DONTWAIT);
                    if(ret == -1){   //客户端连接关闭
                        printf("%s:%d connect closed!\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

                        //从监听中移除
				        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        close(events[i].data.fd);   
                        break;   
                    }
                }    
            } 
        }
    }

    //销毁锁
    pthread_mutex_destroy(&mutex);
    close(listenfd);
    close(c_fd);
	return 0;
}











    