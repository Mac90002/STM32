#ifndef __BSP_HC05_H
#define __BSP_HC05_H

#include "stm32f10x.h"

#define HC05_USART                  USART3

#define BLT_INT_GPIO_PORT    	    GPIOC			              /* GPIO�˿� */
#define BLT_INT_GPIO_CLK 	        RCC_APB2Periph_GPIOC		/* GPIO�˿�ʱ�� */
#define BLT_INT_GPIO_PIN		  		GPIO_Pin_4		          /* ���ӵ�HC05 INT���ŵ�GPIO */

#define BLT_KEY_GPIO_PORT    	    GPIOG			              /* GPIO�˿� */
#define BLT_KEY_GPIO_CLK 	        RCC_APB2Periph_GPIOG		/* GPIO�˿�ʱ�� */
#define BLT_KEY_GPIO_PIN		    	GPIO_Pin_8		          /* ���ӵ�HC05 KEY���ŵ�GPIO */

#define BLT_KEY_HIGHT  		        GPIO_SetBits(BLT_KEY_GPIO_PORT, BLT_KEY_GPIO_PIN);
#define BLT_KEY_LOW  							GPIO_ResetBits(BLT_KEY_GPIO_PORT, BLT_KEY_GPIO_PIN);
#define IS_HC05_CONNECTED() 	    GPIO_ReadInputDataBit(BLT_INT_GPIO_PORT,BLT_INT_GPIO_PIN)

//��������豸����
#define BLTDEV_MAX_NUM 10

                                       
                                       
/*������ַ��������ʽ����NAP��UAP��LAP��*/																			 

typedef  struct 
{
	uint8_t num;		//ɨ�赽�������豸����
		
	char unpraseAddr[BLTDEV_MAX_NUM][50];	//�����豸��ַ���ַ�����ʽ������ɨ��ʱ������ʱʹ��
	
	char name[BLTDEV_MAX_NUM][50];	//�����豸������
	
}BLTDev;

//�����豸�б��� bsp_hc05.c �ļ��ж���
extern  BLTDev bltDevList;


enum
{
  HC05_DEFAULT_TIMEOUT = 200,
  HC05_INQUIRY_DEFAULT_TIMEOUT = 10000,
  HC05_PAIRING_DEFAULT_TIMEOUT = 10000,
  HC05_PASSWORD_MAXLEN = 16,
  HC05_PASSWORD_BUFSIZE = HC05_PASSWORD_MAXLEN + 1,
  HC05_NAME_MAXLEN = 32,
  HC05_NAME_BUFSIZE = HC05_NAME_MAXLEN + 1,
  HC05_ADDRESS_MAXLEN = 14,
  HC05_ADDRESS_BUFSIZE = HC05_ADDRESS_MAXLEN + 1,
};


uint8_t HC05_Init(void);
uint8_t HC05_Send_CMD(char* cmd , uint8_t clean);
void TransData_CtrlLED_Test(void);
#endif /* __BSP_HC05_H */

