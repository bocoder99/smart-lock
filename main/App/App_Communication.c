#include "App_Communication.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "string.h"
#include "esp_crt_bundle.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <sys/socket.h>
#include "esp_wifi.h"


#define HASH_LEN 32
static void get_sha256_of_partitions(void)
{
    uint8_t         sha_256[HASH_LEN] = {0};
    esp_partition_t partition;

    // get sha256 digest for bootloader
    partition.address = ESP_BOOTLOADER_OFFSET;
    partition.size    = ESP_PARTITION_TABLE_OFFSET;
    partition.type    = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
}



#define TAG "ota"
/// 处理一系列的HTTP事件
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}


static void App_Communication_OTADownloadBin(void)
{
    // esp_err_t err = nvs_flash_init();
    //wifi初始化调用过了，此处的不需要了
    // if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     nvs_flash_erase();
    //     err = nvs_flash_init();
    // }

    /* 1. 获取分区信息 */
    get_sha256_of_partitions();

    /* 2. 初始化网络 */
    esp_netif_init();

    /* 3. 创建和初始化默认事件循环 */
    esp_event_loop_create_default();

    esp_http_client_config_t config = {
        //每个人修改自己的IP地址，后面是要下载的文件名
        .url               = "http://192.168.113.85:8080/hello-world.bin",
        .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler     = NULL,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota(&ota_config);
}

void App_Communication_OTA(void)
{
    /* 1. 连接wifi(初始化) */
    Dri_Wifi_Init();

    /* 2. ota升级   使用python启动个本地http-server 命令
          C:\esp\tools\idf-python\3.11.2\python -m http.server 8080
    */
    printf("ota开始升级\r\n");
    App_Communication_OTADownloadBin();
    printf("ota完成升级\r\n");

    /* 3. 关闭wifi，可以不关，毕竟 */
    esp_wifi_stop();

    /* 4. 重启esp32 */
    esp_restart();
}





//蓝牙模块函数初始化
void App_Communication_Init(void)
{
    Dri_BT_Init();
}


/* 定义esp32收到手机数据时的回调弱函数函数 */
void  App_Communication_RecvDataCb(uint8_t *data, uint16_t dataLen)
{
    printf("接收到的数据:%s\r\n",data);

     /*
    蓝牙发送数据格式:     功能
            1:           开锁
            2:密码       设置密码
            3:密码       删除密码
    */
    /* 1. 数据长度 < 2, 直接返回, 没有任何操作 */
    if (dataLen < 2)
    {
        sayIllegalOperation();
        return;
    }
    /*
        客户端连接上蓝牙之后, 会发送锁的 序列号 +open 来开锁
            锁的序列号一般在锁出厂的时候就已经固定了,而且是唯一的
            我们可以使用 esp32的mac地址作为序列号
     */
    uint8_t pwd[100] = {0};
    switch (data[0])
    {
    case '1'://1+666666验证密码
        memcpy(pwd,&data[2],dataLen - 2);
        App_IO_CheckPwd(pwd);
        break;
    case '2'://2+555555添加密码
        memcpy(pwd,&data[2],dataLen - 2);
        App_IO_AddPwd(pwd);
        break;
    case '3'://3+666666删除密码
        memcpy(pwd,&data[2],dataLen - 2);
        App_IO_DelPwd(pwd);
        break;
    default:
        break;
    }

}