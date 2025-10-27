#include "Inf/Inf_BDR6120.h"


/**
 * @brief 电机初始化BDR引脚
 * 
 */
void Inf_BDR6120_Init(void)
{
    //1.初始化
    //C:\Espressif\frameworks\esp-idf-v5.3.1\examples\peripherals\gpio\generic_gpio VSCODE打开
    //1.1定义GPIO引脚配置信息句柄
    gpio_config_t io_config = {};
    //1.2不开启中断
    io_config.intr_type = GPIO_INTR_DISABLE;
    //1.3配置为输出模式
    io_config.mode = GPIO_MODE_OUTPUT;
    //1.4关闭上下拉
    io_config.pull_up_en = 0;
    io_config.pull_down_en = 0;
    //1.5引脚屏蔽位，即为选择哪一个哪几个引脚   1左移引脚对应的位数
    io_config.pin_bit_mask = ((1 << BDR6120_INA) | (1 << BDR6120_INB));
    //1.6让配置信息生效
    gpio_config(&io_config);

    //2.初始状态 电机停止
    Inf_BDR6120_Brake();
}


/**
 * @brief 电机正转（A1 B0）
 * 
 */
void Inf_BDR6120_ForwardRotaTion(void)
{
    gpio_set_level(BDR6120_INA, 1);
    gpio_set_level(BDR6120_INB, 0);
}


/**
 * @brief 电机反转（A0 B1）
 * 
 */
void Inf_BDR6120_ReceverRotaTion(void)
{
    gpio_set_level(BDR6120_INA, 0);
    gpio_set_level(BDR6120_INB, 1);
}


/**
 * @brief 电机不转,都给1或者0（A1 B1）
 * 
 */
void Inf_BDR6120_Brake(void)
{
    gpio_set_level(BDR6120_INA, 1);
    gpio_set_level(BDR6120_INB, 1);
}


/**
 * @brief 电机开锁 正转1s 停止1s 反转1s 停止
 * 
 */ 
void Inf_BDR6120_OpenLock(void)
{
    Inf_BDR6120_ForwardRotaTion();
    delay_ms(1000);
    Inf_BDR6120_Brake();
    delay_ms(1000);
    Inf_BDR6120_ReceverRotaTion();
    delay_ms(1000);
    Inf_BDR6120_Brake();
    
}