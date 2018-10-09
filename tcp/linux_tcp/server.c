/*
 ============================================================================
 Name        : TCPServer.c
 Author      : jiangyu
 Version     :
 date        : 2018-10-8
 Description : Simple Socket Server
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

#define MYPORT 8080
 
int main( int argc , char ** argv )
{
	struct sockaddr_in saddr, caddr;
 
	char buf[1024];
	int sockfd, connfd;
	int addr_len;
 
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
 
	memset(&saddr, 0, sizeof(saddr) );
	memset( buf, 0, sizeof(buf) );
	saddr.sin_family = AF_INET;
    saddr.sin_port = htons(MYPORT);
    //saddr.sin_addr.s_addr = inet_addr( INADDR_ANY ); 
	saddr.sin_addr.s_addr = htonl( INADDR_ANY ); //any address
 
	bind( sockfd, (struct sockaddr *)&saddr, 16 );
 
	listen( sockfd, 20 );
 
	printf( "Accepting connections ... \n" );
 
	int n;
	while(1)
	{
		addr_len = sizeof( caddr );
		connfd = accept( sockfd, (struct sockaddr*)&caddr, &addr_len );
		n = recv( connfd, buf, 100, 0  );	

		printf("Recived %d bytes: %s \n",n, buf);
		close( connfd );
	}
 
	return 0;
}
