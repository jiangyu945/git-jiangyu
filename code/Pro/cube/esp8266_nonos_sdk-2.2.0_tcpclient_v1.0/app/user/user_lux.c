#include "ets_sys.h"
#include "osapi.h"
#include "eagle_soc.h"
#include "user_interface.h"

#include "driver/uart.h"
#include "user_lux.h"

/*---------------------------------------------------------------------
 ��������: ��ʼ�ź�
 ����˵��: ��
 ��������: ��
 ---------------------------------------------------------------------*/
void IIC_Start(void)
{
  IIC_SDA_H;                         //����������
  IIC_SCL_H;                         //����ʱ����
  IIC_Delay_us(5);             		 //��ʱ
  IIC_SDA_L;                         //�����½���
  IIC_Delay_us(5);					 //��ʱ
  IIC_SCL_L;                         //����ʱ����
}

/*---------------------------------------------------------------------
 ��������: ֹͣ�ź�
 ����˵��: ��
 ��������: ��
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
 ��������: ����Ӧ���ź�
 ����˵��: ack - Ӧ���ź�(0:ACK 1:NAK)
 ��������: ��
 ---------------------------------------------------------------------*/
void IIC_Send_Ack(uint8 ack)
{
  if (ack&0x01)	IIC_SDA_H;			 //дӦ���ź�
  else	IIC_SDA_L;

  IIC_SCL_H;                         //����ʱ����
  IIC_Delay_us(5);                   //��ʱ
  IIC_SCL_L;                         //����ʱ����
  IIC_SDA_H;
  IIC_Delay_us(5);                   //��ʱ
}

/*---------------------------------------------------------------------
 ��������: ����Ӧ���ź�
 ����˵��: ��
 ��������: ����Ӧ���ź�
 ---------------------------------------------------------------------*/
uint8 IIC_Recv_Ack(void)
{
  uint8 CY = 0x00;
  IIC_SDA_H;

//  IIC_SDA_IN;

  IIC_SCL_H;                         //����ʱ����
  IIC_Delay_us(5);                   //��ʱ

  CY |= IIC_SDA_DAT;                 //��Ӧ���ź�

  IIC_Delay_us(5);                   //��ʱ

  IIC_SCL_L;                         //����ʱ����

//	IIC_SDA_OUT;

  return CY;
}

/*---------------------------------------------------------------------
 ��������: ��IIC���߷���һ���ֽ�����
 ����˵��: dat - д�ֽ�
 ��������: ����Ӧ���ź�
 ---------------------------------------------------------------------*/
uint8 IIC_Send_Byte(uint8 dat)
{
  uint8 i;

  for (i=0; i<8; i++)         		//8λ������
  {
    if (dat&0x80)	IIC_SDA_H;
    else	IIC_SDA_L;              //�����ݿ�

    IIC_Delay_us(5);             	//��ʱ
    IIC_SCL_H;                		//����ʱ����
    IIC_Delay_us(5);             	//��ʱ
    IIC_SCL_L;                		//����ʱ����
    IIC_Delay_us(5);             	//��ʱ
    dat <<= 1;              	    //�Ƴ����ݵ����λ
  }

	return IIC_Recv_Ack();
}

/*---------------------------------------------------------------------
 ��������: ��IIC���߽���һ���ֽ�����
 ����˵��: ��
 ��������: �����ֽ�
 ---------------------------------------------------------------------*/
uint8 IIC_Recv_Byte(void)
{
  uint8 i;
  uint8 dat = 0;

//  IIC_SDA_IN;
  IIC_SDA_H;                         //ʹ���ڲ�����,׼����ȡ����,
  for (i=0; i<8; i++)         	     //8λ������
  {
    IIC_SCL_H;                       //����ʱ����
    IIC_Delay_us(5);             	 //��ʱ
    dat |= IIC_SDA_DAT;              //������
    IIC_SCL_L;                       //����ʱ����
    IIC_Delay_us(5);             	 //��ʱ

    if (i<7) dat <<= 1;
  }
//  IIC_SDA_OUT;

  return dat;
}

/*---------------------------------------------------------------------
 ��������: ��ʼ��IIC�ӿ�
 ����˵��: ��
 ��������: ��
 ---------------------------------------------------------------------*/
