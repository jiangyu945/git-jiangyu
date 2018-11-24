#include "ets_sys.h"
#include "osapi.h"
#include "eagle_soc.h"
#include "user_interface.h"

#include "driver/uart.h"
#include "user_lux.h"

/*---------------------------------------------------------------------
 功能描述: 起始信号
 参数说明: 无
 函数返回: 无
 ---------------------------------------------------------------------*/
void IIC_Start(void)
{
  IIC_SDA_H;                         //拉高数据线
  IIC_SCL_H;                         //拉高时钟线
  IIC_Delay_us(5);             		 //延时
  IIC_SDA_L;                         //产生下降沿
  IIC_Delay_us(5);					 //延时
  IIC_SCL_L;                         //拉低时钟线
}

/*---------------------------------------------------------------------
 功能描述: 停止信号
 参数说明: 无
 函数返回: 无
 ---------------------------------------------------------------------*/
void IIC_Stop(void)
{
	IIC_SCL_L;
	IIC_SDA_L;
	IIC_Delay_us(5);
	IIC_SCL_H;
	IIC_Delay_us(5);
	IIC_SDA_H;
	IIC_Delay_us(5);
}

/*---------------------------------------------------------------------
 功能描述: 发送应答信号
 参数说明: ack - 应答信号(0:ACK 1:NAK)
 函数返回: 无
 ---------------------------------------------------------------------*/
void IIC_Send_Ack(uint8 ack)
{
  if (ack&0x01)	IIC_SDA_H;			 //写应答信号
  else	IIC_SDA_L;

  IIC_SCL_H;                         //拉高时钟线
  IIC_Delay_us(5);                   //延时
  IIC_SCL_L;                         //拉低时钟线
  IIC_SDA_H;
  IIC_Delay_us(5);                   //延时
}

/*---------------------------------------------------------------------
 功能描述: 接收应答信号
 参数说明: 无
 函数返回: 返回应答信号
 ---------------------------------------------------------------------*/
uint8 IIC_Recv_Ack(void)
{
  uint8 CY = 0x00;
  IIC_SDA_H;

//  IIC_SDA_IN;

  IIC_SCL_H;                         //拉高时钟线
  IIC_Delay_us(5);                   //延时

  CY |= IIC_SDA_DAT;                 //读应答信号

  IIC_Delay_us(5);                   //延时

  IIC_SCL_L;                         //拉低时钟线

//	IIC_SDA_OUT;

  return CY;
}

/*---------------------------------------------------------------------
 功能描述: 向IIC总线发送一个字节数据
 参数说明: dat - 写字节
 函数返回: 返回应答信号
 ---------------------------------------------------------------------*/
uint8 IIC_Send_Byte(uint8 dat)
{
  uint8 i;

  for (i=0; i<8; i++)         		//8位计数器
  {
    if (dat&0x80)	IIC_SDA_H;
    else	IIC_SDA_L;              //送数据口

    IIC_Delay_us(5);             	//延时
    IIC_SCL_H;                		//拉高时钟线
    IIC_Delay_us(5);             	//延时
    IIC_SCL_L;                		//拉低时钟线
    IIC_Delay_us(5);             	//延时
    dat <<= 1;              	    //移出数据的最高位
  }

	return IIC_Recv_Ack();
}

/*---------------------------------------------------------------------
 功能描述: 从IIC总线接收一个字节数据
 参数说明: 无
 函数返回: 接收字节
 ---------------------------------------------------------------------*/
uint8 IIC_Recv_Byte(void)
{
  uint8 i;
  uint8 dat = 0;

//  IIC_SDA_IN;
  IIC_SDA_H;                         //使能内部上拉,准备读取数据,
  for (i=0; i<8; i++)         	     //8位计数器
  {
    IIC_SCL_H;                       //拉高时钟线
    IIC_Delay_us(5);             	 //延时
    dat |= IIC_SDA_DAT;              //读数据
    IIC_SCL_L;                       //拉低时钟线
    IIC_Delay_us(5);             	 //延时

    if (i<7) dat <<= 1;
  }
//  IIC_SDA_OUT;

  return dat;
}

/*---------------------------------------------------------------------
 功能描述: 初始化IIC接口
 参数说明: 无
 函数返回: 无
 ---------------------------------------------------------------------*/
