#ifndef _USER_UDP_H_
#define _USER_UDP_H_

#define USER_UDP_ROLE			0   //0��client   1��server

#define UDP_SERVER_PORT			8888

void user_udp_client_send_data(char *pusrdata, unsigned short length);
void user_udp_init();


#endif
