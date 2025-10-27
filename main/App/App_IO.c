#include "App_IO.h"

Pwd_Op_Status pwdopstatus = CHECK;
extern TaskHandle_t fingerScanHandle;
extern uint8_t isHasFinger;
extern TaskHandle_t otaHandle;

// 按#号之后处理逻辑
void App_IO_InputHander();

/**
 * @brief 按键模块整体初始化
 *
 */
void App_IO_Init(void)
{
    // 1.NVS
    Dri_NVS_Init();
    // 2.电机
    Inf_BDR6120_Init();
    // 3.语音
    Inf_WTN6170_Init();
    //第一次上来就指纹开锁，指纹验证成功的语音会被吞掉，所以加的它
    sayWaterDrop();
    // 4.按键
    Inf_SC12B_Init();
    // 5.灯光
    Inf_WS2812_Init();
    // 6.指纹
    Inf_FPM383_Init();
}

/**
 * @description: 按键扫描
 *
 *    密码输入和设定 状态机:  共分为3个状态
 *      0:自由状态:       默认状态. 在此状态下, 如果检测到有任何按键, 则进入 1:密码输入阶段
 *
 *      1:密码输入阶段
 *                      保存密码
 *      2:输入完成阶段
 *                      对输入密码根据协议进行各种处理
 *
 * @return {*}
 */
Input_Status inputStatus = FREE;
uint8_t password[100] = {0};
uint8_t pwdlen = 0;
void App_IO_KeyScan(void)
{
    // 定义没有按键时间
    static uint16_t noKeyTime = 0;

    // 读取按键
    Touch_Key key = Inf_SC12B_KeyClick();

    if (key == KEY_NO)
    {
        noKeyTime++;
        if (noKeyTime >= 100)
        {
            inputStatus = FREE;
            // 防止溢出
            noKeyTime = 100;
            // 关灯
            Inf_WS2812_LightAllLeds(black);
            // 清除前置所有的输入
            memset(password, 0, 100);
            pwdlen = 0;
        }
        return;
    }
    else
    {
        printf("key=%d\r\n", key);
        // 按键按下，清0计时变量
        noKeyTime = 0;

        switch (inputStatus)
        {
        // 从空闲状态读取到有按键按下，则唤醒，进入输入状态，点亮背景灯
        case FREE:
            Inf_WS2812_LightAllLeds(white);
            inputStatus = INPUT;
            break;

        case INPUT:
            // 无论哪种按键，统一处理逻辑，亮按键灯，水滴声
            Inf_WS2812_LightAllLeds(black);
            delay_ms(10);
            Inf_WS2812_LightKeyLed((uint8_t)key, blue);
            sayWaterDrop();
            delay_ms(500);
            // 根据具体按键，做不同业务逻辑处理
            if (key == KEY_M)
            {
                printf("按键M键,非法输入,清除前置所有数据\r\n");
                inputStatus = FREE;
                sayIllegalOperation();
                delay_ms(50); // 清除前置所有的输入
                memset(password, 0, 100);
                pwdlen = 0;
            }
            else if (key == KEY_SHARP)
            {
                // 按下#号键，需要根据前置输入处理
                inputStatus = DONE;

                // 调用逻辑处理函数
                App_IO_InputHander();

                // 恢复状态
                inputStatus = INPUT;
                // 清除前置所有的输入
                memset(password, 0, 100);
                pwdlen = 0;
            }
            else
            {
                // 数字键被按下,保存到临时存储
                // NVS里面存储的都是字符串，所以数字转字符
                password[pwdlen++] = key + 48;
            }

            break;

        default:
            break;
        }
    }
}

/*
键盘输入协议:
    1. 所有输入都是以 # 结束
    2. 输入M位非法输入, 以前所有输入作废
    3. 协议规则
            01#  新增密码
            02#  删除密码
                ...

            10#  新增指纹
            11#  删除指纹
                ...

            20#  OTA更新
                ...
    4. 数字超过2位的认为是在输入密码开门
 */
void App_IO_InputHander()
{
    // 如果输入的数组小于2
    if (pwdlen < 2)
    {
        printf("输入数字小于2为非法操作!\r\n");
        sayIllegalOperation();
    }
    else if (pwdlen == 2)
    {
        // 输入的为操作指令
        if (password[0] == '0' && password[1] == '1')
        {
            delay_ms(1000);
            // 添加用户密码
            pwdopstatus = ADD;
            sayAddUser();
            delay_ms(1500);
            sayPassword();
            delay_ms(50);
        }
        else if (password[0] == '0' && password[1] == '2')
        {
            // 删除用户密码
            pwdopstatus = DELETE;
            sayDelUser();
            delay_ms(1500);
            sayPassword();
            delay_ms(50);
        }
        else if (password[0] == '1' && password[1] == '1') // 添加指纹指令
        {
            // 任务句柄，值，直接覆盖写入通知值
            xTaskNotify(fingerScanHandle, (uint32_t)'1', eSetValueWithOverwrite);
        }
        else if (password[0] == '1' && password[1] == '2') // 删除指纹指令
        {
            xTaskNotify(fingerScanHandle, (uint32_t)'2', eSetValueWithOverwrite);
        }
        else if (password[0] == '1' && password[1] == '3') // 删除全部指纹
        {
            Inf_FPM383_DeleteAllFingerPrint();
            esp_rom_printf("全部清除完成\n");
        }
        else if (password[0] == '2' && password[1] == '1') // OTA升级指令
        {
            xTaskNotify(otaHandle, (uint32_t)'4', eSetValueWithOverwrite);
           
        }
        else
        {
            printf("输入指令不存在!\r\n");
            sayIllegalOperation();
        }
    }
    else
    {
        if (pwdlen < 5 || pwdlen > 10)
        {
            printf("密码长度不规范!\r\n");
            sayIllegalOperation();
            delay_ms(50);
        }
        else
        {
            switch (pwdopstatus)
            {
            case ADD:
                App_IO_AddPwd(password);
                pwdopstatus = CHECK;
                break;
            case DELETE:
                App_IO_DelPwd(password);
                pwdopstatus = CHECK;
                break;
            case CHECK:
                App_IO_CheckPwd(password);
                break;
            default:
                break;
            }
        }
    }
}

