#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <malloc.h>


#define MYPORT  8887
//#define MYPORT  8088

#define BUFFER_SIZE   512
#define DATA_REQ_SIZE  9
#define DATA_SIZE      33

#define MAX_EPOLL_EVENT_COUNT 5 

//定义数据格式
#define HEAD    0xAA
#define LEN     0x09
#define OPTION  0x00
#define CMD     0X01
#define DATA1   0x0f
#define DATA2   0xff  
#define TAIL    0x55

char* SERVER_IP = "192.168.31.41";
//char* SERVER_IP = "192.168.31.91";

int set_opt(int,int, int, char, int);
unsigned short CRC_Compute(unsigned char*,unsigned char);


//设置套接字为非阻塞函数
static int make_socket_non_blocking (int sock)
{
    int flags, s;
    flags = fcntl (sock, F_GETFL, 0);

    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }
    flags |= O_NONBLOCK;
    s = fcntl (sock, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }
    return 0;
}



int main()
{
	int fd;	
	char *uart3 = "/dev/ttySAC3";
	char buffer[BUFFER_SIZE];
	char req_buf[DATA_REQ_SIZE];
    char sendbuf[BUFFER_SIZE];
	char data_pack[BUFFER_SIZE];

		

	char *uart_out = "please input\r\n";
	memset(buffer, 0, sizeof(buffer));
	memset(req_buf, 0, sizeof(req_buf));

	//构造请求数据
    unsigned short crc16 = CRC_Compute(req_buf,6);
	req_buf[0]=HEAD;
	req_buf[1]=LEN;
	req_buf[2]=OPTION;
	req_buf[3]=CMD;
	req_buf[4]=DATA1;
	req_buf[5]=DATA2;
	req_buf[6] = crc16;
    req_buf[7] = crc16>>8;
	req_buf[8]=TAIL;

    

	if((fd = open(uart3, O_RDWR|O_NOCTTY|O_NDELAY) )<0)   // |O_NDELAY //NONBLOCK ?
		printf("open %s is failed",uart3);
	else
	{
		set_opt(fd, 115200, 8, 'N', 1);

		//write(fd,uart_out, strlen(uart_out));
        
		int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(MYPORT);  
        servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

		printf("connect %s:%d\n",SERVER_IP,MYPORT);  
	    
		//TCP Connect    
	     if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("connect fialed");

            exit(1);
        }
        printf("connect successful!\n");

		
		//创建epoll池
	    int epfd;
		struct epoll_event  event;
		struct epoll_event *events;
		event.events = EPOLLIN|EPOLLET;
		event.data.fd = fd;
					
        events = calloc(MAX_EPOLL_EVENT_COUNT,sizeof(struct epoll_event));		
        		
	    epfd=epoll_create(5);

        
		//将串口fd可读事件加入epoll池
		if (epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event) != 0) 
		{
		    printf("set epoll error!\n");
		    return -1;
	    }
	    	    
          
        while(1)
		{
           	//发送数据请求	
			write(fd,req_buf,DATA_REQ_SIZE);
            		
	         //等待串口fd可读事件发生
			int i;
			int n,nByte,totalrd=0;
			n = epoll_wait(epfd,events,MAX_EPOLL_EVENT_COUNT,10000);
            for (i=0;i<n;i++)
	        {
                if( events[i].events & EPOLLIN )
	            {
                    while(1)
                    {    
				        if( (nByte=read(fd, buffer, DATA_SIZE)) >0 )  
                        {
                            memcpy(&data_pack[totalrd],buffer,nByte);					
                            totalrd += nByte;
				        }
				        if(totalrd == DATA_SIZE)
				        {
                            //data_pack[totalrd+1] = '\0';
                            send(sock_cli,data_pack,DATA_SIZE,0);
					    	break;
				        	
                        }	
					}
				}	
			}					
           //延时10s
		    sleep(10);						
	    }  
		close(epfd);  	     
	    free(events);
		close(sock_cli);
    }
	close(fd);	 
	return -1;
}



int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) { 
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	//newtio.c_cflag  |=  CLOCAL | CREAD; //使能接收和本地模式
	//newtio.c_cflag &= ~CSIZE;
    //newtio.c_lflag &=~ICANON;//原始模式    
    //newtio.c_lflag |=ICANON; //标准模式	
    	

	

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E': 
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':  
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;  
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
		newtio.c_cc[VTIME]  = 150; //150*100ms=15s 等待时间，单位百毫秒，0表示一直等待
		newtio.c_cc[VMIN] = DATA_SIZE;  //最小字节数
		tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	
	//	printf("set done!\n\r");
	return 0;
}

unsigned short CRC_Compute(unsigned char* snd,unsigned char len)
{
	auto int i, j;
	auto unsigned short c,crc=0xFFFF;
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

 
		 


