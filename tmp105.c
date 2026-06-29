#include <stdio.h>
#include "esp_log.h"
#include "esp_check.h"

#include "tmp105.h"

#define TEMPERATURE_REGISTER (0x00)
#define CONFIG_REGISTER (0x01)
#define T_LOW_REGISTER (0x02)
#define T_HIGH_REGISTER (0x03)
#define TMP_CONVERSION_FACTOR (0.0625)

const static char *TAG = "TMP105";

static esp_err_t tmp105_write_reg(tmp105_handle_t handle, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    i2c_bus_device_handle_t bus_inst = (i2c_bus_device_handle_t)handle;
    return i2c_bus_write_bytes(bus_inst, reg_addr, len, data);
}

static esp_err_t tmp105_read_reg(tmp105_handle_t handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_bus_device_handle_t bus_inst = (i2c_bus_device_handle_t)handle;
    return i2c_bus_read_bytes(bus_inst, reg_addr, len, data);
}

esp_err_t tmp105_create(const tmp105_i2c_config_t *i2c_conf, tmp105_handle_t *handle_out)
{
    ESP_RETURN_ON_FALSE(i2c_conf, ESP_ERR_INVALID_ARG, TAG, "invalid device config pointer");
    ESP_RETURN_ON_FALSE(handle_out, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");

    i2c_bus_device_handle_t i2c_device_handle = i2c_bus_device_create(i2c_conf->bus_inst, i2c_conf->i2c_addr, 0);
    if (NULL == i2c_device_handle)
    {
        ESP_LOGE(TAG, "i2c_bus_device_create failed");
        return ESP_FAIL;
    }

    *handle_out = i2c_device_handle;
    return ESP_OK;
}

esp_err_t tmp105_delete(tmp105_handle_t handle)
{
    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");
    i2c_bus_device_handle_t bus_inst = (i2c_bus_device_handle_t)handle;
    esp_err_t ret = i2c_bus_device_delete(&bus_inst);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_bus_device_delete failed");
        return ret;
    }

    return ESP_OK;
}

esp_err_t tmp105_read_temperature(tmp105_handle_t handle, int16_t *temperature_raw, float *temperature)
{
    uint8_t buf[2];

    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");

    ESP_RETURN_ON_ERROR(tmp105_read_reg(handle, TEMPERATURE_REGISTER, buf, 2), TAG, "I2C read error");
    *temperature_raw = ((buf[0]) << 4) | (buf[1] >> 4);

    // if raw value > 0x7FF then temperature is -ve
    // use 2's complement in that case
    if (*temperature_raw > 0x7FF)
    {
        *temperature_raw |= 0xF000;
    }

    *temperature = ((float)*temperature_raw) * TMP_CONVERSION_FACTOR;

    return ESP_OK;
}