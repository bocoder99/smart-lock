#ifndef __INF_BDR6120_H__
#define __INF_BDR6120_H__

#include "driver/gpio.h"
#include "common/common_config.h"

#define BDR6120_INA GPIO_NUM_4
#define BDR6120_INB GPIO_NUM_5

void Inf_BDR6120_Init(void);

void Inf_BDR6120_ForwardRotaTion(void);

void Inf_BDR6120_ReceverRotaTion(void);

void Inf_BDR6120_Brake(void);

void Inf_BDR6120_OpenLock(void);

#endif /* __INF_BDR6120_H__ */
