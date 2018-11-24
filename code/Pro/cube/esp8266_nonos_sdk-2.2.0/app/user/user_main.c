/*
GPIO_REG_WRITE * ESPRESSIF MIT License
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
#include "mem.h"
#include "upgrade.h"

#include "driver/uart.h"
#include "user_interface.h"
#include "user_gpio.h"
#include "user_tcp.h"
#include "user_udp.h"
#include "user_sntp.h"
#include "user_adc.h"
#include "user_hw_timer.h"
#include "user_sht10.h"
#include "user_lux.h"
#include "user_ota.h"

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


/************************************更新记录******************************************
 * wifi连接本地路由，esp8266作为tcp_client连接tcp_server,可以当串口模块简单使用。 ---2018.10.23
 * esp8266作为tcp_server，通过宏定义USER_TCP_ROLE配置esp8266角色。 ---2018.10.24
 * 修复两个esp8266通讯时数据丢失后半段，定位：串口接收中断中接收数据遇到0xFF误触发TOUT_INT，根据判断条件规避错误。 ---2018.10.25
 * 新增部分功能函数：gpio_interrupt,hw_timer,adc,sntp ---2018.11.2
 * 新增sht10的iic驱动程序。  --- 2018.11.3
 * 新增light_sleep模式，以及udp通讯模式，休眠模式唤醒后可重新连接。   --- 2018.11.6
 * sth10和lux传感器挂载同一iic总线，因不兼容标准协议，不共用iic操作流程。  --- 2018.11.10
 * 搭建本地服务器tomcat，实现本地fota升级固件,需要开启boot，生成user1,user2。   --- 2018.11.14
*/

os_timer_t timer_router_connected;

void ICACHE_FLASH_ATTR
fpm_wakeup_cb_func1(void)
{
   wifi_fpm_close();    //关闭强制休眠模式

   /* 使用udp可以直接如下操作就可恢复连接与udp server通讯 */
  // wifi_set_opmode(STATION_MODE);
  // wifi_station_connect();

   /* 如果使用tcp通讯唤醒后必须重新初始化该函数才可与tcp server通讯 */
   user_tcp_init();

   os_printf("\r\n sleep wakeup successed! \r\n");
}

void ICACHE_FLASH_ATTR
user_light_sleep(void)
{
	wifi_station_disconnect();							//断开station
	wifi_set_opmode(NULL_MODE);

	wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);             //设置IO唤醒
	wifi_fpm_open();                                    //开启强制休眠模式

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13); //初始化配置GPIO13
	gpio_pin_wakeup_enable(13, GPIO_PIN_INTR_LOLEVEL);  //设置唤醒引脚-低电平

	wifi_fpm_set_wakeup_cb(fpm_wakeup_cb_func1);        //系统唤醒后的回调函数
	wifi_fpm_do_sleep(0xfffffff); 			    		//设置参数为0xFFFFFFF，则系统将一直休眠，直到唤醒触发。
}

/* 根据功能需求进行选择性调用  */
void ICACHE_FLASH_ATTR
function_init(void)
{
	my_sntp_init();                 //打印网络时间

//	user_shtx_init();  				//sht10初始化,并获得温湿度值
//
	//user_lux_init();				//lux初始化，并获得光照值
//
//	user_fota_init();				//fota初始化

//	user_gpio_interrupt_init();		//gpio中断触发初始化

//	user_hw_timer_init(1000000);	//硬件定时器初始化(定时时间：0~1999999微妙),1000 000us=1s
//
	//user_sntp_init();				//sntp初始化，打印网络时间
//
//	user_adc_init();				//adc初始化，打印adc值 （0-1024）
//
	//user_light_sleep();				//进入轻度休眠
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
	/* 上电时，系统会自动打印一些系统信息，需要使用748800波特率查看，如果设备对该打印信息敏感，也可进行屏蔽，此处没有设置屏蔽。 */
//  os_printf("SDK version:%s\n", system_get_sdk_version());

	user_uart_init();				//串口初始化，两个串口的波特率都设为115200

	user_tcp_init();				//tcp初始化,可通过配置宏USER_TCP_ROLE开启client/server

//	user_udp_init(); 				//udp初始化,可通过配置宏USER_UDP_ROLE开启client/server

	/*等待连接路由器完成后，进行其他功能初始化，因为有些初始化会用到网络功能*/
	os_timer_disarm(&timer_router_connected);
	os_timer_setfn(&timer_router_connected, (os_timer_func_t *)function_init, NULL);
	os_timer_arm(&timer_router_connected, 10000, 1);  //定时10s，是否重复：0/1
}

