#include "Inf_FPM383.h"

static void Inf_FPM383_Intr_Handler(void *);
uint8_t isHasFinger = 0;
static uint8_t receData[100] = {0};
void Inf_FPM383_Init(void)
{
    // 1.参数列表
    const uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,         // 文档说是2位，但是不行
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // 硬件流控不开启
        .source_clk = UART_SCLK_DEFAULT,       // UART模块开启所用的时钟，不是通信用的时钟
    };

    // 2.安装UART服务，0号UART被串口打印用了，此处用的1； 2048接收数据缓冲区大小，0发送数据缓冲区大小
    // 0队列大小，NULL队列句柄，
    uart_driver_install(UART_NUM_1, 2048, 0, 0, NULL, 0);

    // 3.让配置信息生效
    // UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE这两个参数做做硬件流控用的，硬件流控已经关掉了
    uart_param_config(UART_NUM_1, &uart_config);

    // 4.绑定引脚
    // UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE硬件流控的参数，不需要
    uart_set_pin(UART_NUM_1, FPM_TX_PIN, FPM_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // 5.中断引脚配置
    // 5.1中断引脚相关配置信息
    gpio_config_t io_config = {};
    io_config.intr_type = GPIO_INTR_POSEDGE;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.pin_bit_mask = 1 << FPM_INTR_PIN;

    // 5.2配置信息生效
    gpio_config(&io_config);

    // 5.3采用默认配置
    gpio_install_isr_service(0);

    // 5.4添加中断的回调函数
    gpio_isr_handler_add(FPM_INTR_PIN, Inf_FPM383_Intr_Handler, (void *)FPM_INTR_PIN);

    // 5.5控制中断开启与否
    // gpio_intr_enable(FPM_INTR_PIN);
    gpio_intr_disable(FPM_INTR_PIN);

    // 6.芯片休眠：A低功耗 B进入休眠模式后会把Touch(中断引脚拉低)
    // 手指放到上面会拉高，不进入休眠指纹就会一直高电平，无法检测新的中断
    Inf_FPM383_Sleep();
}

/**
 * @brief
 *
 */
static void Inf_FPM383_Intr_Handler(void *)
{
    esp_rom_printf("中断内\r\n");
    isHasFinger = 1;
    gpio_intr_disable(FPM_INTR_PIN);
}

/**
 * @brief 发送命令数据
 *
 * @param cmd
 * @param len
 * @return Com_Status
 */
Com_Status Inf_FPM383_WriteCmd(uint8_t *cmd, uint8_t len)
{
    int txBytes = uart_write_bytes(UART_NUM_1, cmd, len);
    // txBytes返回值，发送成功了几个
    //  返回值用com_sattus，如果发送的字节数和txBytes相等代表发送成功
    return txBytes == len ? Com_OK : Com_ERROR;

}

/**
 * @brief 接收数据
 *
 * @param *receData  接收到的数据
 * @param len        准备接收的长度
 * @param timeout    等待时间
 * @return Com_Status
 */
Com_Status Inf_FPM383_ReadData(uint8_t len, uint16_t timeout)
{
    memset(receData, 0, sizeof(receData));
    int rxBytes = uart_read_bytes(UART_NUM_1, receData, len, timeout);
    return rxBytes == len ? Com_OK : Com_ERROR;
}

/**
 * @brief 读取FPM唯一序列号
 *
 */
void Inf_FPM383_ReadId(void)
{
    // 1.封装获取唯一序列号指令
    uint8_t cmd[13] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址
            0x01,                   // 包标识
            0x00, 0x04,             // 包长度
            0x34,                   // 指令码
            0x00,                   // 参数
            0x00, 0x39              // 校验和
        };

    // 2.发送指令
    Inf_FPM383_WriteCmd(cmd, 13);

    // 3.接收应答包
    Inf_FPM383_ReadData(44, 2000);

    // 判断是否正常接收,如果正常则打印序列号
    if (receData[9] == 0x00)
    {
        printf("获取到的唯一序列号为:%.32s\r\n", &receData[10]);
    }
    else
    {
        printf("获取序列号失败\r\n");
    }
}

void Inf_FPM383_Sleep(void)
{
    // 1.封装休眠指令
    uint8_t cmd[12] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址
            0x01,                   // 包标识
            0x00, 0x03,             // 包长度
            0x33,                   // 指令码
            0x00, 0x37              // 校验和
        };

    do
    {
        // 2.发送休眠指令
        Inf_FPM383_WriteCmd(cmd, 12);

        // 3.接收应答包
        Inf_FPM383_ReadData(12, 2000);

        printf("开始进入芯片休眠...\r\n");

    } while (receData[9] != 0x00);
    printf("休眠成功!\r\n");

    // 4.打开中断引脚
    gpio_intr_enable(FPM_INTR_PIN);
}

