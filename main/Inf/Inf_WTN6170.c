#include "Inf_WTN6170.h"

void Inf_WTN6170_Init(void)
{
    // 1.配置GPIO引脚
    gpio_config_t io_config = {};

    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pin_bit_mask = 1 << WTN6170_SDA;

    gpio_config(&io_config);
}

void Inf_WTN6170_SendCmd(uint8_t cmd)
{
    // 1.拉低数据线之后，延迟ms
    WTN6170_SDA_L;
    delay_ms(10);

    // 按照低位先行的方式发送8位数据
    for (uint8_t i = 0; i < 8; i++)
    {
        // 2.1读取低位数据
        if (cmd & 0x01)
        {
            WTN6170_SDA_H;
            delay_us(600);
            WTN6170_SDA_L;
            delay_us(200);
        }
        else
        {
            WTN6170_SDA_H;
            delay_us(200);
            WTN6170_SDA_L;
            delay_us(600);
        }
        cmd >>= 1;
    }

    // 3.拉高数据线，延迟一会
    WTN6170_SDA_H;
    delay_ms(5);

    //初始化随便发一个语音，防止被吞声音
    //sayWaterDrop();
}
