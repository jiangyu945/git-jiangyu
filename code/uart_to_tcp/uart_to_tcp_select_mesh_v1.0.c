/**************************************
Company: Greein
Author: Yu Jiang
Function: uart(bluetooth) to tcp
Version: v1.0
Date:2018-09-05
comment: add bluetooth mesh data format
************************************/

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


#define SERVER_IP  "192.168.31.41"
#define MYPORT  8080

#define DATA_REQ_MESH_SIZE 15     //蓝牙协议帧数据总长
#define DATA_REQ_SIZE  9          //传感器信息协议帧总长
#define DATA_TOTAL_SIZE 40        //一帧蓝牙mesh数据包总长

//定义蓝牙mesh数据请求格式
#define HEAD_MESH    0xFE     //帧头
#define CMD_MESH     0x80    //命令符（80表发送，81表接收）
#define LEN_MESH     0x0B    //长度=数据域长度+2
#define TYPE_MESH    0x02    //组播方式
#define NUM_GROUP    0x01    //组号为01
#define TAIL_MESH    0xAA    //帧尾

//传感器数据请求格式
#define HEAD    0xAA        
#define LEN     0x09
#define OPTION  0x00
#define CMD     0X01
#define DATA1   0x0f      //数据域，表示需要请求哪些数据
#define DATA2   0xff      //数据域，表示需要请求哪些数据
#define TAIL    0x55

void req_data_frame(void);
int set_opt(int,int, int, char, int);
unsigned short CRC_Compute(unsigned char*,unsigned char);
void read_one_frame(int);

int fd;	
char *uart3 = "/dev/ttySAC3";

char req_buf_mesh[DATA_REQ_MESH_SIZE] = {0};
char read_data[256] = {0};
char read_buf[256] = {0};
char send_buf[256] = {0};

int main()
{
	//请求帧构造函数
	req_data_frame();
	
	if((fd = open(uart3, O_RDWR|O_NOCTTY|O_NONBLOCK) )<0)   // |O_NDELAY //NONBLOCK 非阻塞打开标志
		printf("open %s is failed",uart3);
	else
	{
		set_opt(fd, 9600, 8, 'N', 1);     //串口初始化
		        
		//创建套接字并绑定地址
		int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
		struct sockaddr_in servaddr;
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(MYPORT);  
		servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

		printf("connect %s:%d\n",SERVER_IP,MYPORT);  
		
		//TCP Connect    
		if(connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
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
			write(fd,req_buf_mesh,DATA_REQ_MESH_SIZE); 
			printf("send req success!\n");
	
			if (select(fd+1,&fds,NULL,NULL,&timeout) > 0 )  //监听串口，超时时间为10s
			{  
				read_one_frame(sock_cli);  //读取一帧数据，并发送给服务器		    		
	    		sleep(5);   //延时5s，每5s发送一次数据请求
			}
			       					  								
        }   
		close(sock_cli);    
	}
    close(fd);	 
    return -1;
}

//请求帧构造函数
void req_data_frame()
{
	
	char req_buf[DATA_REQ_SIZE] = {0};
	//构造蓝牙mesh请求数据帧
    req_buf_mesh[0]=HEAD_MESH;
	req_buf_mesh[1]=CMD_MESH;
	req_buf_mesh[2]=LEN_MESH;
	req_buf_mesh[3]=TYPE_MESH;
	req_buf_mesh[4]=NUM_GROUP;
	req_buf_mesh[14]=TAIL_MESH;

  	//构造传感器信息请求数据帧
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
    
    //将 传感器信息请求数据帧 加入 蓝牙mesh发送数据帧
	memcpy(&req_buf_mesh[5],req_buf,sizeof(req_buf));
}

//串口初始化函数
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) { 
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
    		
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
		newtio.c_cc[VTIME]  = 0; //150*100ms=15s 等待时间，单位百毫秒，0表示一直等待
		newtio.c_cc[VMIN] = 0;  //最小字节数
		tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	
	//	printf("set done!\n\r");
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

//读取并发送
void read_one_frame(int sock_fd)
{
	char read_tmp[256]={0};
	int nByte,totalrd=0,get_complete_flag=0,get_head_flag=0,return_flag=0,count;
	int i,len_data_mesh;
	
	while(1)
	{
		memset(read_tmp,0,sizeof(read_tmp)); 

		//读取串口数据
		if( (nByte=read(fd, read_tmp, sizeof(read_tmp))) >0 )  
		{
			totalrd += nByte;  //保存已读取到的数据总长度
			printf("totalrd: %d\n",totalrd);
			for(i=0;i<strlen(read_tmp);i++)
			{ 
				//帧头检测，并取出数据长度
				if( (read_tmp[i] == HEAD_MESH) && (i < (nByte-2) ) )
				{
					memset(read_data,0,sizeof(read_data));
					char tmp[5] = {0};
					tmp[0]=read_tmp[i];
					strcat(read_data,tmp);
					len_data_mesh = read_tmp[i+2]+4; //取出数据长度，并计算数据帧总长度
					get_head_flag = 1;     //检查到头部标志
					count = len_data_mesh-1;  //count用于计数后面还需取出的字节数
				}
				else if(get_head_flag == 1)
				{ 
					char tmp[5]={0};
					tmp[0]=read_tmp[i];
					strcat(read_data,tmp);  
					count--;
					if(count == 0)  
					{
						get_complete_flag = 1;     //标志位置1，表明已取够一帧完整数据
						memset(read_buf,0,sizeof(read_buf));
						memcpy(read_buf,read_data,sizeof(read_data));
						break;
					}       
				}
				
			}

			if(get_complete_flag == 1)                         												         	  
			{
				//数据长度判断
				if(len_data_mesh == DATA_TOTAL_SIZE)
				{																					    		
					//进行数据处理,提取传感器数据信息
					int k;
					for(k=4;k<(len_data_mesh-1);k++)
					{          
						send_buf[k-4] = read_buf[k];        
					}
					return_flag = 1;   //提取传感器信息成功标志
				}
				else
				{ 
					//长度校验失败，退出当前循环
					break;
				}
			}
				
			//提取成功，进行传感器数据帧处理
			if(return_flag == 1) 
			{
				unsigned short result = CRC_Compute(&send_buf[2],strlen(send_buf)-5);
				char result_1 = (char)result;
				char result_2 = (char)(result>>8);

				//数据校验：帧头、帧尾、长度、校验码
				if((send_buf[2]==HEAD) && (send_buf[len_data_mesh-6]==TAIL) && (send_buf[3]==(len_data_mesh-5)) && (send_buf[len_data_mesh-6-2]==result_1) && (send_buf[len_data_mesh-6-1]==result_2)        )
				{
					//校验正确，进行数据处理
					send(sock_fd,send_buf,strlen(send_buf),0);     //发送数据给服务器
					printf("send to server...\n");
					printf("send %d bytes successful!\n",strlen(send_buf));
					memset(send_buf,0,sizeof(send_buf));
					break;
				}
				else
				{
					break;
				}

            }
        }				
    } 
}

		 


