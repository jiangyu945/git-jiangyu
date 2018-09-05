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


#define BUFFER_SIZE 512
#define MAX_EPOLL_EVENT_COUNT 5

int set_opt(int,int, int, char, int);


int main()
{
	int fd;
    int nByte;


    char *uart3 = "/dev/ttySAC3";

    char buffer[BUFFER_SIZE];

    char *uart_out = "please input\r\n";

    if((fd = open(uart3, O_RDWR|O_NOCTTY|O_NDELAY))<0)  
		printf("open %s is failed",uart3);
	else
	{
		set_opt(fd, 115200, 8, 'N', 1);
        write(fd,uart_out,strlen(uart_out));
        

        //创建epoll池
	    int epfd;
		struct epoll_event  ev,events[MAX_EPOLL_EVENT_COUNT];
		ev.events = EPOLLIN|EPOLLET;
		ev.data.fd = fd;
		
	    epfd=epoll_create(5);

        
		//将串口fd可读事件加入epoll池
		if (epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev) != 0) 
		{
		    printf("set epoll error!\n");
		    return -1;
	    }


         while(1)
		{
            
            //等待串口fd可读事件发生
			int i =0,Num= 0;

	        while (1) 
	        {
				//等待串口可读
		        Num = epoll_wait(epfd, events, MAX_EPOLL_EVENT_COUNT, 10000);
               // printf("Num = %d\n", Num);
		        if( Num == 0)
				return 0;
		        for (i = 0; i < Num; i++) 
		        {
			        if (events[i].events & EPOLLIN)
					{    //读取串口数据
						nByte = read(fd, buffer, strlen(buffer));
						write(fd,buffer,strlen(buffer));
                        printf("nByte = %d\n",nByte);
                    }
                }                
            }
        
        }
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
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

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
		newtio.c_cc[VMIN] = 9;  //最小字节数
		tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	
	//	printf("set done!\n\r");
	return 0;
}