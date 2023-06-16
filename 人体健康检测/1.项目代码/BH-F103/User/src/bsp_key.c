#include "bsp_key.h"

/**
 * @brief  ���ð����õ���I/O��
 * @param  ��
 * @retval ��
 */
void Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*���������˿ڵ�ʱ��*/
	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK | KEY2_GPIO_CLK, ENABLE);

	// ѡ�񰴼�������
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN;
	// ���ð���������Ϊ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	// ʹ�ýṹ���ʼ������
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);

	// ѡ�񰴼�������
	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN;
	// ���ð���������Ϊ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	// ʹ�ýṹ���ʼ������
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);
}

/*
 * ��������Key_Scan
 * ����  ������Ƿ��а�������
 * ����  ��GPIOx��x ������ A��B��C��D���� E
 *		     GPIO_Pin������ȡ�Ķ˿�λ
 * ���  ��KEY_OFF(û���°���)��KEY_ON�����°�����
 */
uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	/*����Ƿ��а������� */
	if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
	{
		/*�ȴ������ͷ� */
		while (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
			;
		return KEY_ON;
	}
	else
		return KEY_OFF;
}
/*********************************************END OF FILE**********************/
static void NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_Initstruct;

	// ���� NVIC KEY1
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_Initstruct.NVIC_IRQChannel = KEY1_NVIC_IRQN;
	NVIC_Initstruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Initstruct.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_Initstruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_Initstruct);

	NVIC_Initstruct.NVIC_IRQChannel = KEY2_NVIC_IRQN;
	NVIC_Initstruct.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_Initstruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Initstruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_Initstruct);
}

void BSP_KEY_EXTI_Config(void)
{
	GPIO_InitTypeDef GPIO_Initstruct;
	EXTI_InitTypeDef EXIT_Initstruct;
	NVIC_Config();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	// ��ʼ��GPIOA KEY1
	GPIO_Initstruct.GPIO_Pin = KEY1_GPIO_PIN;
	GPIO_Initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_Initstruct);
	// GPIO_ResetBits(KEY1_GPIO_PORT,KEY1_GPIO_PIN);

	// ��ʼ�� EXTI	KEY1 ѡ�� EXTI���ź�Դ
	GPIO_EXTILineConfig(KEY1_EXTI_PortSource, KEY1_EXTI_Pin);
	EXIT_Initstruct.EXTI_Line = KEY1_EXTI_LINE;
	EXIT_Initstruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXIT_Initstruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXIT_Initstruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXIT_Initstruct);

	// ��ʼ��GPIOC  KEY2
	GPIO_Initstruct.GPIO_Pin = KEY2_GPIO_PIN;
	GPIO_Initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_Initstruct);
	// GPIO_ResetBits(KEY2_GPIO_PORT,KEY2_GPIO_PIN);

	// ��ʼ�� EXTI KEY2
	GPIO_EXTILineConfig(KEY2_EXTI_PortSource, KEY2_EXTI_Pin);
	EXIT_Initstruct.EXTI_Line = KEY2_EXTI_LINE;
	EXIT_Initstruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXIT_Initstruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXIT_Initstruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXIT_Initstruct);
}