/**
 * @brief 取消自动注册与自动验证
 *
 */
void Inf_FPM383_CancelAutoAction(void)
{
    // 1.取消自动注册与自动验证指令
    uint8_t cmd[12] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址
            0x01,                   // 包标识
            0x00, 0x03,             // 包长度
            0x30,                   // 指令码
            '\0', '\0'              // 校验和
        };

    // 2.计算校验和
    Inf_FPM383_AddCheckSum(cmd, 12);

    do
    {
        // 3.发送指令
        Inf_FPM383_WriteCmd(cmd, 12);
        Inf_FPM383_ReadData(12, 2000);
    } while (receData[9] != 0x00);

    // 4.成功
    esp_rom_printf("取消成功!\r\n");
}

/**
 * @brief 获取最小的可用ID
 *
 * @return uint16_t
 */
uint16_t Inf_FPM383_GetMinId(void)
{
    // 1.获取索引表指令
    uint8_t cmd[13] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址
            0x01,                   // 包标识
            0x00, 0x04,             // 包长度
            0x1f,                   // 指令码
            0x00,                   // 页码,只使用0页就够了
            '\0', '\0'              // 校验和
        };

    // 2.添加校验和
    Inf_FPM383_AddCheckSum(cmd, 13);

    // 3.发送指令
    Inf_FPM383_WriteCmd(cmd, 13);
    Inf_FPM383_ReadData(44, 3000);

    // 4.根据返回结果中的索引信息，计算最小可用的空间号（从0开始）
    for (uint8_t i = 0; i < 32; i++)
    {
        // 提取出单个字节，从低位遍历
        uint8_t index = receData[i + 10];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (index & 0x01)
            {
                // 最低位不可用，继续查看次低位
                index >>= 1;
            }
            else
            {
                return 8 * i + j;
            }
        }
    }
    // 已经存满了,特殊处理
    return 0;
}

/**
 * @brief 一站式注册指纹
 *
 * @param id
 * @return Com_Status
 */
Com_Status Inf_FPM383_AutoEnroll(uint16_t id)
{
    // 1.一站式注册指纹指令
    uint8_t cmd[17] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址，默认，可以通过指令修改
            0x01,                   // 包标识
            0x00, 0x08,             // 包长度
            0x31,                   // 指令码
            '\0', '\0',             // ID号
            0x02,                   // 录入次数 次
            0x00, 0x3B,             // 参数
            '\0', '\0',             // 校验和
        };

    // 2.补充ID号,高字节在前???
    // 3.发送是先发高八位，再发低八位
    cmd[10] = id >> 8;
    cmd[11] = id;

    // 3.添加校验和
    Inf_FPM383_AddCheckSum(cmd,17);

    // 4.取消四次自动注册
    Inf_FPM383_CancelAutoAction();
    Inf_FPM383_CancelAutoAction();
    Inf_FPM383_CancelAutoAction();
    Inf_FPM383_CancelAutoAction();

    // 5.发送指令
    Inf_FPM383_WriteCmd(cmd, 17);

    while (1)
    {
        // 提取关键阶段的返回值结果
        Inf_FPM383_ReadData(14,2000);
        // 只要中间阶段任何一次返回不是00，则返回失败
        if (receData[9] != 0x00)
        {
            return Com_ERROR;
        }
        // 返回的确认码是00，同时参数的结果是0x06,则是保存模板成功，认为注册成功
        else if (receData[10] == 0x06)
        {
            return Com_OK;
        }
    }
    // 返回结果
    return Com_TIMEOUT;
}

/**
 * @brief 计算校验和
 *
 * @param cmd 指令数组
 * @param len 指令长度
 */
void Inf_FPM383_AddCheckSum(uint8_t *cmd, uint8_t len)
{
    // 校验和是从标识至校验和之间所有字节之和，包含包标识，不含包标识
    uint16_t checksum = 0;
    for (uint8_t i = 6; i < len - 2; i++)
    {
        checksum += cmd[i];
    }

    // 将计算完的校验和写入指令集
    cmd[len - 2] = checksum >> 8;
    cmd[len - 1] = checksum;
}

