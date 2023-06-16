#include "IIC.h"
#include "bsp_SysTick.h"

static void iic_delay(void)
{
    Delay_us(2); /* 2us����ʱ, ��д�ٶ���250Khz���� */
}

void iic_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    IIC_SCL_GPIO_CLK_ENABLE(); /* SCL����ʱ��ʹ�� */

    gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init_struct.GPIO_Pin = IIC_SCL_GPIO_PIN;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SCL_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio_init_struct.GPIO_Pin = IIC_SDA_GPIO_PIN;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SDA_GPIO_PORT, &gpio_init_struct);

    iic_stop(); /* ֹͣ�����������豸 */
}

void iic_start(void)
{
    IIC_SCL(1);
    IIC_SDA(1);
    iic_delay();
    IIC_SDA(0);
    iic_delay();
    IIC_SCL(0);
    iic_delay();
}

void iic_stop(void)
{
    IIC_SDA(0);
    iic_delay();
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(1);
    iic_delay();
}

void iic_ack(void)
{
    IIC_SDA(0);
    iic_delay();
    IIC_SCL(1);
    iic_delay();
    IIC_SCL(0);
    iic_delay();
    IIC_SDA(1); // ����ͷ�����
    iic_delay();
}

void iic_nack(void)
{
    IIC_SDA(1);
    iic_delay();
    IIC_SCL(1);
    iic_delay();
    IIC_SCL(0);
    iic_delay();
}
/**
 * @brief       �ȴ�Ӧ���źŵ���
 * @param       ��
 * @retval      1������Ӧ��ʧ��
 *              0������Ӧ��ɹ�
 */
uint8_t iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    IIC_SDA(1); /* �����ͷ�SDA��(��ʱ�ⲿ������������SDA��) */
    iic_delay();
    IIC_SCL(1); /* SCL=1, ��ʱ�ӻ����Է���ACK */
    iic_delay();

    while (IIC_READ_SDA)
    {
        waittime++;

        if (waittime > 250)
        {
            iic_stop();
            rack = 1;
            break;
        }
    }

    IIC_SCL(0); /* SCL=0, ����ACK��� */
    iic_delay();
    return rack;
}

void iic_send_byte(uint8_t data)
{
    uint8_t t;

    for (t = 0; t < 8; t++)
    {
        IIC_SDA((data & 0x80) >> 7); // �ȷ������λ
        iic_delay();
        IIC_SCL(1);
        iic_delay();
        IIC_SCL(0); // ��SCLΪ0��ʱ�� ���ݿ���ת���ƽ
        iic_delay();
        data <<= 1;
    }
    IIC_SDA(1); /* �������, �����ͷ�SDA�� */
}

/// @brief
/// @param ack
/// @return 1 ACk 0 NACK
uint8_t iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++)
    {
        receive <<= 1; /* ��λ�����,�������յ�������λҪ���� */
        IIC_SCL(1);
        iic_delay();

        if (IIC_READ_SDA)
        {
            receive++;
        }

        IIC_SCL(0);
        iic_delay();
    }
    if (!ack)
    {
        iic_nack(); /* ����nACK */
    }
    else
    {
        iic_ack(); /* ����ACK */
    }

    return receive;
}

u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    u8 i;
    iic_start();
    iic_send_byte((addr << 1) | 0); // ����������ַ+д����
    if (iic_wait_ack())             // �ȴ�Ӧ��
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg); // д�Ĵ�����ַ
    iic_wait_ack();     // �ȴ�Ӧ��
    for (i = 0; i < len; i++)
    {
        iic_send_byte(buf[i]); // ��������
        if (iic_wait_ack())    // �ȴ�ACK
        {
            iic_stop();
            return 1;
        }
    }
    iic_stop();
    return 0;
}

u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    iic_start();
    iic_send_byte((addr << 1) | 0); // ����������ַ+д����
    if (iic_wait_ack())             // �ȴ�Ӧ��
    {
        iic_stop();
        return 1;
    }
    iic_send_byte(reg); // д�Ĵ�����ַ
    iic_wait_ack();     // �ȴ�Ӧ��
    iic_start();
    iic_send_byte((addr << 1) | 1); // ����������ַ+������
    iic_wait_ack();                 // �ȴ�Ӧ��
    while (len)
    {
        if (len == 1)
            *buf = iic_read_byte(0); // ������,����nACK
        else
            *buf = iic_read_byte(1); // ������,����ACK
        len--;
        buf++;
    }
    iic_stop(); // ����һ��ֹͣ����
    return 0;
}
