#ifndef __DRI_NVS_H__
#define __DRI_NVS_H__

#include <stdio.h>
#include <inttypes.h>
#include "nvs_flash.h"
#include "nvs.h"

void Dri_NVS_Init(void);

esp_err_t Dri_NVS_ReadU8(char *key,uint8_t *value);

esp_err_t Dri_NVS_DeletePwd(char *key);

esp_err_t Dri_NVS_WriteU8(char *key,uint8_t value);

//判断key是否存在函数
esp_err_t Dri_NVS_IsPwdExists(char *key);

#endif /* __DRI_NVS_H__ */
