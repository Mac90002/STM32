#include "bsp_as608.h"
#include "bsp_usart.h"
#include "bsp_ili9341_lcd.h"
#include "rx_data_queue.h"
#include "bsp_SysTick.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

extern QueueHandle_t semaphore_handle1 ;

static uint16_t PS_Connect(uint32_t *PS_Addr);
static void AS608_SendData(uint8_t data);
static void AS608_PackHead(void);
static void NVIC_Configuration(void);
static void SendLength(uint16_t length);
static void SendFlag(uint8_t flag);
static void Sendcmd(uint8_t cmd);
static void SendCheck(uint16_t check);
static uint16_t ReturnFlag(uint16_t *i);
static void ShowErrMessage(uint16_t ensure);
static void Add_FR(void);

uint32_t AS608_Addr = 0xFFFFFFFF;
uint16_t ID ;

void AS608_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  /*��������GPIO�ڵ�ʱ��*/
  RCC_APB2PeriphClockCmd(AS608_TouchOut_INT_GPIO_CLK, ENABLE);
  AS608_USART_GPIO_APBxClkCmd(AS608_USART_GPIO_CLK, ENABLE);
  /*�򿪴��������ʱ��*/
  AS608_USART_APBxClkCmd(AS608_USART_CLK, ENABLE);

  /* ���� NVIC �ж�*/
  NVIC_Configuration();

  /* TouchOut���õ���GPIO */
  GPIO_InitStructure.GPIO_Pin = AS608_TouchOut_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(AS608_TouchOut_INT_GPIO_PORT, &GPIO_InitStructure);

  /* ѡ��EXTI���ź�Դ */
  GPIO_EXTILineConfig(AS608_TouchOut_INT_EXTI_PORTSOURCE, AS608_TouchOut_INT_EXTI_PINSOURCE);
  EXTI_InitStructure.EXTI_Line = AS608_TouchOut_INT_EXTI_LINE;

  /* EXTIΪ�ж�ģʽ */
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /*��USART Tx��GPIO����Ϊ���츴��ģʽ*/
  GPIO_InitStructure.GPIO_Pin = AS608_USART_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(AS608_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  /*��USART Rx��GPIO����Ϊ��������ģʽ*/
  GPIO_InitStructure.GPIO_Pin = AS608_USART_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(AS608_USART_RX_GPIO_PORT, &GPIO_InitStructure);

  /*���ô��ڵĹ�������*/
  USART_InitStructure.USART_BaudRate = AS608_USART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(AS608_USART, &USART_InitStructure);

  USART_ITConfig(AS608_USART, USART_IT_RXNE, ENABLE);
  USART_ITConfig(AS608_USART, USART_IT_IDLE, ENABLE); // ʹ�ܴ������߿����ж�

  USART_Cmd(AS608_USART, ENABLE);
}

static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Ƕ�������жϿ�������ѡ�� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  /* ����USARTΪ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = AS608_USART_IRQ;
  /* �������ȼ�*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* �����ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* ʹ���ж� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
  /* �����ж�Դ��TouchOut�� */
  NVIC_InitStructure.NVIC_IRQChannel = AS608_TouchOut_INT_EXTI_IRQ;
  /* �������ȼ�*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* ���������ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
}

static void AS608_SendData(uint8_t data)
{
  USART_SendData(AS608_USART, data);
  while (USART_GetFlagStatus(AS608_USART, USART_FLAG_TXE) == RESET)
    ;
}

// ���Ͱ�ͷ��оƬ��ַ
static void AS608_PackHead(void)
{
  // ��ͷ
  AS608_SendData(0xEF);
  AS608_SendData(0x01);

  AS608_SendData(AS608_Addr >> 24);
  AS608_SendData(AS608_Addr >> 16);
  AS608_SendData(AS608_Addr >> 8);
  AS608_SendData(AS608_Addr);
}

// ���Ͱ���ʶ
static void SendFlag(uint8_t flag)
{
  AS608_SendData(flag);
}

// ���Ͱ�����
static void SendLength(uint16_t length)
{
  AS608_SendData(length >> 8);
  AS608_SendData(length);
}

// ����ָ��
static void Sendcmd(uint8_t cmd)
{
  AS608_SendData(cmd);
}

// ����У��
static void SendCheck(uint16_t check)
{
  AS608_SendData(check >> 8);
  AS608_SendData(check);
}

// ����ȷ����
static uint16_t ReturnFlag(uint16_t *i)
{
  QUEUE_DATA_TYPE *rx_data;

  rx_data = cbRead(&rx_queue); /*�ӻ�������ȡ���ݣ����д���*/

  if (rx_data != NULL) /*������зǿ�*/
  {
    /*��ӡ���н��յ�������*/
    QUEUE_DEBUG_ARRAY((uint8_t *)rx_data->head, rx_data->len);

    *i = ((uint16_t)(*(rx_data->head + 9))); /*ȷ����*/

    cbReadFinish(&rx_queue); /*ʹ�������ݱ������cbReadFinish���¶�ָ��*/

    return *i;
  }
  else
  {
    *i = 0xff;

    return *i;
  }
}

