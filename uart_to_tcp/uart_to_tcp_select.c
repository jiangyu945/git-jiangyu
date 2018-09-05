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
#include <sys/select.h>



#define MYPORT  8887
//#define MYPORT  8088

#define BUFFER_SIZE   512
#define DATA_REQ_SIZE  9
#define DATA_SIZE      33


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
	char req_buf[DATA_REQ_SIZE];
	char data_pack[BUFFER_SIZE];

	char read_data[256] = {0};
    char read_buf[256] = {0};

	memset(req_buf,0,sizeof(req_buf));

	

	//构造请求数据
  	req_buf[0]=HEAD;
	req_buf[1]=LEN;
	req_buf[2]=OPTION;
	req_buf[3]=CMD;
	req_buf[4]=DATA1;
	req_buf[5]=DATA2;

	unsigned short crc16 = CRC_Compute(req_buf,(unsigned char)6);
	req_buf[6] = (char)crc16;
    req_buf[7] = (char)(crc16>>8);

	printf("req_buf[6] = %x\n",req_buf[6]);
	printf("req_buf[7] = %x\n",req_buf[7]);
	req_buf[8]=TAIL;
    
	if((fd = open(uart3, O_RDWR|O_NOCTTY|O_NONBLOCK) )<0)   // |O_NDELAY //NONBLOCK 非阻塞打开标志
		printf("open %s is failed",uart3);
	else
	{
		set_opt(fd, 115200, 8, 'N', 1);
        
		//创建套接字并绑定地址
		int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(MYPORT);  
        servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

		printf("connect %s:%d\n",SERVER_IP,MYPORT);  
		//make_socket_non_blocking(sock_cli); 
	    
		//TCP Connect    
	     if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("connect fialed");
            exit(1);
        }
        printf("connect successful!\n");

        
        while(1)
		{         	             							
            fd_set fds; //定义文件描述符集
    		struct timeval timeout;
    		timeout.tv_sec = 10;  //设置超时时间为10s
     		timeout.tv_usec = 0;
    		FD_ZERO(&fds);//清除文件描述符集
     		FD_SET(fd, &fds);//将文件描述符加入文件描述符集中；

       
	        //发送数据请求	
			write(fd,req_buf,DATA_REQ_SIZE); 
			if (select(fd+1,&fds,NULL,NULL,&timeout) > 0 )  //监听串口，超时时间为10s
			{  
				 //读取一个完整数据包
                char read_tmp[256]={0};
	            int nByte,return_flag = 0 ;
	            int i,len_data,totalrd=0;
	            while(1)
	            {
		            memset(read_tmp,0,sizeof(read_tmp)); 
            		if( (nByte=read(fd, read_tmp, sizeof(read_tmp))) >0 )  
                    {
	            		//printf("nByte = %d\n",nByte);
						totalrd += nByte;
						//printf("totalrd = %d\n",totalrd);
	            		for(i=0;i<nByte;i++)
	            		{
							//帧头检测，并取出数据长度
                            if( (read_tmp[i] == HEAD) && (i < (nByte-1) ) )
		            		{
		             			memset(read_data,0,sizeof(read_data));
			            		char tmp[5] = {0};
			            		tmp[0]=read_tmp[i];
			            		strcat(read_data,tmp);
								len_data = read_tmp[i+1];
			            	}
							//帧尾检测
			            	else if(read_tmp[i]==TAIL)
			            	{
			            	 	char tmp[5]={0};
			            		tmp[0]=read_tmp[i];
			            		strcat(read_data,tmp);

			            		return_flag = 1;
			             		memset(read_buf,0,sizeof(read_buf));
			            		memcpy(read_buf,read_data,sizeof(read_data));
								break;
			            	}
			            	else
			            	{
                                char tmp[5]={0};
			            		tmp[0]=read_tmp[i];
			            	  	strcat(read_data,tmp);
			              	}			    	    	   
				    	}

						if(return_flag == 1) 
						{
							//数据长度判断
							if(totalrd == len_data)
							{
								unsigned short result = CRC_Compute(read_buf,strlen(read_buf)-3);
								printf("result = %d\n",result);
								char result_1 = (char)result;
								char result_2 = (char)(result>>8);
																
								//校验和判断
								if( (result_1 == read_buf[strlen(read_buf)-3]) && (result_2 == read_buf[strlen(read_buf)-2]) )         
								{
									//校验正确，可进行数据处理
                                    send(sock_cli,read_buf,strlen(read_buf),0);
						    		printf("send size :%d\n",strlen(read_buf));
						     		break;
								}
								else
								{ 
									//校验失败，退出当前循环
									break;
								}
                                
							}
							else
							{
								//若长度校验不通过，退出当前循环
								break;
							}                           												         	
				    	}	
		    		}					
                } 
	    		sleep(5);
			}        					  								
        }   
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
		newtio.c_cc[VTIME]  = 100; //150*100ms=15s 等待时间，单位百毫秒，0表示一直等待
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
 


		 


