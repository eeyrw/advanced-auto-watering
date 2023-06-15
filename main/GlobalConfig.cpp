#include "GlobalConfig.h"

template <>
esp_err_t ConfigRead<std::string>(const char *ns, const char *key, std::string &value)
{
    esp_err_t err = ESP_FAIL;

    // auto namespaceIter = GlobalConfigMap.find(ns);
    // if (namespaceIter == GlobalConfigMap.end())
    // {
    //     err = ESP_FAIL;
    //     ESP_LOGE("GLOBAL_CONFIG", "Fail to find namespace %s!", ns);
    // }
    // else
    // {
    //     std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(namespaceIter->first, NVS_READWRITE, &err);
    //     auto kvPairIter = namespaceIter->second.find(key);
    //     if (kvPairIter == namespaceIter->second.end())
    //     {
    //         err = ESP_FAIL;
    //         ESP_LOGE("GLOBAL_CONFIG", "Fail to find key %s!", key);
    //     }
    //     else
    //     {
    //         auto defaultValue = std::any_cast<T>(kvPairIter->second);

    //         if (err != ESP_OK)
    //         {
    //             ESP_LOGE("GLOBAL_CONFIG", "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    //         }
    //         else
    //         {
    //             err = handle->get_item(key, value);
    //             switch (err)
    //             {
    //             case ESP_OK:
    //                 ESP_LOGI("GLOBAL_CONFIG", "%s = %d", key, value);
    //                 break;
    //             case ESP_ERR_NVS_NOT_FOUND:
    //                 ESP_LOGW("GLOBAL_CONFIG", "Key "
    //                                           "%s"
    //                                           " is not initialized yet!",
    //                          key);
    //                 err = handle->set_item(key, defaultValue);
    //                 err = handle->commit();
    //                 if (err != ESP_OK)
    //                 {
    //                     ESP_LOGE("GLOBAL_CONFIG", "Fail to update "
    //                                               "%s"
    //                                               " due to %s.",
    //                              key, esp_err_to_name(err));
    //                 }
    //                 break;
    //             default:
    //                 ESP_LOGE("GLOBAL_CONFIG", "Fail to access "
    //                                           "%s"
    //                                           " due to %s.\n",
    //                          key, esp_err_to_name(err));
    //             }
    //         }
    //     }
    // }

    return err;
}
