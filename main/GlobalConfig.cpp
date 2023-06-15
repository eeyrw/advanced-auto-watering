#include "GlobalConfig.h"

template <>
esp_err_t ConfigRead<std::string>(const char *ns, const char *key, std::string &value)
{
    esp_err_t err = ESP_FAIL;

    auto namespaceIter = GlobalConfigMap.find(ns);
    if (namespaceIter == GlobalConfigMap.end())
    {
        err = ESP_FAIL;
        ESP_LOGE("GLOBAL_CONFIG", "Fail to find namespace %s!", ns);
    }
    else
    {
        std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(namespaceIter->first, NVS_READWRITE, &err);
        auto kvPairIter = namespaceIter->second.find(key);
        if (kvPairIter == namespaceIter->second.end())
        {
            err = ESP_FAIL;
            ESP_LOGE("GLOBAL_CONFIG", "Fail to find key %s!", key);
        }
        else
        {
            std::string defaultString = std::get<std::string>(kvPairIter->second);

            if (err != ESP_OK)
            {
                ESP_LOGE("GLOBAL_CONFIG", "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            }
            else
            {
                size_t stringLen;

                err = handle->get_item_size(nvs::ItemType::SZ, key, stringLen);
                if (err != ESP_OK)
                    goto ERR;
                ESP_LOGI("GLOBAL_CONFIG", "String len:%u", stringLen);
                value = std::string(stringLen, '\0');
                err = handle->get_string(key, (char *)value.c_str(), stringLen);
                if (err != ESP_OK)
                    goto ERR;

            ERR:
                switch (err)
                {
                case ESP_OK:
                    ESP_LOGI("GLOBAL_CONFIG", "%s = %s", key, value.c_str());
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGW("GLOBAL_CONFIG", "Key "
                                              "%s"
                                              " is not initialized yet!",
                             key);
                    err = handle->set_string(key, defaultString.c_str());
                    err = handle->commit();
                    value = defaultString;
                    if (err != ESP_OK)
                    {
                        ESP_LOGE("GLOBAL_CONFIG", "Fail to update "
                                                  "%s"
                                                  " due to %s.",
                                 key, esp_err_to_name(err));
                    }
                    break;
                default:
                    ESP_LOGE("GLOBAL_CONFIG", "Fail to access "
                                              "%s"
                                              " due to %s.\n",
                             key, esp_err_to_name(err));
                }
            }
        }
    }

    return err;
}

template <>
esp_err_t ConfigWrite(const char *ns, const char *key, const std::string &value)
{
    esp_err_t err;

    auto namespaceIter = GlobalConfigMap.find(ns);
    if (namespaceIter == GlobalConfigMap.end())
    {
        err = ESP_FAIL;
        ESP_LOGE("GLOBAL_CONFIG", "Fail to find namespace %s!", ns);
    }
    else
    {
        std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(namespaceIter->first, NVS_READWRITE, &err);
        auto kvPairIter = namespaceIter->second.find(key);
        if (kvPairIter == namespaceIter->second.end())
        {
            err = ESP_FAIL;
            ESP_LOGE("GLOBAL_CONFIG", "Fail to find key %s!", key);
        }
        else
        {

            if (err != ESP_OK)
            {
                ESP_LOGE("GLOBAL_CONFIG", "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            }
            else
            {
                err = handle->set_string(key, value.c_str());
                if (err != ESP_OK)
                    goto ERR;
                err = handle->commit();
                if (err != ESP_OK)
                    goto ERR;
            ERR:
                switch (err)
                {
                case ESP_OK:
                    ESP_LOGI("GLOBAL_CONFIG", "%s = %s", key, value.c_str());
                    break;
                default:
                    ESP_LOGE("GLOBAL_CONFIG", "Fail to write "
                                              "%s"
                                              " due to %s.\n",
                             key, esp_err_to_name(err));
                }
            }
        }
    }

    return err;
}
