#ifndef __USER_TCPCLIENT_H__
#define __USER_TCPCLIENT_H__
#include "ip_addr.h"
#include "espconn.h"


//open server port
#define TCP_SERVER_PORT  	    8888

#define USER_TCP_ROLE   		0  			  //0:client 1:server
#define UART0_DEBUG_LOG			0  			  //1:开启调试打印   0：关闭调试打印

void user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length);
void user_tcp_sent_cb(void *arg);
void user_tcp_discon_cb(void *arg);
void user_tcp_client_send_data(char *pusrdata, unsigned short length);
void user_tcp_server_multi_send(char *pusrdata, unsigned short length);
void user_tcp_connect_cb(void *arg);
void user_tcp_recon_cb(void *arg, sint8 err);
void user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg);
void user_dns_check_cb(void *arg);
void check_tcp_ip(void);
void user_tcp_init(void);
void user_tcp_init(void);


#endif

