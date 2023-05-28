#include "bsp_hc05.h"
#include "bsp_hc05_usart.h"
#include "bsp_SysTick.h"
#include "bsp_usart.h"
#include "bsp_led.h"
#include "bsp_ili9341_lcd.h"
#include <string.h>

BLTDev bltDevList;
extern ReceiveData DEBUG_USART_ReceiveData;
extern ReceiveData BLT_USART_ReceiveData;

extern uint8_t Clear_Message ;

static void HC05_GPIO_Config(void)
{		
		
    GPIO_InitTypeDef GPIO_InitStructure;

    /*����GPIOʱ��*/
    RCC_APB2PeriphClockCmd( BLT_INT_GPIO_CLK|BLT_KEY_GPIO_CLK, ENABLE); 

    GPIO_InitStructure.GPIO_Pin = BLT_INT_GPIO_PIN;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(BLT_INT_GPIO_PORT, &GPIO_InitStructure);	

    
    GPIO_InitStructure.GPIO_Pin = BLT_KEY_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(BLT_KEY_GPIO_PORT, &GPIO_InitStructure);	
}

/// @brief ����AT���� ֻ�����ھ���OKӦ�������
/// @param cmd ���� ��Ҫ��׺\r\n
/// @param clean �Ƿ�������ջ����� 1 ��� 0 �����
/// @return 0 AT���ͳɹ� 1 ʧ��
uint8_t HC05_Send_CMD(char* cmd , uint8_t clean){
    uint8_t     retry=3; // ����3��
    uint16_t    i = 0 ;
    uint16_t    len;
    char *      redata;

    while(retry --){
        BLT_KEY_HIGHT; // ����KEY���� 
        Usart_SendString(HC05_USART,(uint8_t *)cmd);
        Delay_ms(10);
        i = 500 ;
        do{
            redata = get_rebuff(&len);
            if(len>0)
                {
                    if(strstr(redata,"OK"))				
                    {
                        printf("send CMD: %s",cmd); //��ӡ���͵�����ָ��ͷ�����Ϣ

                        printf("recv back: %s",redata);
                        
                        if(clean==1)
                            clean_rebuff();
                        return 0; //ATָ��ɹ�
                    }
                }

               Delay_ms(10); 
                
          }while(i--);
          printf("send CMD: %s",cmd); //��ӡ���͵�����ָ��ͷ�����Ϣ
          printf("recv back: %s",redata);
          printf("HC05 send CMD fail %d times", retry); //��ʾʧ������
    }

    printf("HC05 send CMD fail ");
		
	if(clean==1)
		clean_rebuff();

	return 1; //ATָ��ʧ�� 
}


uint8_t HC05_Init(){
    
    HC05_USART_Init();
    HC05_GPIO_Config();
    return HC05_Send_CMD("AT\r\n",1);

}

void TransData_CtrlLED_Test(void)
{
  /* �����������ڽ��յ����������� */
  if(BLT_USART_ReceiveData.receive_data_flag == 1)
  {
    BLT_USART_ReceiveData.uart_buff[BLT_USART_ReceiveData.datanum] = 0;
    //����������Լ�������Ҫ���յ��ַ���Ȼ����
    //������յ��ֻ����������� ��1234���ͻ�Ѱ�������ĺ��ȡ��һ��
    if(strstr((char *)BLT_USART_ReceiveData.uart_buff,"1234"))
    {
            
			LED2_TOGGLE;
			Clear_Message = 0 ;
			ILI9341_Clear(0,0,240,320);
			ILI9341_DispString_EN(20,40,"your password is right !!!");
      ILI9341_DispString_EN(20,60,"The door is already open !!!");
      SysTick_Delay_Ms(4000);
    }
		
		if(!strstr((char *)BLT_USART_ReceiveData.uart_buff,"1234"))
    {
			LED2_OFF;
			ILI9341_Clear(0,0,240,320);
			ILI9341_DispString_EN(20,40,"password is failed !!!");
			SysTick_Delay_Ms(4000);
    }
    
    //����������ʾ���յ�������
    Usart_SendString( DEBUG_USARTx, "\r\nrecv HC-05 data:\r\n" );
    Usart_SendString( DEBUG_USARTx, BLT_USART_ReceiveData.uart_buff );
    Usart_SendString( DEBUG_USARTx, "\r\n" );
    
    //���������������ݻ���
    BLT_USART_ReceiveData.receive_data_flag = 0;		//�������ݱ�־����
    BLT_USART_ReceiveData.datanum = 0;  
  }
}



