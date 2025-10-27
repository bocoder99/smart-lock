#include "Inf_SC12B.h"

//按键中断的回调函数
void SC12B_Handler(void *args);

void Inf_SC12B_Init(void)
{
    // 1.设置I2C参数
    i2c_config_t config = {};
    config.mode = I2C_MODE_MASTER;
    config.scl_io_num = I2C_MASTER_SCL_IO;
    config.sda_io_num = I2C_MASTER_SDA_IO;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = I2C_MASTER_FREQ_HZ;

    // 2.使配置信息生效 第一个是I2C的端口号，例如两个I2C，I2C0/I2C1
    i2c_param_config(I2C_NUM_0, &config);

    // 3.开启I2C模块
    i2c_driver_install(I2C_NUM_0,
                       config.mode,
                       0, 0, // 在主模式下可以不配置接收和发送缓冲区
                       0);

    //4.中断引脚配置
    //4.1引脚工作信息
    gpio_config_t io_config = {};
    io_config.intr_type = GPIO_INTR_POSEDGE;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_down_en = 1;
    io_config.pull_up_en = 0;
    io_config.pin_bit_mask = 1 << I2C_MASTER_INTR;

    //4.2让配置信息生效
    gpio_config(&io_config);

    //4.3安装ISR服务 默认配置0
    gpio_install_isr_service(0);   

    //4.4将引脚与回调函数绑定
    gpio_isr_handler_add(I2C_MASTER_INTR,SC12B_Handler,(void *)I2C_MASTER_INTR);      
}

uint8_t isTouch = 0;
void SC12B_Handler(void *args)
{
    isTouch = 1;
}


uint8_t Inf_SC12B_ReadReg(uint8_t reg)
{
    uint8_t data;
    // const uint8_t* write_buffer 读取的寄存器的地址
    i2c_master_write_read_device(I2C_NUM_0, SC12B_I2C_ADDR, &reg, 1, &data, 1, 2000);
    return data;
}

/**
 * @brief 获取按键的值
 *
 * @return Touch_Key
 */
Touch_Key Inf_SC12B_ReadKey(void)
{

    // 1.读取08/09寄存器中的数据
    uint8_t data1 = Inf_SC12B_ReadReg(0x08);
    uint8_t data2 = Inf_SC12B_ReadReg(0x09);

    // 2.拼接读取到的两部分数据
    uint16_t key = (data1 << 8) | data2;
    Touch_Key touchkey = KEY_NO;

    // 3.判断是哪一个按键
    switch (key)
    {
    case 0x8000:
        touchkey = KEY_0;
        break;
    case 0x4000:
        touchkey = KEY_1;
        break;
    case 0x2000:
        touchkey = KEY_2;
        break;
    case 0x1000:
        touchkey = KEY_3;
        break;
    case 0x0100:
        touchkey = KEY_4;
        break;
    case 0x0400:
        touchkey = KEY_5;
        break;
    case 0x0200:
        touchkey = KEY_6;
        break;
    case 0x0800:
        touchkey = KEY_7;
        break;
    case 0x0040:
        touchkey = KEY_8;
        break;
    case 0x0020:
        touchkey = KEY_9;
        break;
    case 0x0010:
        touchkey = KEY_SHARP;
        break;
    case 0x0080:
        touchkey = KEY_M;
        break;
    default:
        break;
    }

    // 4.输出
    return touchkey;
}

Touch_Key Inf_SC12B_KeyClick(void)
{
    Touch_Key  key = KEY_NO;
    if(isTouch)
    {
        key = Inf_SC12B_ReadKey();
        isTouch = 0;
    }
    return key;
}