void IIC_Init(void)
{
	PIN_FUNC_SELECT(LUX_SDA_IO_MUX, LUX_SDA_IO_FUNC);
	PIN_FUNC_SELECT(LUX_SCL_IO_MUX, LUX_SCL_IO_FUNC);

	//由于B_LUX_V30A比较特殊，需要设置io口为推挽输出可避免偶尔读取失败，一般iic通讯时设置为开漏输出即可。
	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SDA_IO_NUM)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SDA_IO_NUM))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << LUX_SDA_IO_NUM));
	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SCL_IO_NUM)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SCL_IO_NUM))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << LUX_SCL_IO_NUM));

	PIN_PULLUP_EN(LUX_SDA_IO_MUX); //设置上拉使能
	PIN_PULLUP_EN(LUX_SCL_IO_MUX);

	IIC_SDA_H ;
	IIC_SCL_H ;
}


/*---------------------------------------------------------------------
 功能描述: 光照读取函数
 参数说明: 无
 函数返回: 0：成功，>0:失败
 ---------------------------------------------------------------------*/
uint8 user_get_lux(uint32 *vLux)
{
  uint8 vRval = 0;
  uint8 vBuf[20];                                    	//接收数据缓存区
  uint32 val32 = 0;
  float temp;
  uint8 i = 0;

  //---------------------------读程序
  IIC_Start();                                  		//起始信号
  vRval += IIC_Send_Byte(B_LUX_V30B_SlaveAddress+0);    //发送设备地址+写信号
  vRval += IIC_Send_Byte(0x00);                         //指定操作地址

  IIC_Start();                                 			//起始信号
  vRval += IIC_Send_Byte(B_LUX_V30B_SlaveAddress+1);    //发送设备地址+读信号

  IIC_Delay_ms(10);                            			//延时10ms
  for (i=0; i<=4; i++)                          		//连续读取4个地址数据，存储中BUF
  {
    vBuf[i] = IIC_Recv_Byte();
    if (i == 4)
    {
      IIC_Send_Ack(noACK);                       	    //最后一个数据需要回NOACK
    }
    else
    {
      IIC_Send_Ack(ACK);                         		//回应ACK
    }
  }
  IIC_Stop();                                    		//停止信号

  //---------------------------

  /*
   //---------------------------写程序
  IIC_Start();                                 			//起始信号
  IIC_Send_Byte(B_LUX_V30B_SlaveAddress+0);    			//发送设备地址+写信号
  IIC_Send_Byte(0x10);                                  //指定操作地址
  for (i=0; i<=112; i++)                       			//连续写入
  {
    IIC_Send_Byte(0x30+i);
  }
  IIC_Stop();                                 			//停止信号
  //---------------------------
*/

  val32   = vBuf[3];                  					//存储32位光照值
  val32 <<= 8;
  val32  |= vBuf[2];
  val32 <<= 8;
  val32  |= vBuf[1];
  val32 <<= 8;
  val32  |= vBuf[0];

//  uart0_tx_buffer(vBuf,4);    //观察读取光照值

  temp = (float)val32*1.4;           					//半球透明度矫正值*1.4
  *vLux = (uint32)(temp);

  return vRval;
}

/*---------------------------------------------------------------------
 功能描述: 光照值打印函数
 参数说明: 无
 函数返回: 0：成功，>0:失败
 ---------------------------------------------------------------------*/
uint32 m_Lux = 0;
os_timer_t timer_lux_read;

void lux_read_data(void)
{
	LOCAL uint8 read_lux_cnt = 0;

	//刚初始化运行时第一次会读取失败，
	if(user_get_lux(&m_Lux) == 0)
	{
		read_lux_cnt++;
		os_printf("Lux = %d.%d \n",m_Lux / 1000,m_Lux % 1000);


	}
	else
	{
		os_printf("lux iic fail !");
	}

	/*
	os_timer_disarm(&timer_lux_read);
	os_timer_setfn(&timer_lux_read, (os_timer_func_t *)lux_read_data, NULL);
	os_timer_arm(&timer_lux_read, 2000, 1);
    */

	/*硬件连接gpio16<->rst，深度休眠5s后自动唤醒，gpio16会输出低电平触发复位，复位后从user_init()重新执行。
	                 设置了深度休眠模式后，无法下载程序，需要将器GPIO16连线断开。 */
//	if(read_lux_cnt == 5)
//	{
////		system_deep_sleep(5000000);  //发送一次进入深度休眠模式，5s后唤醒
//		system_deep_sleep(0);  		 //可通过外部任意GPIO拉低RST来唤醒。
//	}
}

/*---------------------------------------------------------------------
 功能描述: 初始化lux
 参数说明: 无
 函数返回: 无
 ---------------------------------------------------------------------*/
void ICACHE_FLASH_ATTR
user_lux_init(void)
{
	LOCAL uint8 read_lux_cnt = 0;

	IIC_Init();   		//iic 初始化

	lux_read_data();	//lux 读取数据
}

