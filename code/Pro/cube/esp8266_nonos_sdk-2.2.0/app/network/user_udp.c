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

	espconn_regist_recvcb(&s_udp_server, user_udp_recv_cb);		//ע��UDP���ջص�����
	espconn_regist_sentcb(&s_udp_server, udp_server_sent_cb); 	//ע��UDP���ͻص�����
	espconn_create(&s_udp_server);

	os_printf("udp_server_init\r\n");
}

void ICACHE_FLASH_ATTR
check_udp_ip(void)
{
	struct ip_info ipconfig;

	//get ip info of ESP8266 station
	wifi_get_ip_info(STATION_IF, &ipconfig);

    //���״̬��ȷ��֤���Ѿ�����
    if (wifi_station_get_connect_status() == STATION_GOT_IP  && ipconfig.ip.addr != 0)
    {
    	os_printf("Connected to router and assigned IP!\r\n");
        os_timer_disarm(&timer_udp_check);

		#if (USER_UDP_ROLE == 0)
        {
			wifi_set_broadcast_if(STATION_MODE);     //���� ESP8266 ���� UDP�㲥��ʱ���� station �ӿڷ���
			user_udp_espconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));//����ռ�
			user_udp_espconn.type = ESPCONN_UDP;                                //��������ΪUDPЭ��
			user_udp_espconn.proto.udp->local_port = espconn_port();            //���ض˿�
			user_udp_espconn.proto.udp->remote_port = 8888;                     //Ŀ��˿�
			const char udp_remote_ip[4] = {192,168,31,103};                     //Ŀ��IP��ַ���㲥��
			os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);

			espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb);         //���ջص�
			espconn_regist_sentcb(&user_udp_espconn, udp_client_sent_cb);       //���ͻص�
			espconn_create(&user_udp_espconn);            						//���� UDP����

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
user_udp_init() //��ʼ��
{
    wifi_set_opmode(STATION_MODE); 			   		//����ΪSTATIONģʽ

    struct station_config stationConf;
    os_strcpy(stationConf.ssid, "Greein");     		//�ĳ���Ҫ���ӵ� ·�������û���
    os_strcpy(stationConf.password, "Greein@888"); 	//�ĳ���Ҫ���ӵ�·����������

    wifi_station_set_config(&stationConf);     		//����WiFi station�ӿ����ã������浽 flash
    wifi_station_connect();                    		//����·����

    os_timer_disarm(&timer_udp_check);  			//ȡ����ʱ����ʱ
    os_timer_setfn(&timer_udp_check, (os_timer_func_t *) check_udp_ip,NULL);    //���ö�ʱ���ص�����
    os_timer_arm(&timer_udp_check, 500, 1);      	//������ʱ������λ������
}