// ¼��ͼ��
uint16_t PS_GetImage(void)
{
  uint16_t temp;
  uint16_t sure, p = 0;

  AS608_DELAY_MS(3000); /*��ָ�����붯��Ԥ��ʱ��*/

  AS608_PackHead();
  SendFlag(0x01); /*�������ʶ*/
  SendLength(0x03);
  Sendcmd(0x01); /*¼ָ��ָ��*/
  temp = 0x01 + 0x03 + 0x01;
  SendCheck(temp);

  AS608_DELAY_MS(500); /*�ȴ�ָ��ʶ��ģ�鴦������*/

  sure = ReturnFlag(&p);

  return sure;
}

uint16_t PS_GenChar(uint8_t BufferID)
{
  uint16_t temp;
  uint16_t sure, p = 0;

  AS608_PackHead();
  SendFlag(0x01);
  SendLength(0x04);
  Sendcmd(0x02); /*��������ָ��*/
  AS608_SendData(BufferID);
  temp = 0x01 + 0x04 + 0x02 + BufferID;
  SendCheck(temp);

  AS608_DELAY_MS(600);

  sure = ReturnFlag(&p);

  return sure;
}

/// @brief �� CharBuffer1 �� CharBuffer2 �е������ļ��������������򲿷�ָ�ƿ⡣�����������򷵻�ҳ�롣
/// @param BufferID ������ CharBuffer1��CharBuffer2 �� BufferID �ֱ�Ϊ 1h �� 2h
/// @param StartPage ��ʼҳ
/// @param PageNum   ҳ�� -> �ж���ҳ
/// @param p ҳ��
/// @return ȷ����=00H ��ʾ��������
uint16_t PS_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, uint16_t *p)
{
  uint16_t temp;
  uint16_t ensure;
  QUEUE_DATA_TYPE *rx_data;

  AS608_PackHead();
  SendFlag(0x01);
  SendLength(0x08);
  Sendcmd(0x1b); /*��������ָ�ƿ�ָ��*/
  AS608_SendData(BufferID);
  AS608_SendData(StartPage >> 8);
  AS608_SendData(StartPage);
  AS608_SendData(PageNum >> 8);
  AS608_SendData(PageNum);
  temp = 0x01 + 0x08 + 0x1b + BufferID + (StartPage >> 8) + (uint8_t)StartPage + (PageNum >> 8) + (uint8_t)PageNum;
  SendCheck(temp);

  AS608_DELAY_MS(500);

  rx_data = cbRead(&rx_queue); /*�ӻ�������ȡ���ݣ����д���*/
  if (rx_data != NULL)         /*������зǿ�*/
  {
    /*��ӡ���н��յ�������*/
    QUEUE_DEBUG_ARRAY((uint8_t *)rx_data->head, rx_data->len);

    ensure = ((uint16_t)(*(rx_data->head + 9))); /*ȷ����*/

    /*����ҳ�루��ƥ���ָ��ģ��ID��*/
    *p = ((*(rx_data->head + 10)) << 8) + (*(rx_data->head + 11));

    cbReadFinish(&rx_queue); /*ʹ�������ݱ������cbReadFinish���¶�ָ��*/

    return ensure;
  }
  else
  {
    ensure = 0xff;
    return ensure;
  }
}

