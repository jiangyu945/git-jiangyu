#include "net.h"


int init_net()
{
    struct sockaddr_in saddr, caddr;
    int addr_len;

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
 
	memset(&saddr, 0, sizeof(saddr) );
	saddr.sin_family = AF_INET;
    saddr.sin_port = htons(MYPORT);
	saddr.sin_addr.s_addr = htonl( INADDR_ANY ); //any address
 
	bind( sockfd, (struct sockaddr *)&saddr, 16 );
 
	listen( sockfd, 20 );
 
	printf( "Accepting connections ... \n" );

    addr_len = sizeof( caddr );
	connfd = accept( sockfd, (struct sockaddr*)&caddr, &addr_len );

    return 0;
}