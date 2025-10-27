#ifndef __INF_FPM383_H__
#define __INF_FPM383_H__

#include "common/common_config.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"


#define FPM_TX_PIN GPIO_NUM_21
#define FPM_RX_PIN GPIO_NUM_20
#define FPM_INTR_PIN GPIO_NUM_10

void Inf_FPM383_Init(void);

Com_Status Inf_FPM383_WriteCmd(uint8_t *cmd,uint8_t len);

Com_Status Inf_FPM383_ReadData(uint8_t len,uint16_t timeout);

void Inf_FPM383_ReadId(void);

void Inf_FPM383_Sleep(void);

void Inf_FPM383_CancelAutoAction(void);

//id从0开始，注册指纹自增，删除后，用可获取的最小的id给新指纹
uint16_t Inf_FPM383_GetMinId(void);

Com_Status Inf_FPM383_AutoEnroll(uint16_t id);

void Inf_FPM383_AddCheckSum(uint8_t *cmd,uint8_t len);

Com_Status Inf_FPM383_CheckFingerPrint(void);

int16_t Inf_FPM383_SearchFingerPrint(void);

Com_Status Inf_FPM383_DeleteFingerPrint(uint16_t id);

void Inf_FPM383_DeleteAllFingerPrint(void);

#endif