/**
 * @brief 添加密码
 *
 */
void App_IO_AddPwd(uint8_t password[])
{
    // 限定密码存储上限为100
    uint8_t pwdCount = 0;

    // 读取FLASH中存储的密码个数
    Dri_NVS_ReadU8(PWD_CT, &pwdCount);
    if (pwdCount >= 100)
    {
        printf("密码上限!\r\n");
        sayPasswordAddFail();
    }
    else
    {
        if (Dri_NVS_IsPwdExists((char *)password) == ESP_OK)
        {
            sayPasswordAddFail();
            return;
        }
        // 存储密码
        esp_err_t err = Dri_NVS_WriteU8((char *)password, 0);
        if (err == ESP_OK)
        {
            sayPasswordAddSucc();
            delay_ms(2000);

            // 将密码个数+1并存储进flash
            pwdCount++;
            Dri_NVS_WriteU8(PWD_CT, pwdCount);

            printf("密码个数:%d\r\n", pwdCount);
        }
        else
        {
            sayPasswordAddFail();
            delay_ms(2000);
        }
    }
}
void App_IO_DelPwd(uint8_t password[])
{
    esp_err_t err = Dri_NVS_DeletePwd((char *)password);
    if (err == ESP_OK)
    {
        sayDelSucc();
        // 将密码个数-1
        uint8_t pwdCt = 0;
        Dri_NVS_ReadU8(PWD_CT, &pwdCt);
        Dri_NVS_WriteU8(PWD_CT, pwdCt - 1);
    }
    else
    {
        sayDelFail();
    }
}

void App_IO_CheckPwd(uint8_t password[])
{
    esp_err_t err = Dri_NVS_IsPwdExists((char *)password);
    if (err == ESP_OK)
    {
        // 验证成功，执行开锁
        sayPasswordVerifySucc();
        delay_ms(2000);
        Inf_BDR6120_OpenLock();
        sayDoorOpen();
        delay_ms(200);
    }
    else
    {
        // 验证失败重试
        sayPasswordVerifyFail();
        delay_ms(2000);
        sayRetry();
    }
}

/**
 * @brief 指纹扫描任务调用的函数
 * 1.录入指纹（由按键任务通知）
 */
void App_IO_FingerScan(void)
{
    uint32_t action = 0;
    // xTaskNotifyWait(进入等待清空位，退出时清空位，&接收buffer，等待次数)
    //进入退出全清空
    xTaskNotifyWait(0xFFFFFFFF, 0xFFFFFFFF, &action, 0);

    if (action != 0)
    {
        //关闭中断
        gpio_intr_disable(FPM_INTR_PIN);

        // 注册指纹
        if (action == '1')
        {
            sayAddUserFingerprint();
            delay_ms(2000);
            sayPlaceFinger();
            delay_ms(2000);

            // 获取最小可用的ID
            uint16_t id = Inf_FPM383_GetMinId();

            esp_rom_printf("添加的指纹库中的ID:%d\r\n", id);

            Com_Status comstatus = Inf_FPM383_AutoEnroll(id);

            if (comstatus == Com_OK)
            {
                sayFingerprintAddSucc();
                delay_ms(2000);
            }
            else
            {
                sayFingerprintAddFail();
                delay_ms(2000);
            }

            // 进入休眠
            Inf_FPM383_Sleep();
            // 注册已经删除指纹后，芯片会出现问题，所以重启问题
            esp_restart();
        }
        else if (action == '2')
        {
            sayDelUserFingerprint();
            delay_ms(2000);
            sayPlaceFinger();
            delay_ms(2000);

            // 获取按下的手指，在指纹库中的ID
            int16_t id = Inf_FPM383_SearchFingerPrint();
            esp_rom_printf("删除的指纹库中的ID:%d\r\n", id);

            if (id == -1)
            {
                sayDelFail();
            }
            else 
            {
                Com_Status comstatus = Inf_FPM383_DeleteFingerPrint(id);

                if (comstatus == Com_OK)
                {
                    sayDelSucc();
                    delay_ms(2000);
                }
                else
                {
                    sayDelFail();
                    delay_ms(2000);
                }
            }

            // 进入休眠
            Inf_FPM383_Sleep();
            esp_restart();
        }
       
    }
    else
    {
        // 验证指纹
        if (isHasFinger)
        {
            isHasFinger = 0;
            Com_Status comstatus = Inf_FPM383_CheckFingerPrint();
            if (comstatus == Com_OK)
            {
                //sayAlarm();
                //delay_ms(2000);
                sayFingerprintVerifySucc();
                delay_ms(2000);
                Inf_BDR6120_OpenLock();
                sayDoorOpen();
            }
            else
            {
                sayFingerprintVerifyFail();
                delay_ms(2000);
            }
            Inf_FPM383_Sleep();
        }
    }
}