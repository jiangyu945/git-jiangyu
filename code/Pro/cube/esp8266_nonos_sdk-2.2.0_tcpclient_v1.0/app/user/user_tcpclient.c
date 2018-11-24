#include "espconn.h"
#include "mem.h"
#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"
#include "user_esp_platform.h"
#include "user_tcpclient.h"

#define NET_DOMAIN "cn.bing.com"
#define pheadbuffer "GET / HTTP/1.1\r\nUser-Agent: curl/7.37.0\r\nHost: %s\r\nAccept: */*\r\n\r\n"

#define packet_size   (2 * 1024)


LOCAL os_timer_t timer_tcp_check;
LOCAL struct espconn user_tcp_conn;
LOCAL struct _esp_tcp user_tcp;
ip_addr_t tcp_server_ip;


/******************************************************************************
 * FunctionName : user_tcp_recv_cb
 * Description  : receive callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
 void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
   //received some data from tcp connection

    os_printf("Received data string: %s \r\n", pusrdata);

}
/******************************************************************************
 * FunctionName : user_tcp_sent_cb
 * Description  : data sent callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
 void ICACHE_FLASH_ATTR
user_tcp_sent_cb(void *arg)
{
   //data sent successfully

    os_printf("Sent callback: data sent successfully.\r\n");
}
/******************************************************************************
 * FunctionName : user_tcp_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
 void ICACHE_FLASH_ATTR
user_tcp_discon_cb(void *arg)
{
   //tcp disconnect successfully

    os_printf("Disconnected from server.\r\n");
}
/******************************************************************************
 * FunctionName : user_esp_platform_sent
 * Description  : Processing the application data and sending it to the host
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
 void ICACHE_FLASH_ATTR
user_send_data(struct espconn *pespconn)
{
    char *pbuf = (char *)os_zalloc(packet_size);

    os_sprintf(pbuf, pheadbuffer, NET_DOMAIN);

   espconn_send(pespconn, pbuf, os_strlen(pbuf));

   os_free(pbuf);

}

/******************************************************************************
 * FunctionName : user_tcp_connect_cb
 * Description  : A new incoming tcp connection has been connected.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
 void ICACHE_FLASH_ATTR
user_tcp_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;

    os_printf("Connected to server...\r\n");

    espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
    espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
    espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

    user_send_data(pespconn);
}

/******************************************************************************
 * FunctionName : user_tcp_recon_cb
 * Description  : reconnect callback, error occured in TCP connection.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_tcp_recon_cb(void *arg, sint8 err)
{
   //error occured , tcp connection broke. user can try to reconnect here.

    os_printf("Reconnect callback called, error code: %d !!! \r\n",err);
}

#ifdef DNS_ENABLE
/******************************************************************************
 * FunctionName : user_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/

void ICACHE_FLASH_ATTR
user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    if (ipaddr == NULL)
   {
        os_printf("user_dns_found NULL \r\n");
        return;
    }

   //dns got ip
    os_printf("user_dns_found %d.%d.%d.%d \r\n",
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if (tcp_server_ip.addr == 0 && ipaddr->addr != 0)
   {
      // dns succeed, create tcp connection
        os_timer_disarm(&timer_tcp_check);
        tcp_server_ip.addr = ipaddr->addr;
        os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4); // remote ip of tcp server which get by dns

        pespconn->proto.tcp->remote_port = 80; // remote port of tcp server

        pespconn->proto.tcp->local_port = espconn_port(); //local port of ESP8266

        espconn_regist_connectcb(pespconn, user_tcp_connect_cb); // register connect callback
        espconn_regist_reconcb(pespconn, user_tcp_recon_cb); // register reconnect callback as error handler

        espconn_connect(pespconn); // tcp connect
    }
}
/******************************************************************************
 * FunctionName : user_esp_platform_dns_check_cb
 * Description  : 1s time callback to check dns found
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/

void ICACHE_FLASH_ATTR
user_dns_check_cb(void *arg)
{
    struct espconn *pespconn = arg;

    espconn_gethostbyname(pespconn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // recall DNS function

    os_timer_arm(&timer_tcp_check, 1000, 0);
}
#endif

/******************************************************************************
 * FunctionName : check_tcp_ip
 * Description  : check whether get ip addr or not
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
check_tcp_ip(void)
{
    struct ip_info ipconfig;

   //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
   {

	  //disarm timer first
	  os_timer_disarm(&timer_tcp_check);

      os_printf("Connected to router and assigned IP!\r\n");

      // Connect to tcp server as NET_DOMAIN
      user_tcp_conn.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
      user_tcp_conn.type = ESPCONN_TCP;
      user_tcp_conn.state = ESPCONN_NONE;

#ifdef DNS_ENABLE
       tcp_server_ip.addr = 0;
       espconn_gethostbyname(&user_tcp_conn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // DNS function

       os_timer_setfn(&timer_tcp_check, (os_timer_func_t *)user_dns_check_cb, &user_tcp_conn);
       os_timer_arm(&timer_tcp_check, 1000, 0);
#else

       const char esp_tcp_server_ip[4] = {192,168,31,41}; // remote IP of TCP server

       os_memcpy(user_tcp_conn.proto.tcp->remote_ip,esp_tcp_server_ip,4);

       user_tcp_conn.proto.tcp->remote_port =8887;  // remote port

       user_tcp_conn.proto.tcp->local_port = espconn_port(); //local port of ESP8266

       espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb); // register connect callback
       espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb); // register reconnect callback as error handler
       espconn_connect(&user_tcp_conn);

#endif
    }
}


/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{

   struct station_config stationConf;

   os_strcpy(stationConf.ssid, "Greein");     		//要连接的 路由器的用户名
   os_strcpy(stationConf.password, "Greein@888"); 	//要连接的路由器的密码

   wifi_station_set_config(&stationConf);			//设置配置参数
   wifi_station_connect();							//station连接

   // Set timer to check whether router allotted an IP
   os_timer_disarm(&timer_tcp_check);
   os_timer_setfn(&timer_tcp_check, (os_timer_func_t *)check_tcp_ip, NULL);
   os_timer_arm(&timer_tcp_check, 100, 1); 			//0 只执行一次，1 重复执行

}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
tcpuser_init(void)
{

   //Set station mode
   wifi_set_opmode(STATION_MODE);

   //ESP8266 connect to router
   user_set_station_config();

}