void IIC_Init(void)
{
	PIN_FUNC_SELECT(LUX_SDA_IO_MUX, LUX_SDA_IO_FUNC);
	PIN_FUNC_SELECT(LUX_SCL_IO_MUX, LUX_SCL_IO_FUNC);

	//����B_LUX_V30A�Ƚ����⣬��Ҫ����io��Ϊ��������ɱ���ż����ȡʧ�ܣ�һ��iicͨѶʱ����Ϊ��©������ɡ�
	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SDA_IO_NUM)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SDA_IO_NUM))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << LUX_SDA_IO_NUM));
	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SCL_IO_NUM)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(LUX_SCL_IO_NUM))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << LUX_SCL_IO_NUM));

	PIN_PULLUP_EN(LUX_SDA_IO_MUX); //��������ʹ��
	PIN_PULLUP_EN(LUX_SCL_IO_MUX);

	IIC_SDA_H ;
	IIC_SCL_H ;
}


/*---------------------------------------------------------------------
 ��������: ���ն�ȡ����
 ����˵��: ��
 ��������: 0���ɹ���>0:ʧ��
 ---------------------------------------------------------------------*/
uint8 user_get_lux(uint32 *vLux)
{
  uint8 vRval = 0;
  uint8 vBuf[20];                                    	//�������ݻ�����
  uint32 val32 = 0;
  float temp;
  uint8 i = 0;

  //---------------------------������
  IIC_Start();                                  		//��ʼ�ź�
  vRval += IIC_Send_Byte(B_LUX_V30B_SlaveAddress+0);    //�����豸��ַ+д�ź�
  vRval += IIC_Send_Byte(0x00);                         //ָ��������ַ

  IIC_Start();                                 			//��ʼ�ź�
  vRval += IIC_Send_Byte(B_LUX_V30B_SlaveAddress+1);    //�����豸��ַ+���ź�

  IIC_Delay_ms(10);                            			//��ʱ10ms
  for (i=0; i<=4; i++)                          		//������ȡ4����ַ���ݣ��洢��BUF
  {
    vBuf[i] = IIC_Recv_Byte();
    if (i == 4)
    {
      IIC_Send_Ack(noACK);                       	    //���һ��������Ҫ��NOACK
    }
    else
    {
      IIC_Send_Ack(ACK);                         		//��ӦACK
    }
  }
  IIC_Stop();                                    		//ֹͣ�ź�

  //---------------------------

  /*
   //---------------------------д����
  IIC_Start();                                 			//��ʼ�ź�
  IIC_Send_Byte(B_LUX_V30B_SlaveAddress+0);    			//�����豸��ַ+д�ź�
  IIC_Send_Byte(0x10);                                  //ָ��������ַ
  for (i=0; i<=112; i++)                       			//����д��
  {
    IIC_Send_Byte(0x30+i);
  }
  IIC_Stop();                                 			//ֹͣ�ź�
  //---------------------------
*/

  val32   = vBuf[3];                  					//�洢32λ����ֵ
  val32 <<= 8;
  val32  |= vBuf[2];
  val32 <<= 8;
  val32  |= vBuf[1];
  val32 <<= 8;
  val32  |= vBuf[0];

//  uart0_tx_buffer(vBuf,4);    //�۲��ȡ����ֵ

  temp = (float)val32*1.4;           					//����͸���Ƚ���ֵ*1.4
  *vLux = (uint32)(temp);

  return vRval;
}

/*---------------------------------------------------------------------
 ��������: ����ֵ��ӡ����
 ����˵��: ��
 ��������: 0���ɹ���>0:ʧ��
 ---------------------------------------------------------------------*/
uint32 m_Lux = 0;
os_timer_t timer_lux_read;

void lux_read_data(void)
{
	LOCAL uint8 read_lux_cnt = 0;

	//�ճ�ʼ������ʱ��һ�λ��ȡʧ�ܣ�
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

	/*Ӳ������gpio16<->rst���������5s���Զ����ѣ�gpio16������͵�ƽ������λ����λ���user_init()����ִ�С�
	                 �������������ģʽ���޷����س�����Ҫ����GPIO16���߶Ͽ��� */
//	if(read_lux_cnt == 5)
//	{
////		system_deep_sleep(5000000);  //����һ�ν����������ģʽ��5s����
//		system_deep_sleep(0);  		 //��ͨ���ⲿ����GPIO����RST�����ѡ�
//	}
}

/*---------------------------------------------------------------------
 ��������: ��ʼ��lux
 ����˵��: ��
 ��������: ��
 ---------------------------------------------------------------------*/
void ICACHE_FLASH_ATTR
user_lux_init(void)
{
	LOCAL uint8 read_lux_cnt = 0;

	IIC_Init();   		//iic ��ʼ��

	lux_read_data();	//lux ��ȡ����
}

