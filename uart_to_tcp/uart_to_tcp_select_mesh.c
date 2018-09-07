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

#define DATA_REQ_MESH_SIZE 15
#define DATA_REQ_SIZE  9


//定义蓝牙mesh数据请求格式
#define HEAD_MESH    0xFE     //帧头
#define CMD_MESH     0x80    //命令符
#define LEN_MESH     0x0B    //长度=数据长度+2
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

char* SERVER_IP = "192.168.31.41";
//char* SERVER_IP = "192.168.31.91";

int set_opt(int,int, int, char, int);
unsigned short CRC_Compute(unsigned char*,unsigned char);


int main()
{
	int fd,k;	
	char *uart3 = "/dev/ttySAC0";
    char req_buf_mesh[DATA_REQ_MESH_SIZE];
	char req_buf[DATA_REQ_SIZE];

    char read_data_mesh[256] = {0};
    char read_buf_mesh[256] = {0};
    char read_buf[256] = {0};

    memset(req_buf_mesh,0,sizeof(req_buf_mesh));
	memset(req_buf,0,sizeof(req_buf));


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
    
    //将 传感器信息请求数据帧 放入 蓝牙mesh请求数据帧
    for(k=5;k<14;k++)
    {
        req_buf_mesh[k] = req_buf[k-5];
    }
    
	if((fd = open(uart3, O_RDWR|O_NOCTTY|O_NONBLOCK) )<0)   // |O_NDELAY //NONBLOCK 非阻塞打开标志
		printf("open %s is failed",uart3);
	else
	{
		set_opt(fd, 9600, 8, 'N', 1);
        
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
			write(fd,req_buf_mesh,DATA_REQ_MESH_SIZE); 
			if (select(fd+1,&fds,NULL,NULL,&timeout) > 0 )  //监听串口，超时时间为10s
			{  
				//读取一个完整数据包
                char read_tmp_mesh[256]={0};
                char read_tmp[256]={0};
				char read_buf_tmp[256]={0};
                int nByte_mesh,return_flag_mesh=0,get_data_flag=0,head_flag=0,count;
	            int return_flag=0;
                int i_mesh,len_data_mesh,totalrd_mesh=0;
				
	            while(1)
	            {
		            memset(read_tmp_mesh,0,sizeof(read_tmp_mesh)); 
            		if( (nByte_mesh=read(fd, read_tmp_mesh, sizeof(read_tmp_mesh))) >0 )  
                    {
						totalrd_mesh += nByte_mesh;
						printf("totalrd_mesh = %d\n",totalrd_mesh);
	            		for(i_mesh=0;i_mesh<strlen(read_tmp_mesh);i_mesh++)
	            		{ 
                            //帧头检测，并取出数据长度
                            if( (read_tmp_mesh[i_mesh] == HEAD_MESH) && (i_mesh < (nByte_mesh-2) ) )
                            {
                                memset(read_data_mesh,0,sizeof(read_data_mesh));
			            		char tmp_mesh[5] = {0};
			            		tmp_mesh[0]=read_tmp_mesh[i_mesh];
			            		strcat(read_data_mesh,tmp_mesh);
								len_data_mesh = read_tmp_mesh[i_mesh+2]+4; //取出数据长度，并计算数据帧总长度
                                head_flag = 1;     //检查到头部标志
								count = len_data_mesh-1;  //count用于计数后面还需取出的字节数
                            }
                            else if(head_flag == 1)
                            { 
                                char tmp_mesh[5]={0};
			            	    tmp_mesh[0]=read_tmp_mesh[i_mesh];
								
                                strcat(read_data_mesh,tmp_mesh);  
                                count--;
                                if(count == 0)
							    {
                                    return_flag_mesh = 1;
                                    memset(read_buf_mesh,0,sizeof(read_buf_mesh));
			            	        memcpy(read_buf_mesh,read_data_mesh,sizeof(read_data_mesh));
								    break;
							    }       
                            }
                            
                        }

                        if(return_flag_mesh == 1)                         												         	  
				     	{
				     		//数据长度判断
					    	if(totalrd_mesh == len_data_mesh)
					     	{																					    		
						    	//进行数据处理,提取传感器数据信息
                                int k_mesh;
                                for(k_mesh=4;k_mesh<(strlen(read_buf_mesh)-1);k_mesh++)
                                {          
                                    read_buf_tmp[k_mesh-4] = read_buf_mesh[k_mesh];        
                                }
                                get_data_flag = 1;   //提取传感器信息成功标志
                            }
                            else
						    { 
						    	//长度校验失败，退出当前循环
							    break;
						    }
                        }
                            

                        if(get_data_flag == 1) //提取成功，进行传感器数据帧处理
                        {
							unsigned short result = CRC_Compute(&read_buf_tmp[2],strlen(read_buf_tmp)-5);
					     	//printf("result = %d\n",result);
						    char result_1 = (char)result;
						    char result_2 = (char)(result>>8);
							//数据校验：帧头、帧尾、长度、校验码
                            if((read_buf_tmp[2]==HEAD) && (read_buf_tmp[len_data_mesh-6]==TAIL) && (read_buf_tmp[3]==(len_data_mesh-5)) && (read_buf_tmp[len_data_mesh-6-2]==result_1) && (read_buf_tmp[len_data_mesh-6-1]==result_2)        )
							{
								//校验正确，可进行数据处理
								memcpy(read_buf,read_buf_tmp,sizeof(read_buf_tmp));
								memset(read_buf_tmp,0,sizeof(read_buf_tmp));
                                send(sock_cli,read_buf,strlen(read_buf),0);
								break;
							}
							else
							{
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
 


		 


