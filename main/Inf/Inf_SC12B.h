#ifndef __INF_SC12B_H__
#define __INF_SC12B_H__

#include "driver/gpio.h"
#include "driver/i2c.h"

//数据引脚
#define I2C_MASTER_SDA_IO GPIO_NUM_2
//时钟引脚
#define I2C_MASTER_SCL_IO GPIO_NUM_1
//中断引脚
#define I2C_MASTER_INTR   GPIO_NUM_0

//设备地址 2036 ===>0X40   2341===>0x42
#define SC12B_I2C_ADDR 0x40

//时钟频率（传输速率）
#define I2C_MASTER_FREQ_HZ  100000  //100KHz

typedef enum
{
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SHARP,
    KEY_M,
    KEY_NO
} Touch_Key;


void Inf_SC12B_Init(void);

Touch_Key Inf_SC12B_ReadKey(void);

Touch_Key Inf_SC12B_KeyClick(void);


#endif /* __INF_SC12B_H__ */
