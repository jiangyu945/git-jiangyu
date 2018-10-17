/**************************************
Company: Greein
Author: Yu Jiang
Function: uart(bluetooth) to tcp
Version: v2.0
Date:2018-10-06
comment: format optimize (div-package)
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

#define REQ_SIZE_M 10 

//定义蓝牙mesh数据请求格式
#define HEAD    0xFE     //帧头
#define M_CMD   0x80    //命令符（80表发送，81表接收）
#define LEN     0x06    //长度=数据域长度+2
#define M_TYPE  0x02    //组播方式
#define ADDR    0x01    //组号为01
#define TAIL    0xAA    //帧尾

//传感器数据请求格式
#define ID      0x01       //地域ID  
#define D_CMD   0X01       //命令

void req_data_frame(void);
int set_opt(int,int, int, char, int);
unsigned char CheckSum(unsigned char*,unsigned char);
void read_one_frame(int);

int fd;	
char *uart_dev = "/dev/ttySAC3";

char req_buf_m[REQ_SIZE_M] = {0};
char read_data[256] = {0};
char read_buf[256] = {0};

char d_buf[256] = {0};
char d_buf1[256] = {0};
char d_buf2[256] = {0};
char d_buf3[256] = {0};
char d_buf4[256] = {0};

char send_to_server[256] = {0};
char addr;      //来源节点号
char D_TYPE;   //分包号

int main()
{	
	if((fd = open(uart_dev, O_RDWR|O_NOCTTY|O_NONBLOCK) )<0)   // O_NDELAY 或者 NONBLOCK 非阻塞打开标志
		printf("open %s is failed",uart_dev);
	else
	{
		set_opt(fd, 115200, 8, 'N', 1);     //串口初始化
		        
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

		//主循环
        while(1)
		{      	             							
            fd_set fds; //定义文件描述符集
    		struct timeval timeout;
    		timeout.tv_sec = 10;  //设置超时时间为10s
     		timeout.tv_usec = 0;
    		FD_ZERO(&fds);//清除文件描述符集
     		FD_SET(fd, &fds);//将文件描述符加入文件描述符集中；
       
	        //发送数据请求
            char num_p;
            for(num_p=1;num_p<5;num_p++)
            {
                D_TYPE = num_p;  
	            req_data_frame();   //构造请求帧
                write(fd,req_buf_m,sizeof(req_buf_m)); 
                printf("send req success!\n");
        
                if (select(fd+1,&fds,NULL,NULL,&timeout) > 0 )  //监听串口，超时时间为10s
                {  
                    read_one_frame(sock_cli);  //读取一帧数据，并发送给服务器		    		
                    sleep(5);   //延时5s，每5s发送一次数据请求
                }     
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
	
	char req_buf[REQ_SIZE_M] = {0};
	//构造蓝牙mesh请求数据帧
    req_buf_m[0]=HEAD;
	req_buf_m[1]=M_CMD;
	req_buf_m[2]=LEN;
	req_buf_m[3]=M_TYPE;
	req_buf_m[4]=ADDR;
	

  	//构造传感器信息请求数据帧
    req_buf[0]=ID;
	req_buf[1]=D_CMD;
	req_buf[2]=D_TYPE;
	req_buf[3]=CheckSum(req_buf,3);
	printf("CheckSum :%x\n",CheckSum(req_buf,3));
  
    //将 传感器信息请求数据帧 填入 蓝牙mesh发送数据帧
	memcpy(&req_buf_m[5],req_buf,sizeof(req_buf));
	req_buf_m[REQ_SIZE_M-1]=TAIL;
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
unsigned char CheckSum(unsigned char* snd,unsigned char len)
{
    unsigned char sum = 0;
	unsigned char* p = snd;
	int i;
    for(i=0;i<len;i++)
	{
        sum += p[i];
    }
    
	return sum;
}

//数据读取、处理函数
void read_one_frame(int sock_fd)
{
	char read_tmp[256]={0};
    
	int nByte,totalrd=0,get_complete_flag=0,get_head_flag=0,return_flag=0,count,count_msg=0,got_flag1=0,got_flag2=0,got_flag3=0,got_flag4=0;
	int i,len_data_mesh;
	
	while(1)
	{
		memset(read_tmp,0,sizeof(read_tmp)); 

		//读取串口数据
		if( (nByte=read(fd, read_tmp, sizeof(read_tmp))) >0 )  
		{
			printf("nByte:%d\n",nByte);
			totalrd += nByte;  //保存已读取到的数据总长度
			printf("totalrd: %d\n",totalrd);
			for(i=0;i<strlen(read_tmp);i++)
			{ 
				//帧头检测，并取出数据长度
				if( (read_tmp[i] == HEAD) && (i < (nByte-2) ) )
				{
					memset(read_data,0,sizeof(read_data));
					char tmp[5] = {0};
					tmp[0]=read_tmp[i];
					strcat(read_data,tmp);
					len_data_mesh = read_tmp[i+2]+4;   //取出数据长度，并计算数据帧总长度
					get_head_flag = 1;                //检查到头部标志
					count = len_data_mesh-1;         //count用于计数后面还需取出的字节数
					printf("count:%d\n",count);
				}
				else if(get_head_flag == 1)
				{ 
					char tmp[5]={0};
					tmp[0]=read_tmp[i];
					strcat(read_data,tmp);  
					count--;
					printf("count:%d\n",count);
					if(count == 0)                  //count==0代表取足一帧数据
					{
						memset(read_buf,0,sizeof(read_buf));
						memcpy(read_buf,read_data,sizeof(read_data));

                        //提取传感器信息
                        int k;
                        for(k=4;k<(len_data_mesh-1);k++)
                        {          
                            d_buf[k-4] = read_buf[k];        
                        }

                        //传感器数据校验：校验和
                        if( d_buf[9] == CheckSum(&d_buf[3],6) )
                        {
                            //校验正确，进行数据处理
                            addr = read_buf[3];
                            switch(addr)                   //根据ADDR将数据分别存入对应buffer
                            {
                                case 01:
									if(got_flag1 == 0)
									{
										strncat(d_buf1,&d_buf[3],6);
										get_head_flag = 0;	
										count_msg++;             //用于计数已成功读取到的消息数
										got_flag1 == 1;           //标记已取得数据
										if(strlen(d_buf1) == 24)
										{
											//构造发送数据(发给server))
											send_to_server[0] = HEAD;
											send_to_server[1] = M_CMD;
											send_to_server[2] = 0xFE;   //数据帧总长度
											send_to_server[3] = 0x01;   //区域ID
											send_to_server[4] = addr;   //来源节点ADDR

											memcpy(&send_to_server[5],d_buf1,sizeof(d_buf1));
											send_to_server[29] = CheckSum(&send_to_server[4],24);  //计算校验和
											send_to_server[30] = TAIL;
											send(sock_fd,send_to_server,strlen(send_to_server),0);     //发送数据给服务器

											printf("send node %x data successful!\n",read_buf[3]);
                      					    memset(d_buf1,0,sizeof(d_buf1));
											memset(send_to_server,0,sizeof(send_to_server));
										}
									}                                   								                           
                                    break;

                                case 02:
                                    if(got_flag2 == 0)
									{
										strncat(d_buf2,&d_buf[3],6);
										get_head_flag = 0;
										count_msg++;             //用于计数已成功读取到的消息数	
										got_flag2 == 1;           //标记已取得数据
										if(strlen(d_buf2) == 24)
										{
											//构造发送数据(发给server))
											send_to_server[0] = HEAD;
											send_to_server[1] = M_CMD;
											send_to_server[2] = 0xFE;   //数据帧总长度
											send_to_server[3] = 0x01;   //区域ID
											send_to_server[4] = addr;   //来源节点ADDR

											memcpy(&send_to_server[5],d_buf2,sizeof(d_buf2));
											send_to_server[29] = CheckSum(&send_to_server[4],24);  //计算校验和
											send_to_server[30] = TAIL;
											send(sock_fd,send_to_server,strlen(send_to_server),0);     //发送数据给服务器

											printf("send node %x data successful!\n",read_buf[3]);
                      					    memset(d_buf2,0,sizeof(d_buf2));
											memset(send_to_server,0,sizeof(send_to_server));
										}
									}                         
                                    break;

                                case 03:
                                    if(got_flag3 == 0)
									{
										strncat(d_buf3,&d_buf[3],6);
										get_head_flag = 0;
										count_msg++;             //用于计数已成功读取到的消息数	
										got_flag3 == 1;           //标记已取得数据
										if(strlen(d_buf3) == 24)
										{
											//构造发送数据(发给server))
											send_to_server[0] = HEAD;
											send_to_server[1] = M_CMD;
											send_to_server[2] = 0xFE;   //数据帧总长度
											send_to_server[3] = 0x01;   //区域ID
											send_to_server[4] = addr;   //来源节点ADDR

											memcpy(&send_to_server[5],d_buf3,sizeof(d_buf3));
											send_to_server[29] = CheckSum(&send_to_server[4],24);  //计算校验和
											send_to_server[30] = TAIL;
											send(sock_fd,send_to_server,strlen(send_to_server),0);     //发送数据给服务器

											printf("send node %x data successful!\n",read_buf[3]);
                      					    memset(d_buf3,0,sizeof(d_buf3));
											memset(send_to_server,0,sizeof(send_to_server));
										}
									}        
                                    break;

                                case 04:
                                    if(got_flag4 == 0)
									{
										strncat(d_buf4,&d_buf[3],6);
										get_head_flag = 0;
										count_msg++;             //用于计数已成功读取到的消息数	
										got_flag4 == 1;           //标记已取得数据
										if(strlen(d_buf4) == 24)
										{
											//构造发送数据(发给server))
											send_to_server[0] = HEAD;
											send_to_server[1] = M_CMD;
											send_to_server[2] = 0xFE;   //数据帧总长度
											send_to_server[3] = 0x01;   //区域ID
											send_to_server[4] = addr;   //来源节点ADDR

											memcpy(&send_to_server[5],d_buf4,sizeof(d_buf4));
											send_to_server[29] = CheckSum(&send_to_server[4],24);  //计算校验和
											send_to_server[30] = TAIL;
											send(sock_fd,send_to_server,strlen(send_to_server),0);     //发送数据给服务器

											printf("send node %x data successful!\n",read_buf[3]);
                      					    memset(d_buf4,0,sizeof(d_buf4));
											memset(send_to_server,0,sizeof(send_to_server));
										}
									}        
                                    break;

                                default:
                                    printf("This ADDR is unknown!\n");
                                    break; 
                            }
                            
                        }
						
					}       
				}
				
			}
			
        }
		else
		{
			if( count_msg == 2 )
			break;
		}
						
    } 
}

		 


