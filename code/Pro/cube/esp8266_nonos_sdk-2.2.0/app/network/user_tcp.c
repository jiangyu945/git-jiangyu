#include "espconn.h"
#include "mem.h"
#include "ets_sys.h"
#include "osapi.h"

#include "driver/uart.h"
#include "user_interface.h"
#include "user_tcp.h"

#define packet_size   (256)

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
	/* 直接通过串口转发 */
	uart0_tx_buffer(pusrdata,length);
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
   /* tcp disconnect successfully */
    os_printf("Disconnected from server.\r\n");
}
/******************************************************************************
 * FunctionName : user_esp_platform_sent
 * Description  : Processing the application data and sending it to the host
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
 void ICACHE_FLASH_ATTR
user_tcp_client_send_data(char *pusrdata, unsigned short length)
{

   char *pbuf = (char *)os_zalloc(packet_size);   //max data len 2048 byte

   espconn_send(&user_tcp_conn, pusrdata, length);

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
    espconn_send(pespconn,"Come from the client!\r\n",21);

    /* 注册接收回调函数，将tcp接收到的数据通过串口发送  */
    espconn_regist_recvcb(pespconn, user_tcp_recv_cb);

    /* 注册发送回调函数，发送完毕的回调函数  */
    espconn_regist_sentcb(pespconn, user_tcp_sent_cb);

    /* 注册断开连接的回调函数  */
    espconn_regist_disconcb(pespconn, user_tcp_discon_cb);
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

/******************************************************************************
 * FunctionName : tcp_server_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_discon_cb(void *arg)   //断开连接回调函数
{
   //tcp disconnect successfully

    os_printf("tcp disconnect succeed !!! \r\n");
}

void ICACHE_FLASH_ATTR
user_tcp_server_multi_send(char *pusrdata, unsigned short length)
{
   struct espconn *pesp_conn = &user_tcp_conn;

   remot_info *premot = NULL;
   uint8 count = 0;
   sint8 value = ESPCONN_OK;
   if (espconn_get_connection_info(pesp_conn,&premot,0) == ESPCONN_OK){
      for (count = 0; count < pesp_conn->link_cnt; count ++){
         pesp_conn->proto.tcp->remote_port = premot[count].remote_port;

         pesp_conn->proto.tcp->remote_ip[0] = premot[count].remote_ip[0];
         pesp_conn->proto.tcp->remote_ip[1] = premot[count].remote_ip[1];
         pesp_conn->proto.tcp->remote_ip[2] = premot[count].remote_ip[2];
         pesp_conn->proto.tcp->remote_ip[3] = premot[count].remote_ip[3];

         espconn_send(pesp_conn, pusrdata, length);
      }
   }
}

/******************************************************************************
 * FunctionName : tcp_server_listen
 * Description  : TCP server listened a connection successfully
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_listen(void *arg)     //连接监听
{
    struct espconn *pesp_conn = arg;
//    os_printf("tcp_server_listen !!! \r\n");

    espconn_regist_recvcb(pesp_conn, user_tcp_recv_cb);
    espconn_regist_reconcb(pesp_conn, user_tcp_recon_cb);
    espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);

//    espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);
}


/******************************************************************************
 * FunctionName : user_tcpserver_init
 * Description  : parameter initialize as a TCP server
 * Parameters   : port -- server port
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_tcpserver_init(uint32 port)  //TCP server参数初始化
{
	user_tcp_conn.type = ESPCONN_TCP;
	user_tcp_conn.state = ESPCONN_NONE;
	user_tcp_conn.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    user_tcp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&user_tcp_conn, tcp_server_listen);

    sint8 ret = espconn_accept(&user_tcp_conn);

    os_printf("espconn_accept [%d] !!! \r\n", ret);

    espconn_regist_time(&user_tcp_conn,0,0);    //建立长连接，去掉则会在几秒后断开client的连接
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
        os_timer_disarm(&test_timer);
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

    os_timer_arm(&test_timer, 1000, 0);
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

   /* get ip info of ESP8266 station */
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
    {
      os_printf("Connected to router and assigned IP!\r\n");

      /*关闭该定时器*/
      os_timer_disarm(&timer_tcp_check);

      /* 作为tcp_client连接客户端，作为tcp_server打开客户端  */
	  #if (USER_TCP_ROLE == 0)
      {
		  // Connect to tcp server as NET_DOMAIN
		  user_tcp_conn.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
		  user_tcp_conn.type = ESPCONN_TCP;
		  user_tcp_conn.state = ESPCONN_NONE;

		#ifdef DNS_ENABLE
		  {
			   tcp_server_ip.addr = 0;
			   espconn_gethostbyname(&user_tcp_conn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // DNS function

			   os_timer_setfn(&test_timer, (os_timer_func_t *)user_dns_check_cb, &user_tcp_conn);
			   os_timer_arm(&test_timer, 1000, 0);
		  }
		#else
		  {
			   const char esp_tcp_server_ip[4] = {192,168,31,41};   // remote IP of TCP server

			   os_memcpy(user_tcp_conn.proto.tcp->remote_ip,esp_tcp_server_ip,4);

			   user_tcp_conn.proto.tcp->remote_port = 8887;  		 // remote port

			   user_tcp_conn.proto.tcp->local_port = espconn_port(); //local port of ESP8266

			   espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb); // register connect callback
			   espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb); // register reconnect callback as error handler
			   espconn_connect(&user_tcp_conn);  					 //连接tcp server
		  }
		#endif
      }
	  #else
	      {
	    	  // as server config open port
	    	  user_tcpserver_init(TCP_SERVER_PORT);
	      }
	  #endif
	}
}


/******************************************************************************
 * FunctionName : user_tcp_init
 * Description  : config tcp conneted.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_tcp_init(void)
{
   wifi_set_opmode(STATION_MODE); 			   		//设置为station模式

   struct station_config stationConf;
   os_strcpy(stationConf.ssid, "Greein");     		//改成你要连接的 路由器的用户名
   os_strcpy(stationConf.password, "Greein@888"); 	//改成你要连接的路由器的密码

   wifi_station_set_config(&stationConf);			//设置配置参数
   wifi_station_connect();							//station连接

   // Set timer to check whether router allotted an IP
   os_timer_disarm(&timer_tcp_check);
   os_timer_setfn(&timer_tcp_check, (os_timer_func_t *)check_tcp_ip, NULL);
   os_timer_arm(&timer_tcp_check, 100, 1); 			//0 只执行一次，1 重复执行
}




