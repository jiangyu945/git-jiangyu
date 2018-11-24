#ifndef _USER_SHT10_H_
#define _USER_SHT10_H_


#define SHT10_INVALID_VALUE  	0
#define SHT_CMD_GET_TEMP        0x03
#define SHT_CMD_GET_HUMI		0x05

#define SHT_SDA_IO_MUX 	 		PERIPHS_IO_MUX_GPIO4_U
#define SHT_SDA_IO_NUM 			4
#define SHT_SDA_IO_FUNC 		FUNC_GPIO4
#define SHT_SDA_IO_BIT			BIT4

#define SHT_SCL_IO_MUX			PERIPHS_IO_MUX_GPIO5_U
#define SHT_SCL_IO_NUM 			5
#define SHT_SCL_IO_FUNC 		FUNC_GPIO5
#define SHT_SCL_IO_BIT			BIT5

#define SHTSDA_SET()			gpio_output_set(SHT_SDA_IO_BIT, 0, SHT_SDA_IO_BIT, 0);
#define SHTSDA_CLR()			gpio_output_set(0,SHT_SDA_IO_BIT, SHT_SDA_IO_BIT, 0);

#define SHTSCL_SET()			gpio_output_set(SHT_SCL_IO_BIT, 0, SHT_SCL_IO_BIT, 0);
#define SHTSCL_CLR()			gpio_output_set(0,SHT_SCL_IO_BIT, SHT_SCL_IO_BIT, 0);

#define SHTSDA_READ()			(gpio_input_get()&SHT_SDA_IO_BIT)

#define SHT_DELAY()     		os_delay_us(50)

#define Delayus(t)  			os_delay_us(t)
#define	Delayms(t)				os_delay_us(t*1000)

extern void to_light_sleep(void);

void user_shtx_init(void);
void get_humiture(void);

#endif
