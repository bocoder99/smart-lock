#ifndef __INF_WS2812_H__
#define __INF_WS2812_H__

#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "common/common_config.h"



/*  使用RMT  外设时的计时器分辨率，以赫兹（Hz）为单位  */
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 //  10MHz  resolution,  1  tick  =  0.1us  (led  strip  needs  a  high  resolution)
/*  使用的哪个gpio口  */
#define RMT_LED_STRIP_GPIO_NUM 6
/*  led的数量  */
#define EXAMPLE_LED_NUMBERS 12


//全彩才用的上
#define EXAMPLE_FRAME_DURATION_MS 20
#define EXAMPLE_ANGLE_INC_FRAME 0.02
#define EXAMPLE_ANGLE_INC_LED 0.3


extern uint8_t black[3];
extern uint8_t white[3];
extern uint8_t red[3];
extern uint8_t green[3];
extern uint8_t blue[3];
extern uint8_t cyan[3];   /* 青色 */
extern uint8_t purple[3]; /* 紫色 */


void Inf_WS2812_Init(void);

void Inf_WS2812_LightLed(void);

void Inf_WS2812_LightAllLeds(uint8_t color[]);

void Inf_WS2812_LightKeyLed(uint8_t index, uint8_t color[]);


#endif 