#include "Inf/Inf_WS2812.h"

// 定义了一个数组，用于存储LED灯带中每个LED的RGB值。
static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];

static rmt_channel_handle_t led_chan = NULL;
// 简单编码器句柄
static rmt_encoder_handle_t simple_encoder = NULL;

/* 定义几种常见颜色 */
uint8_t black[3] = {0, 0, 0};
uint8_t white[3] = {255, 255, 255};
uint8_t red[3] = {0, 255, 0};
uint8_t green[3] = {255, 0, 0};
uint8_t blue[3] = {0, 0, 255};
uint8_t cyan[3] = {255, 0, 255};   /* 青色 */
uint8_t purple[3] = {0, 255, 255}; /* 紫色 */

// 定义了RMT符号字，用于表示WS2812 LED的逻辑0。
static const rmt_symbol_word_t ws2812_zero = {
    .level0 = 1,
    .duration0 = 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T0H=0.3us
    .level1 = 0,
    .duration1 = 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T0L=0.9us
};

// 定义了RMT符号字，用于表示WS2812 LED的逻辑1。
static const rmt_symbol_word_t ws2812_one = {
    .level0 = 1,
    .duration0 = 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T1H=0.9us
    .level1 = 0,
    .duration1 = 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T1L=0.3us
};

// reset defaults to 50uS    定义了RMT符号字，用于LED的复位信号。
static const rmt_symbol_word_t ws2812_reset = {
    .level0 = 1,
    .duration0 = RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2,
    .level1 = 0,
    .duration1 = RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2,
};

// 定义了一个回调函数，用于把编码数据转换成RMT符号字。
static size_t encoder_callback(const void *data, size_t data_size,
                               size_t symbols_written, size_t symbols_free,
                               rmt_symbol_word_t *symbols, bool *done, void *arg)
{
    // 检查是否有足够的RMT符号空间来写入数据。
    if (symbols_free < 8)
    {
        return 0;
    }

    // 计算当前编码的位置，并转换数据指针为uint8_t类型。
    size_t data_pos = symbols_written / 8;
    uint8_t *data_bytes = (uint8_t *)data;

    // 检查是否还有数据需要编码。
    if (data_pos < data_size)
    {
        // Encode a byte
        // 初始化符号位置，并开始遍历每个字节的位。
        size_t symbol_pos = 0;
        for (int bitmask = 0x80; bitmask != 0; bitmask >>= 1)
        {
            // 根据位的值，将逻辑0或逻辑1的RMT符号字写入符号数组。
            if (data_bytes[data_pos] & bitmask)
            {
                symbols[symbol_pos++] = ws2812_one;
            }
            else
            {
                symbols[symbol_pos++] = ws2812_zero;
            }
        }
        // We're done; we should have written 8 symbols.
        // 如果所有数据都已编码。
        return symbol_pos;
    }
    else
    {
        // All bytes already are encoded.
        // Encode the reset, and we're done.
        symbols[0] = ws2812_reset;
        *done = 1; // Indicate end of the transaction.
        return 1;  // we only wrote one symbol
    }
    // 编码复位信号，并标记传输结束。
}

void Inf_WS2812_Init(void)
{
    // 1.配置RMT TX通道。
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    // 2.创建新的RMT TX通道。
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    // 3.配置简单的基于回调的RMT编码器。
    const rmt_simple_encoder_config_t simple_encoder_cfg = {
        .callback = encoder_callback
        // Note we don't set min_chunk_size here as the default of 64 is good enough.
    };
    // 4.创建新的编码器
    ESP_ERROR_CHECK(rmt_new_simple_encoder(&simple_encoder_cfg, &simple_encoder));
    // 5.启动发送通道
    ESP_ERROR_CHECK(rmt_enable(led_chan));
}

void Inf_WS2812_LightLed(void)
{
    // 配置信息
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };
    // 使用RMT传输LED数据。亮什么颜色根据led_strip_pixels数组的值来
    rmt_transmit(led_chan, simple_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config);
    // 等待所有RMT传输完成。
    rmt_tx_wait_all_done(led_chan, portMAX_DELAY);
}

/** 12*3 =36的数组，分三个一组,0,255,0,0,255,0.....拷贝进去
 * @brief 所有灯亮同一个颜色
 *
 * @param color
 */
void Inf_WS2812_LightAllLeds(uint8_t color[])
{
    // dest：目标内存地址的指针
    // src：源内存地址的指针
    // n：要复制的字节数
    for (uint8_t i = 0; i < EXAMPLE_LED_NUMBERS; i++)
    {
        memcpy(&led_strip_pixels[3 * i], color, 3);
    }

    Inf_WS2812_LightLed();
}

/**
 * @brief 让指定灯亮指定的颜色
 *
 * @param index 指定的索引
 * @param color 指定的颜色
 */
void Inf_WS2812_LightKeyLed(uint8_t index, uint8_t color[])
{
    //首先关闭所有灯，黑色;大数组全写0
    Inf_WS2812_LightAllLeds(black);

    //2.修改指定灯颜色的数据
    memcpy(&led_strip_pixels[3 * index], color, 3);

    //3.亮灯
    Inf_WS2812_LightLed();
}