// ��ȷ�ȶ� CharBuffer1 �� CharBuffer2 �е�ָ��
uint16_t PS_Match(void)
{
  uint16_t temp;
  uint16_t sure, p = 0;

  AS608_PackHead();
  SendFlag(0x01);
  SendLength(0x03);
  Sendcmd(0x03); /*��ȷ�ȶ�ָ��*/
  temp = 0x01 + 0x03 + 0x03;
  SendCheck(temp);

  AS608_DELAY_MS(500);

  sure = ReturnFlag(&p);

  return sure;
}

// �� CharBuffer1 �� CharBuffer2 �е������ļ��ϲ�����ģ�壬������� CharBuffer1 �� CharBuffer2
uint16_t PS_RegModel(void)
{
  uint16_t temp;
  uint16_t sure, p = 0;

  AS608_PackHead();
  SendFlag(0x01);
  SendLength(0x03);
  Sendcmd(0x05); /*�ϲ�����ָ��*/
  temp = 0x01 + 0x03 + 0x05;
  SendCheck(temp);

  AS608_DELAY_MS(500);

  sure = ReturnFlag(&p);

  return sure;
}

static void ShowErrMessage(uint16_t ensure)
{
  switch (ensure)
  {
  case 0x00:
    printf("OK\r\n");
    break;

  case 0x01:
    printf("���ݰ����մ���\r\n");
    break;

  case 0x02:
    printf("ָ��ģ��û�м�⵽ָ�ƣ�\r\n");
    break;

  case 0x03:
    printf("¼��ָ��ͼ��ʧ��\r\n\r\n");
    break;

  case 0x04:
    printf("ָ��ͼ��̫�ɡ�̫��������������\r\n\r\n");
    break;

  case 0x05:
    printf("ָ��ͼ��̫ʪ��̫��������������\r\n\r\n");
    break;

  case 0x06:
    printf("ָ��ͼ��̫�Ҷ�����������\r\n\r\n");
    break;

  case 0x07:
    printf("ָ��ͼ����������������̫�٣������̫С��������������\r\n");
    break;

  case 0x08:
    printf("ָ�Ʋ�ƥ��\r\n\r\n");
    break;

  case 0x09:
    printf("�Ա�ָ��ʧ�ܣ�ָ�ƿⲻ���ڴ�ָ�ƣ�\r\n\r\n");
    break;

  case 0x0a:
    printf("�����ϲ�ʧ��\r\n");
    break;

  case 0x0b:
    printf("����ָ�ƿ�ʱ��ַ��ų���ָ�ƿⷶΧ\r\n");
    break;

  case 0x10:
    printf("ɾ��ģ��ʧ��\r\n");
    break;

  case 0x11:
    printf("���ָ�ƿ�ʧ��\r\n");
    break;

  case 0x15:
    printf("��������û����Чԭʼͼ��������ͼ��\r\n");
    break;

  case 0x18:
    printf("��д FLASH ����\r\n");
    break;

  case 0x19:
    printf("δ�������\r\n");
    break;

  case 0x1a:
    printf("��Ч�Ĵ�����\r\n");
    break;

  case 0x1b:
    printf("�Ĵ����趨���ݴ���\r\n");
    break;

  case 0x1c:
    printf("���±�ҳ��ָ������\r\n");
    break;

  case 0x1f:
    printf("ָ�ƿ���\r\n");
    break;

  case 0x20:
    printf("��ַ����\r\n");
    break;

  default:
    printf("ģ�鷵��ȷ��������\r\n");
    break;
  }
}
// �� CharBuffer1 �� CharBuffer2 �е�ģ���ļ��浽 PageID ��flash ���ݿ�λ��
// BufferID(��������)��PageID��ָ�ƿ�λ�úţ�
uint16_t PS_StoreChar(uint8_t BufferID, uint16_t PageID)
{
  uint16_t temp;
  uint16_t sure, p = 0;

  AS608_PackHead();
  SendFlag(0x01);
  SendLength(0x06);
  Sendcmd(0x06); /*�洢ģ��ָ��*/
  AS608_SendData(BufferID);
  AS608_SendData(PageID >> 8);
  AS608_SendData(PageID);
  temp = 0x01 + 0x06 + 0x06 + BufferID + (PageID >> 8) + (uint8_t)PageID;
  SendCheck(temp);

  AS608_DELAY_MS(500);

  sure = ReturnFlag(&p);

  return sure;
}

