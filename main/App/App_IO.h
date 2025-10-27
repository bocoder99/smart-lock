#ifndef __APP_IO_H__
#define __APP_IO_H__

#include "Dri_NVS.h"
#include "Inf_BDR6120.h"
#include "Inf_SC12B.h"
#include "Inf_WS2812.h"
#include "Inf_WTN6170.h"
#include "Inf_FPM383.h"

#define PWD_CT "pwd_ct"

/* 输入状态 */
typedef enum
{
    FREE = 0, /* 自由 */
    INPUT,    /* 输入阶段 */
    DONE      /* 输入完成 */
} Input_Status;

/* 密码状态 */
typedef enum
{
    ADD = 0, /* 添加 */
    DELETE,    /* 删除 */
    CHECK      /* 检查 */
} Pwd_Op_Status;

void App_IO_Init(void);

void App_IO_KeyScan(void);

void App_IO_FingerScan(void);


void App_IO_AddPwd(uint8_t password[]);
void App_IO_DelPwd(uint8_t password[]);
void App_IO_CheckPwd(uint8_t password[]);

#endif /* __APP_IO_H__ */