Com_Status Inf_FPM383_CheckFingerPrint(void)
{

    // 1.自动验证指纹指令
    uint8_t cmd[17] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址，默认，可以通过指令修改
            0x01,                   // 包标识
            0x00, 0x08,             // 包长度
            0x32,                   // 指令码
            0x03,                   // 分数等级
            0xFF, 0xFF,             // ID号
            0x00, 0x06,             // 参数
            '\0', '\0',             // 校验和
        };

    //2.添加校验和
    Inf_FPM383_AddCheckSum(cmd,17);

    //3.发送指令
    Inf_FPM383_WriteCmd(cmd,17);

    //4.读取返回值结果
    Inf_FPM383_ReadData(17,3000);

    if(receData[9] == 0x00)
    {
        //获取存储在指纹库中的ID号 大端存储，高位在前（低地址，下标小的地方）???
        uint16_t id = (receData[11] << 8) | receData[12]; 
        esp_rom_printf("验证的指纹库中的ID:%d\r\n",id);
        return Com_OK;
    }
    else
    {
        return Com_ERROR;
    }

}


/**
 * @brief 搜索指定指纹的ID号
 * 
 * @return uint16_t 
 */
int16_t Inf_FPM383_SearchFingerPrint(void)
{

    // 1.自动验证指纹指令
    uint8_t cmd[17] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址，默认，可以通过指令修改
            0x01,                   // 包标识
            0x00, 0x08,             // 包长度
            0x32,                   // 指令码
            0x03,                   // 分数等级
            0xFF, 0xFF,             // ID号
            0x00, 0x06,             // 参数
            '\0', '\0',             // 校验和
        };

    //2.添加校验和
    Inf_FPM383_AddCheckSum(cmd,17);

    //3.发送指令
    Inf_FPM383_WriteCmd(cmd,17);

    //4.读取返回值结果
    Inf_FPM383_ReadData(17,3000);

    if(receData[9] == 0x00)
    {
        //获取存储在指纹库中的ID号 大端存储，高位在前（低地址，下标小的地方）???
        uint16_t id = (receData[11] << 8) | receData[12]; 
        return id;
    }
    else
    {
        return -1;
    }

    // // 1.搜索指纹ID指令
    // uint8_t cmd[17] =
    //     {
    //         0xEF, 0x01,             // 包头
    //         0xFF, 0xFF, 0xFF, 0xFF, // 设备地址，默认，可以通过指令修改
    //         0x01,                   // 包标识
    //         0x00, 0x08,             // 包长度
    //         0x04,                   // 指令码
    //         0x01,                   // 缓冲区号，默认1，就是需要验证的指纹放置的位置
    //         0x00, 0x00,             // 参数，查找的起始页
    //         0xFF, 0xFF,             // 参数，查找的结束页
    //         '\0', '\0',             // 校验和
    //     };

    // //2.添加校验和
    // Inf_FPM383_AddCheckSum(cmd,17);

    // //3.发送指令
    // Inf_FPM383_WriteCmd(cmd,17);

    // //4.读取应答包，获取返回值结果
    // Inf_FPM383_ReadData(16,3000);

    // //5.提取返回结果中的ID号
    // return receData[9] == 0x00 ? (receData[10] << 8) | receData[11] : -1;

    // if(receData[9] == 0x00)
    // {
    //     uint16_t id = (receData[10] << 8) | receData[11];
    // }
    // else
    // {
    //     return -1;
    // }
    
}


/**
 * @brief 删除指纹
 * 
 * @param id 
 * @return Com_Status 
 */
Com_Status Inf_FPM383_DeleteFingerPrint(uint16_t id)
{
    // 1.删除指纹ID指令
    uint8_t cmd[16] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址，默认，可以通过指令修改
            0x01,                   // 包标识
            0x00, 0x07,             // 包长度
            0x0C,                   // 指令码
            '\0','\0',                   // 页码，本质就说指纹ID
            0x00, 0x01,             // 参数，删除个数
            '\0', '\0',             // 校验和
        };

    //2.添加指纹ID???
    cmd[10] = id >>8;
    cmd[11] = id;

    //3.添加校验和
    Inf_FPM383_AddCheckSum(cmd,16);

    //4.发送指令
    Inf_FPM383_WriteCmd(cmd,16);

    //5.获取返回值
    Inf_FPM383_ReadData(12,2000);

    if(receData[9] == 0x00)
    {
        return Com_OK;
    }
    else
    {
        return Com_ERROR;
    }
}

/**
 * @brief 清空指纹库
 * 
 */
void Inf_FPM383_DeleteAllFingerPrint(void)
{
    // 1.清空指纹库指令
    uint8_t cmd[12] =
        {
            0xEF, 0x01,             // 包头
            0xFF, 0xFF, 0xFF, 0xFF, // 设备地址，默认，可以通过指令修改
            0x01,                   // 包标识
            0x00, 0x03,             // 包长度
            0x0D,                   // 指令码
            0x00, 0x11,             // 校验和
        };
    
    //2.发送命令
    Inf_FPM383_WriteCmd(cmd,12);

    //3.获取返回值
    Inf_FPM383_ReadData(12,2000);


}