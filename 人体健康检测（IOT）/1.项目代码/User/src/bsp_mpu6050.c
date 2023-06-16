#include "bsp_mpu6050.h"
#include "IIC.h"
#include "bsp_usart.h"
#include "inv_mpu.h"

void MPU6050_Init(void)
{
    iic_init();
    int i = 0, j = 0;
    // �ڳ�ʼ��֮ǰҪ��ʱһ��ʱ�䣬��û����ʱ����ϵ�����ϵ����ݿ��ܻ����
    for (i = 0; i < 1000; i++)
    {
        for (j = 0; j < 1000; j++)
        {
        }
    }
    MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x00);   // �������״̬                                             reg 107
    MPU6050_WriteReg(MPU6050_RA_SMPLRT_DIV, 0x07);   // �����ǲ����ʣ�1KHz                                       reg25
    MPU6050_WriteReg(MPU6050_RA_CONFIG, 0x06);       // ��ͨ�˲��������ã���ֹƵ����1K��������5K                  reg26
    MPU6050_WriteReg(MPU6050_RA_ACCEL_CONFIG, 0x00); // ���ü��ٶȴ�����������2Gģʽ�����Լ�                      reg28
    MPU6050_WriteReg(MPU6050_RA_GYRO_CONFIG, 0x18);  // �������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s)     reg27
    MPU6050_WriteReg(0x38, 0x01);
    MPU6050_WriteReg(0x37, 0x80);

    // mpu_dmp_init();
}

/// @brief ���Ĵ�����д����
/// @param reg_add �Ĵ�����ַ
/// @param reg_dat ����
void MPU6050_WriteReg(u8 reg_add, u8 reg_dat)
{
    iic_start();
    iic_send_byte(MPU6050_SLAVE_ADDRESS); // ��ַ+д
    iic_wait_ack();
    iic_send_byte(reg_add);
    iic_wait_ack();
    iic_send_byte(reg_dat);
    iic_wait_ack();
    iic_stop();
}

/// @brief �ӼĴ����ж�����
/// @param reg_add �Ĵ�����ַ
/// @param Read ������
/// @param num ����������
void MPU6050_ReadData(u8 reg_add, unsigned char *Read, u8 num)
{
    uint8_t i;
    iic_start();
    iic_send_byte(MPU6050_SLAVE_ADDRESS);
    iic_wait_ack();
    iic_send_byte(reg_add);
    iic_wait_ack();
    iic_start();
    iic_send_byte(MPU6050_SLAVE_ADDRESS + 1); // ��ַ+��
    iic_wait_ack();
    for (i = 0; i <= (num - 1); i++)
    {
        *Read = iic_read_byte(1);
        Read++;
    }
    iic_stop();
}

uint8_t MPU6050ReadID(void)
{
    unsigned char Re = 0;
    MPU6050_ReadData(MPU6050_RA_WHO_AM_I, &Re, 1); // ��������ַ
    if (Re != 0x68)
    {
        printf("MPU6050 dectected error!\r\n��ⲻ��MPU6050ģ��,����ģ���뿪����Ľ���");
        return 0;
    }
    else
    {
        printf("MPU6050 ID = %#x\r\n", Re);
        return 1;
    }
}

/// @brief �¶Ȳ���ֵ reg65 66 type :read only
/// @param tempData
void MPU6050ReadTemp(short *tempData)
{
    uint8_t buf[2];
    MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H, buf, 2); // ��ȡ�¶�ֵ
    *tempData = (buf[0] << 8) | buf[1];
}

/// @brief �������϶�
/// @param Temperature
void MPU6050_ReturnTemp(float *Temperature)
{
    short temp3;
    u8 buf[2];

    MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H, buf, 2); // ��ȡ�¶�ֵ
    temp3 = (buf[0] << 8) | buf[1];
    *Temperature = ((double)temp3 / 340.0) + 36.53;
}

/// @brief ��ȡ����ֵ �Ǽ��ٶ�
/// @param gyroData
void MPU6050ReadGyro(short *gyroData)
{
    u8 buf[6];
    MPU6050_ReadData(MPU6050_GYRO_OUT, buf, 6);
    gyroData[0] = (buf[0] << 8) | buf[1]; // X�����ֵ
    gyroData[1] = (buf[2] << 8) | buf[3]; // Y�����ֵ
    gyroData[2] = (buf[4] << 8) | buf[5]; // Z�����ֵ
}

void MPU6050ReadAcc(short *accData)
{
    u8 buf[6];
    MPU6050_ReadData(MPU6050_ACC_OUT, buf, 6);
    accData[0] = (buf[0] << 8) | buf[1]; // X�����ֵ
    accData[1] = (buf[2] << 8) | buf[3]; // Y�����ֵ
    accData[2] = (buf[4] << 8) | buf[5]; // Z�����ֵ
}
