#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "user_sht10.h"


void ICACHE_FLASH_ATTR
user_shtx_init(void)
{
	PIN_FUNC_SELECT(SHT_SDA_IO_MUX, SHT_SDA_IO_FUNC);
	PIN_FUNC_SELECT(SHT_SCL_IO_MUX, SHT_SCL_IO_FUNC);

    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(SHT_SDA_IO_NUM)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(SHT_SDA_IO_NUM))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << SHT_SDA_IO_NUM));
    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(SHT_SCL_IO_NUM)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(SHT_SCL_IO_NUM))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << SHT_SCL_IO_NUM));

    //将SDA和SCL都拉高以释放总线
    SHTSDA_SET();
    SHTSCL_SET();

    get_humiture();
}

LOCAL int GetSDAbit()
{
	int data = gpio_input_get();

	if ((data&SHT_SDA_IO_BIT) > 0)
		return 1;

    return 0;
}

LOCAL void ResetSHT()
{
	int i;

	SHTSDA_SET();
	SHT_DELAY();
	SHTSCL_CLR();
	SHT_DELAY();

	for(i=0;i<10;i++)
	{
	SHT_DELAY();
		SHTSCL_SET();
	SHT_DELAY();
		SHTSCL_CLR();
	}

	SHTSCL_SET();

	Delayus(20);
}

LOCAL void SHT_START(void)
{
		SHT_DELAY();
	SHTSDA_CLR();    //SDA下降沿，开启信号
		SHT_DELAY();
	SHTSCL_CLR();    //钳住IIC总线，准备发送和接收数据
		SHT_DELAY();
	SHTSCL_SET();
		SHT_DELAY();
	SHTSDA_SET();
		SHT_DELAY();
	SHTSCL_CLR();
		SHT_DELAY();
}


LOCAL int serial_read(unsigned int Code)
{
	int i;
	int temp;

	SHT_START();

	for(i=0;i<8;i++)//code
	{
		if((Code>>(7-i))&0x01) // MSB first
		{
			SHTSDA_SET();
		}
		else
		{
			SHTSDA_CLR();
		}
			SHT_DELAY();
		SHTSCL_SET();
			SHT_DELAY();

		SHTSCL_CLR();
	    	SHT_DELAY();
	}

    SHT_DELAY();
	SHTSCL_SET();
	SHT_DELAY();

	if(GetSDAbit() == 1)     //if no ack
	{
		ResetSHT();

		os_printf("error: cmd sht10 return no ack\n");
		return SHT10_INVALID_VALUE;
	}

	SHTSCL_CLR();
	SHT_DELAY();
	SHTSDA_SET();

	for(i = 0; i < 1000; i++)
	{
		Delayms(1);
		if(GetSDAbit() == 0)  //等待应答
		{
			break;
		}
	}
	if(i >= 1000)
	{
		ResetSHT();

		os_printf("error: time out!\n");
		return SHT10_INVALID_VALUE;
	}

	//MSB
	temp=0;

	for(i=0;i<8;i++)
	{
		temp<<=1;
		SHT_DELAY();
		SHTSCL_SET();
		if(GetSDAbit())
		{
			temp++;
		}

		SHT_DELAY();
		SHTSCL_CLR();
		SHT_DELAY();
	}

	//ack
	SHTSDA_CLR();
		SHT_DELAY();
	SHTSCL_SET();
		SHT_DELAY();
	SHTSCL_CLR();
		SHT_DELAY();
	SHTSDA_SET();

	//LSB
	for(i=0;i<8;i++)
	{
		temp<<=1;
			SHT_DELAY();
		SHTSCL_SET();
		if(GetSDAbit())
		{
			temp++;
		}

			SHT_DELAY();
		SHTSCL_CLR();
			SHT_DELAY();
	}

	//ack
	SHTSDA_CLR();
		SHT_DELAY();
	SHTSCL_SET();
		SHT_DELAY();
	SHTSCL_CLR();
		SHT_DELAY();
	SHTSDA_SET();

	//check sum
	for(i=0;i<8;i++)
	{
			SHT_DELAY();
		SHTSCL_SET();
			SHT_DELAY();
		SHTSCL_CLR();
			SHT_DELAY();
	}
	//ack
	//SHTSET_SDA_OUT();
	SHTSDA_SET();
		SHT_DELAY();
	SHTSCL_SET();
		SHT_DELAY();
	SHTSCL_CLR();
		SHT_DELAY();

	SHTSCL_SET();

	return temp;
}

LOCAL int GetTemp(short *temp)
{
	int SOt;

	SOt=serial_read(SHT_CMD_GET_TEMP);

	if(SOt == SHT10_INVALID_VALUE)
	{
		os_printf("GetTemp:SHT10_INVALID_VALUE\n");
		return -1;
	}

	os_printf("temp:0x%x\n", SOt);

	//Temp=d1+d2*SOt
	//d1=-39.6 d2=0.01
	*temp=(SOt / 10)-396;   //*10避免浮点运算复杂度

	os_printf("temperature:%d.%dC\n",  *temp/10, *temp%10);

	return 	0;
}

LOCAL int GetHumi(unsigned short *humi)
{
	unsigned int SOrh;
	unsigned long RHl,RHt;
	long tmp, diff=0;
	short Tc;

	SOrh=serial_read(SHT_CMD_GET_HUMI);

	if(SOrh==SHT10_INVALID_VALUE)
	{
		os_printf("GetHumi:SHT10_INVALID_VALUE\n");
		return -1;
	}

	/*
	 *	RHliner=c1+c2*SOrh+c3*SOrh^2
	 *  c1=-4  c2=0.0405  c3=-0.0000028
	 *  RHtrue=(Tc-25)*(t1+t2*SOrh)+RHliner
	 *  t1=0.01 t2=0.00008
	 */
	tmp=SOrh;
	RHl = (tmp * 405 / 1000) - (28 * (tmp * tmp)) / 1000000 - 40;  //*10倍，方便换算

	if(GetTemp(&Tc) == 0)      //获得温度进行矫正湿度
	{
		diff = (Tc - 250)*(1000 + 8*tmp)/100000;
		RHt = RHl + diff;
	}
	else
	{
		RHt = RHl;
	}

	if(RHt > 1000)  // 100%
	{
		RHt = 1000;
	}

	*humi = RHl;

	os_printf("RHliner:%ld.%ld%% RHtrue:%ld.%ld%% diff:%ld\n",  RHl/10, RHl%10, RHt/10, RHt%10, diff);

	return 	0;
}


LOCAL os_timer_t timer_sht10_read;
short g_temp,g_humi;

void ICACHE_FLASH_ATTR
get_humiture(void)
{
	os_timer_disarm(&timer_sht10_read);

    if(GetTemp(&g_temp) == -1)   //在GetHumi中会再调用读出temp来做湿度校准
    {
    	g_temp = SHT10_INVALID_VALUE;
    }

    if(GetHumi(&g_humi) == -1)
    {
    	g_humi = SHT10_INVALID_VALUE;
    }

    os_timer_setfn(&timer_sht10_read, (os_timer_func_t *)get_humiture, NULL);
    os_timer_arm(&timer_sht10_read, 2000, 0);
}