static void Add_FR(void)
{
  uint16_t i, j, sure ;

  i = j = 0;

  while (1)
  {
    switch (j)
    {
    case 0: /*ִ�е�1��*/

      i++;
			ILI9341_Clear(0, 120, 240, 140);
      ILI9341_DispString_EN(20, 180, "Please Press Your Finger");
      sure = PS_GetImage(); /*¼��ͼ��*/
      if (sure == 0x00)
      {
        sure = PS_GenChar(CHAR_BUFFER1); /*��������1*/
        if (sure == 0x00)
        {
          ILI9341_Clear(0, 140, 240, 180);
          ILI9341_DispString_EN(20, 180, "fingerprint input succeed");
          SysTick_Delay_Ms(1500);
          sure = PS_HighSpeedSearch(CHAR_BUFFER1, 0, PS_MAXNUM, &ID);
          if (sure == 0x00)
          {
            ILI9341_Clear(0, 140, 240, 180);
            ILI9341_DispString_EN(20, 180, "fingerprint already exist");
            SysTick_Delay_Ms(3000);
						ILI9341_Clear(0, 120, 240, 180);
            return;
          }
          else
          {
            i = 0;
            j = 1; /*��ת����2��*/
          }
        }
        else
        {
          ShowErrMessage(sure);
        }
      }
      else
      {
        ShowErrMessage(sure);
      }
      break;

    case 1:

      i++;
      ILI9341_Clear(0, 140, 240, 180);
      ILI9341_DispString_EN(20, 180, "Please Press Key1 to input fingerprint again");

      sure = PS_GetImage();
      if (sure == 0x00)
      {
        sure = PS_GenChar(CHAR_BUFFER2); /*��������2*/
        if (sure == 0x00)
        {
          ILI9341_Clear(0, 140, 240, 180);
          ILI9341_DispString_EN(20, 180, "fingerprint input succeed");
          SysTick_Delay_Ms(2000);
          i = 0;
          j = 2; /*��ת����3��*/
        }
        else
        {
          ShowErrMessage(sure);
        }
      }
      else
      {
        ShowErrMessage(sure);
      }
      break;

    case 2:

      ILI9341_Clear(0, 140, 240, 180);
      ILI9341_DispString_EN(20, 180, "Fingerprint Compare...");
      SysTick_Delay_Ms(1000);

      sure = PS_Match(); /*��ȷ�ȶ���öָ������*/
      if (sure == 0x00)
      {
        ILI9341_DispString_EN(20, 200, "Compare Succeed !!!");
        SysTick_Delay_Ms(2000);
        j = 3; /*��ת����4��*/
      }
      else
      {
        ILI9341_DispString_EN(20, 200, "Compare Failed !!!");
        ShowErrMessage(sure);
        i = 0;
        j = 0;
      }

      break;

    case 3:

      ILI9341_Clear(0, 140, 240, 180);
      ILI9341_DispString_EN(20, 180, "The Fingerprint Module Is Being Generated...");

      sure = PS_RegModel(); /*�ϲ�����������ģ�壩*/
      if (sure == 0x00)
      {
        ILI9341_DispString_EN(20, 180, "The Generated Succeed !!!");
        SysTick_Delay_Ms(2000);
        j = 4; /*��ת����5��*/
      }
      else
      {
        j = 0;
        ShowErrMessage(sure);
      }

      break;

    case 4:

      //  do
      //  {
      //    printf ("******���������洢ID����ΧΪ0��239******\r\n");

      //    ID=GET_NUM();
      //  }while(!(ID<PS_MAXNUM));

      sure = PS_StoreChar(CHAR_BUFFER2, 1); /*����ģ��*/
      if (sure == 0x00)
      {
      ILI9341_Clear(0, 140, 240, 180);
      ILI9341_DispString_EN(20, 180, "Fingerprint Entry Successful !!!");

        return;
      }
      else
      {
        j = 0;
        ShowErrMessage(sure);
      }
      break;
    }

    SysTick_Delay_Ms(1000);

    if (i == 4) /*����4��û�а���ָ���˳�*/
    {
      ILI9341_Clear(0, 140, 240, 180);
      ILI9341_DispString_EN(20, 180, "Fingerprint Entry Failed !!!");

      break;
    }
  }
}

