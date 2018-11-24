/*
 ============================================================================
 Name        : multipthread_server.c
 Author      : Greein-jy
 Version     : v2.2
 date        : 2018-10-31
 Description : Multipthread Socket Server (add time)
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
#include <time.h>


#define MYPORT 8080

//远端服务器
#define SERVER_IP  "192.168.31.252"
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

unsigned short CRC_Compute(uchar*,uchar);
void req_data_frame(void);
int get_one_frame(int);
void* pthread_handler(void*);

int s_fd,c_fd,new_sock;
uchar req_buf[REQ_FRAME_SIZE] = {0};
struct sockaddr_in saddr, caddr;
uchar ipbuf[50];

uchar send_buf[100] = {0};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  //初始化锁，静态分配


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
	s_fd = socket( AF_INET, SOCK_STREAM, 0 );
   
	memset(&saddr, 0, sizeof(saddr) );
	memset( ipbuf, 0, sizeof(ipbuf) );
	saddr.sin_family = AF_INET;
    saddr.sin_port = htons(MYPORT); 
	saddr.sin_addr.s_addr = htonl( INADDR_ANY ); //any address
 
	bind( s_fd, (struct sockaddr *)&saddr, 16 );
	listen( s_fd, 30 );

    //设置fd可重用  
    int opt=1;  
    if(setsockopt(s_fd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&opt,sizeof(opt)) != 0)  
    {  
        printf("setsockopt error in reuseaddr[%d]\n", s_fd);  
        return ;  
    }  

	printf( "Accepting client connections ... \n" );
 
    return 0;
}

int main( int argc , char ** argv )
{
	int addr_len;

    sock_init();

	addr_len = sizeof( caddr );
	//循环接收 客户端的连入
	while(1)
	{   
        if((new_sock = accept( s_fd, (struct sockaddr*)&caddr, &addr_len ))>0)
		{
			printf("%s:%d connect successfully!\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
		} 
		
		//创建一个线程来接收新客户端的信息
        pthread_t id;
		pthread_create(&id,NULL,pthread_handler,(void*)&new_sock);
    
        pthread_detach(id);	//将状态改为unjoinable状态,unjoinable状态的线程，资源在线程函数退出时或pthread_exit时会被自动释放。线程默认的状态是joinable,需要在之后适时调用pthread_join.
	}

    //销毁锁
    pthread_mutex_destroy(&mutex);
    close(s_fd);
    close(c_fd);
	return 0;
}

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
        ret = send(newsock,req_buf,REQ_FRAME_SIZE,0);
        if(ret == -1){  //连接已断开
            close(newsock);   
            return NULL;   
        }
        printf("send req success!\n");   

        //监听socket，超时时间为10s
        if(select(newsock+1,&fds,NULL,NULL,&timeout) > 0 )  
        {  
            printf("start to receive data...\n");

            //接收数据  
            ret = get_one_frame(newsock);
            if(ret == -1 )
                break;
            sleep(10);   //延时10s，每10s请求一次数据
        }    
    }
    close(newsock);   
}

//接收一帧数据
int get_one_frame(int s_fd)
{
    uchar recv_buf[100] = {0};
    uchar recv_tmp[100] = {0};
	int nByte,get_complete_flag=0,get_head_flag=0,count=0;
	int i,len;

    while(1)
    {
        //接收数据  
        nByte = recv( s_fd, recv_tmp, sizeof(recv_buf), 0 );
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
                    //printf("%s\n",str);
                    
    
                    //加（互斥）锁
                    pthread_mutex_lock(&mutex);

                    memcpy(send_buf,strtime,sizeof(strtime)-1);
                    int k;
                    for(k=0;k<len;k++){
                        hexToStr(recv_buf[k]);
                    }
                    memcpy(&send_buf[sizeof(strtime)-1],recv_buf,len);
                    printf("%s", send_buf);
                    /*
                    for(k=0;k<len;k++){
                        printf("%02x", send_buf[k+sizeof(strtime)-1]);
                    }
                    printf("\n");
                    */
                    //发送到服务端
                    send(c_fd,send_buf,len+sizeof(strtime)-1,0);
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
            close(s_fd);
            break;
        }
    }
    return -1;
}

