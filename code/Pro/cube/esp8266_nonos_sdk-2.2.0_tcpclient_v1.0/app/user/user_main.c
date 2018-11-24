/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_tcpclient.h"
#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"

#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

uint32 priv_param_start_sec;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            priv_param_start_sec = 0x3C;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            priv_param_start_sec = 0x7C;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
            rf_cal_sec = 512 - 5;
            priv_param_start_sec = 0x7C;
            break;
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            priv_param_start_sec = 0xFC;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
            rf_cal_sec = 1024 - 5;
            priv_param_start_sec = 0x7C;
            break;
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            priv_param_start_sec = 0xFC;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            priv_param_start_sec = 0xFC;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            priv_param_start_sec = 0xFC;
            break;
        default:
            rf_cal_sec = 0;
            priv_param_start_sec = 0;
            break;
    }

    return rf_cal_sec;
}



void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

/************************************���¼�¼******************************************
 * wifi���ӱ���·�ɣ�esp8266��Ϊtcp_client����tcp_server,���Ե�����ģ���ʹ�á� ---2018.10.23
 * esp8266��Ϊtcp_server��ͨ���궨��USER_TCP_ROLE����esp8266��ɫ�� ---2018.10.24
 * �������ֹ��ܺ�����gpio_interrupt,adc,sntp ---2018.11.2
 * ����sht10��iic��������  --- 2018.11.3
 * ����light_sleepģʽ���Լ�udpͨѶģʽ������ģʽ���Ѻ���������ӡ�   --- 2018.11.6
 * sth10��lux����������ͬһiic���ߣ��򲻼��ݱ�׼Э�飬������iic�������̡�  --- 2018.11.10
*/


os_timer_t timer_router_connected;

void ICACHE_FLASH_ATTR
fpm_wakeup_cb_func1(void)
{
   wifi_fpm_close();    //�ر�ǿ������ģʽ

   /* ʹ��udp����ֱ�����²����Ϳɻָ�������udp serverͨѶ */
  // wifi_set_opmode(STATION_MODE);
  // wifi_station_connect();

   /* ���ʹ��tcpͨѶ���Ѻ�������³�ʼ���ú����ſ���tcp serverͨѶ */
   tcpuser_init();

   os_printf("\r\n sleep wakeup successed! \r\n");
}

void ICACHE_FLASH_ATTR
user_light_sleep(void)
{
	wifi_station_disconnect();							//�Ͽ�station
	wifi_set_opmode(NULL_MODE);

	wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);             //����IO����
	wifi_fpm_open();                                    //����ǿ������ģʽ

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13); //��ʼ������GPIO13
	gpio_pin_wakeup_enable(13, GPIO_PIN_INTR_LOLEVEL);  //���û�������-�͵�ƽ

	wifi_fpm_set_wakeup_cb(fpm_wakeup_cb_func1);        //ϵͳ���Ѻ�Ļص�����
	wifi_fpm_do_sleep(0xfffffff); 			    		//���ò���Ϊ0xFFFFFFF����ϵͳ��һֱ���ߣ�ֱ�����Ѵ�����
}

/* ���ݹ����������ѡ���Ե���  */
void ICACHE_FLASH_ATTR
function_init(void)
{
	my_sntp_init();                 //��ӡ����ʱ��

	user_shtx_init();  				//sht10��ʼ��,�������ʪ��ֵ
//
	user_lux_init();				//lux��ʼ��������ù���ֵ
//
//	user_fota_init();				//fota��ʼ��

	user_gpio_interrupt_init();		//gpio�жϴ�����ʼ��

//	user_hw_timer_init(1000000);	//Ӳ����ʱ����ʼ��(��ʱʱ�䣺0~1999999΢��),1000 000us=1s

//
	user_adc_init();				//adc��ʼ������ӡadcֵ ��0-1024��
//
	user_light_sleep();				//�����������
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	//os_printf("SDK version:%s\n", system_get_sdk_version());
	//os_printf("hello_ai\n");

	tcpuser_init();

	/*�ȴ�����·������ɺ󣬽����������ܳ�ʼ������Ϊ��Щ��ʼ�����õ����繦��*/
	os_timer_disarm(&timer_router_connected);
	os_timer_setfn(&timer_router_connected, (os_timer_func_t *)function_init, NULL);
	os_timer_arm(&timer_router_connected, 10000, 1);  //��ʱ10s���Ƿ��ظ���0/1
}