void AS608_TASK()
{
  int sure , err ;

		err = xSemaphoreTake(semaphore_handle1,1000);/*��ȡ�ź������ȴ�1000*/
    if(err == pdTRUE){
			err = pdFAIL ;
			Add_FR();
		}
  
    ILI9341_DispString_EN(20, 200, "If your Fingerprints are there, run them directly");
    sure = PS_GetImage(); /*¼��ͼ��*/
    if(sure == 0x00){
      sure = PS_GenChar(CHAR_BUFFER1); /*��������1*/
      if(sure == 0x00){
        sure = PS_HighSpeedSearch(CHAR_BUFFER1, 0, PS_MAXNUM, &ID);
        if(sure == 0x00){
        ILI9341_Clear(0, 120, 240, 200);
        ILI9341_DispString_EN(20, 140, "The Fingerprint is Compare");
        ILI9341_DispString_EN(20, 160, "The Door is Open !!!");
        LED2_TOGGLE;
        SysTick_Delay_Ms(3000);
        LED2_TOGGLE;
        ILI9341_Clear(0, 120, 240, 200);
        ILI9341_DispString_EN(20, 140, "The Door is Close !!!");
				SysTick_Delay_Ms(1000);
				ILI9341_Clear(0, 120, 240, 200);
        }
				else{
					ILI9341_Clear(0, 120, 240, 200);
					ILI9341_DispString_EN(20, 140, "The Fingerprint is Illegal !!!");
					SysTick_Delay_Ms(2000);
					ILI9341_Clear(0, 120, 240, 200);
				}
      }
    }
}

uint16_t PS_Connect(uint32_t *PS_Addr)
{
  QUEUE_DATA_TYPE *rx_data;

  AS608_PackHead();
  AS608_SendData(0X01);
  AS608_SendData(0X00);
  AS608_SendData(0X00);

  AS608_DELAY_MS(1000);

  rx_data = cbRead(&rx_queue); /*�ӻ�������ȡ���ݣ����д���*/
  if (rx_data != NULL)         /*������зǿ�*/
  {
    /*��ӡ���н��յ�������*/
    QUEUE_DEBUG_ARRAY((uint8_t *)rx_data->head, rx_data->len);

    if (/*�ж��ǲ���ģ�鷵�ص�Ӧ���*/
        *(rx_data->head) == 0XEF && *(rx_data->head + 1) == 0X01 && *(rx_data->head + 6) == 0X07)
    {
      printf("ָ��ģ��ĵ�ַΪ:0x%x%x%x%x\r\n", *(rx_data->head + 2),
             *(rx_data->head + 3),
             *(rx_data->head + 4),
             *(rx_data->head + 5));
    }
    cbReadFinish(&rx_queue); /*ʹ�������ݱ������cbReadFinish���¶�ָ��*/

    return 0;
  }

  return 1;
}

/**
 * @brief  �����ָ��ģ���ͨ��
 * @param  ��
 * @retval ��
 */
void AS608_Connect_Test(void)
{
  printf("as608ģ��ʵ��");

  if (PS_Connect(&AS608_Addr)) /*��AS608����ͨ��*/
  {
    ILI9341_DispString_EN(20, 20, "AS608 Failed Connect");
  }
  else
  {
    ILI9341_DispString_EN(20, 20, "AS608 Succeed Connect");
  }
}
