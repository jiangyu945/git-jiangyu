/*
 ============================================================================
 Name        : cli_video.c
 Author      : jiangyu
 Version     :
 date        : 2018-10-8
 Description : Receive h264 video and save as file. 
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#define SERVER_IP  "192.168.31.107"
#define MYPORT        8080
 
unsigned char recv_buf[640*480*3/2] = {0};

int main( int argc, char** argv )
{
	int cli_fd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(MYPORT);
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
 
	if(connect(cli_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("connect fialed");
		exit(1);
	}
	printf("connect successful!\n");

    FILE* fp_h264 = fopen("yuv_640x480.h264", "a+");  //以追加的方式打开
    while(1)
    {
        int ret;
        /*select监听*/
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(cli_fd, &fds);
        ret = select(cli_fd + 1, &fds, NULL, NULL, NULL);
        if(ret >0)
        {
            int n_recv = 0;
            while(1)
            {
                n_recv = recv(cli_fd,recv_buf,sizeof(recv_buf),0);
                if(n_recv > 0){
                    fwrite(recv_buf,n_recv,1,fp_h264);
                    memset(recv_buf,0,sizeof(recv_buf));
                }
                else 
                    break;

            }

        }
    }
    
    

	close(cli_fd);
    fclose(fp_h264);
 
	return 0;
}
