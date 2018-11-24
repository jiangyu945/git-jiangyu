#include "driver/uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"

#include "user_udp.h"

struct espconn user_udp_espconn;
os_timer_t timer_udp_check;


void ICACHE_FLASH_ATTR
user_udp_client_send_data(char *pusrdata, unsigned short length)
{
  char *pbuf = (char *)os_zalloc(100);

  espconn_send(&user_udp_espconn, pusrdata, length);

  os_free(pbuf);
}

void ICACHE_FLASH_ATTR
udp_client_sent_cb(void *arg)
{
    os_printf("\r\n udp client send ok \r\n");
}

void ICACHE_FLASH_ATTR
user_udp_recv_cb(void *arg,char *pdata, unsigned short len)
{
	uart0_tx_buffer(pdata,len);
}

LOCAL void ICACHE_FLASH_ATTR
udp_server_sent_cb(void *arg)
{
	// TODO:
}

void ICACHE_FLASH_ATTR
udp_server_init(uint32 port)
{
	static struct espconn s_udp_server;
	static esp_udp s_espudp;

	s_udp_server.type = ESPCONN_UDP;
	s_udp_server.state = ESPCONN_NONE;
	s_udp_server.proto.udp = &s_espudp;
	s_udp_server.proto.udp->local_port = port;

	espconn_regist_recvcb(&s_udp_server, user_udp_recv_cb);		//注册UDP接收回调函数
	espconn_regist_sentcb(&s_udp_server, udp_server_sent_cb); 	//注册UDP发送回调函数
	espconn_create(&s_udp_server);

	os_printf("udp_server_init\r\n");
}

void ICACHE_FLASH_ATTR
check_udp_ip(void)
{
	struct ip_info ipconfig;

	//get ip info of ESP8266 station
	wifi_get_ip_info(STATION_IF, &ipconfig);

    //如果状态正确，证明已经连接
    if (wifi_station_get_connect_status() == STATION_GOT_IP  && ipconfig.ip.addr != 0)
    {
    	os_printf("Connected to router and assigned IP!\r\n");
        os_timer_disarm(&timer_udp_check);

		#if (USER_UDP_ROLE == 0)
        {
			wifi_set_broadcast_if(STATION_MODE);     //设置 ESP8266 发送 UDP广播包时，从 station 接口发送
			user_udp_espconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));//分配空间
			user_udp_espconn.type = ESPCONN_UDP;                                //设置类型为UDP协议
			user_udp_espconn.proto.udp->local_port = espconn_port();            //本地端口
			user_udp_espconn.proto.udp->remote_port = 8888;                     //目标端口
			const char udp_remote_ip[4] = {192,168,31,103};                     //目标IP地址（广播）
			os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);

			espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb);         //接收回调
			espconn_regist_sentcb(&user_udp_espconn, udp_client_sent_cb);       //发送回调
			espconn_create(&user_udp_espconn);            						//建立 UDP传输

			espconn_send(&user_udp_espconn, "Connected to UDP Server", strlen("Connected to UDP Server"));
        }
		#else
        {
        	udp_server_init(UDP_SERVER_PORT);
        }
		#endif
    }
}

void ICACHE_FLASH_ATTR
user_udp_init() //初始化
{
    wifi_set_opmode(STATION_MODE); 			   		//设置为STATION模式

    struct station_config stationConf;
    os_strcpy(stationConf.ssid, "Greein");     		//改成你要连接的 路由器的用户名
    os_strcpy(stationConf.password, "Greein@888"); 	//改成你要连接的路由器的密码

    wifi_station_set_config(&stationConf);     		//设置WiFi station接口配置，并保存到 flash
    wifi_station_connect();                    		//连接路由器

    os_timer_disarm(&timer_udp_check);  			//取消定时器定时
    os_timer_setfn(&timer_udp_check, (os_timer_func_t *) check_udp_ip,NULL);    //设置定时器回调函数
    os_timer_arm(&timer_udp_check, 500, 1);      	//启动定时器，单位：毫秒
}

