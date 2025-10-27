#include "Driver/Dri_NVS.h"

// 声明NVS操作句柄
static nvs_handle_t my_handle;
void Dri_NVS_Init(void)
{
    // 1.初始化NVS_FLASH模块
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    //2.打开NVS命名空间
    err = nvs_open("pwd", NVS_READWRITE, &my_handle);
}

esp_err_t Dri_NVS_ReadU8(char *key, uint8_t *value)
{
    // 读取数据
    if (nvs_find_key(my_handle, key, NULL) == ESP_OK)
    {
        return nvs_get_u8(my_handle, key, value);
    }
    else
    {
        return ESP_FAIL;
    }
}


esp_err_t Dri_NVS_WriteU8(char *key, uint8_t value)
{
    // //判断Key是否存在
    // //nvs_type_t* out_type是让填value的类型是u8/i8，但是我们只要key所以无所谓填NULL
    // if(nvs_find_key(my_handle,key,NULL) == ESP_OK)
    // {
    //     //已经有key了，不能在写了
    //     return ESP_FAIL;
    // }
    // else
    // {
    //     //存储密码
    //     return nvs_set_u8(my_handle,key,value);
    // }
    return nvs_set_u8(my_handle,key,value);
}


esp_err_t Dri_NVS_DeletePwd(char *key)
{
     //判断Key是否存在
    if(nvs_find_key(my_handle,key,NULL) == ESP_OK)
    {
        //删除指定的key
        return nvs_erase_key(my_handle,key);
    }
    else
    {
        //要删除的key没有，无法删除
        return ESP_FAIL;
    }
}



// 判断key是否存在函数
esp_err_t Dri_NVS_IsPwdExists(char *key)
{
    return nvs_find_key(my_handle,key,NULL);
}