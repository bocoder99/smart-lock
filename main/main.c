#include <stdio.h>
#include "App_IO.h"
#include "freertos/FreeRTOS.h"
#include "Inf/Inf_FPM383.h"
#include "App_Communication.h"
#include "Dri_Wifi.h"

extern uint8_t isTouch;
extern uint8_t isHasFinger;

TaskHandle_t KeyScanHandle;
void Key_Scan_Task(void *);

TaskHandle_t fingerScanHandle;
void Finger_Scan_Task(void *);

TaskHandle_t otaHandle;
void OTA_Task(void *);

int app_main(void)
{
    
    //1.所有模块初始化
    App_IO_Init();
    App_Communication_Init();

    //读取设备号
    Inf_FPM383_ReadId();
    Inf_FPM383_Sleep();
    //wifi初始化
    //Dri_Wifi_Init();
 

    xTaskCreate(Key_Scan_Task,"keytask",2048,NULL,5,&KeyScanHandle);

    xTaskCreate(Finger_Scan_Task,"fingertask",2048,NULL,5,&fingerScanHandle);

    xTaskCreate(OTA_Task,"otatask",8196,NULL,5,&otaHandle);

    return 0;
}


/**
 * @brief 按键扫描调用
 * 
 */
void Key_Scan_Task(void *)
{
    TickType_t keytime =  xTaskGetTickCount();

    while(1)
    {
        App_IO_KeyScan();
        xTaskDelayUntil(&keytime,10);
    }
}


/**
 * @brief 指纹任务
 * 
 */
void Finger_Scan_Task(void *)
{

    TickType_t fingertime =  xTaskGetTickCount();

    while(1)
    {
        App_IO_FingerScan();
        xTaskDelayUntil(&fingertime,50);
    }
}


//通过指令唤醒
void OTA_Task(void *)
{
    uint32_t action = 0;

    while(1)
    {
        //等待时间最高，一直等着，直到指令来
        xTaskNotifyWait(0xFFFFFFFF,0xFFFFFFFF,&action,portMAX_DELAY);
        if(action == '4')
        {
            //执行OTA升级
            App_Communication_OTA();
        }
    }
}