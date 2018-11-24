#ifndef _USER_LUX_H_
#define _USER_LUX_H_


#define B_LUX_V30B_SlaveAddress         0x94                  //B_LUX设备地址

#define LUX_SDA_IO_MUX 	 				PERIPHS_IO_MUX_GPIO4_U
#define LUX_SDA_IO_NUM 					4
#define LUX_SDA_IO_FUNC 				FUNC_GPIO4
#define LUX_SDA_IO_BIT					BIT4

#define LUX_SCL_IO_MUX					PERIPHS_IO_MUX_GPIO5_U
#define LUX_SCL_IO_NUM 					5
#define LUX_SCL_IO_FUNC 				FUNC_GPIO5
#define LUX_SCL_IO_BIT					BIT5

#define IIC_Delay_us(t)  				os_delay_us(t)
#define	IIC_Delay_ms(t)					os_delay_us(t*1000)

/*-----------------------------宏定义---------------------------------*/
//Note:B_LUX_V30比较特殊，IO模拟I2C通讯时，SCL、SDA设置成推挽输出，通过实践设置成开漏输出,容易出现错误数据
#define IIC_SCL_H    					gpio_output_set(LUX_SCL_IO_BIT, 0, LUX_SCL_IO_BIT, 0)
#define IIC_SCL_L    					gpio_output_set(0,LUX_SCL_IO_BIT, LUX_SCL_IO_BIT, 0)
#define IIC_SCL_DAT  					(gpio_input_get()&LUX_SCL_IO_BIT)

#define IIC_SCL_OUT
#define IIC_SCL_IN

///////////////////////////////////////////////////////////////////////////////////////

#define IIC_SDA_H    					gpio_output_set(LUX_SDA_IO_BIT, 0, LUX_SDA_IO_BIT, 0)
#define IIC_SDA_L    					gpio_output_set(0,LUX_SDA_IO_BIT, LUX_SDA_IO_BIT, 0)
#define IIC_SDA_DAT  					(gpio_input_get()&LUX_SDA_IO_BIT)

#define IIC_SDA_OUT
#define IIC_SDA_IN

#define noACK 							1      //结束通讯
#define ACK   							0      //未结束通讯

/*-----------------------------函数声明-------------------------------*/
void IIC_Start(void);                          //起始信号
void IIC_Stop(void);                           //停止信号
void IIC_Send_Ack(uint8 ack);                  //应答ACK

void IIC_Init(void);						   //IIC初始化
uint8 IIC_Recv_Ack(void);                      //读ack
uint8 IIC_Send_Byte(uint8 dat);                //IIC单个字节写
uint8 IIC_Recv_Byte(void);                     //IIC单个字节读

uint8 user_get_lux(uint32 *vLux);
void user_lux_init(void);

#endif
