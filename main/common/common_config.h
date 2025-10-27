#ifndef __COM_CONFIG_H
#define __COM_CONFIG_H

#include "esp_task.h"
#include "sys/unistd.h"

typedef enum
{
    Com_OK,
    Com_ERROR,
    Com_TIMEOUT,
    Com_OTHER,
} Com_Status;

//微秒延迟
#define delay_us(x) usleep(x)

#define delay_ms(x) vTaskDelay(x / portTICK_PERIOD_MS)

#endif