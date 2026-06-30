#include <stdio.h>
#include "esp_log.h"
#include "esp_check.h"

#include "tmp105.h"

#define TEMPERATURE_REGISTER (0x00)
#define CONFIG_REGISTER (0x01)
#define T_LOW_REGISTER (0x02)
#define T_HIGH_REGISTER (0x03)

#define TMP_CONVERSION_FACTOR (0.0625)

#define MIN_TEMP (-40.0f)
#define MAX_TEMP (125.0f)

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

/**
 * @brief makes sure the temperature is within the sensor's operation range
 *
 * @param[in] temperature termperature in degree C to clamp
 * @return float clamped temperature in degree C
 */
static inline float clamp_temp(float temperature)
{
    if (temperature > MAX_TEMP)
    {
        temperature = MAX_TEMP;
    }
    if (temperature < MIN_TEMP)
    {
        temperature = MIN_TEMP;
    }

    return temperature;
}

/**
 * @brief convert temperature value to its register representation
 *
 * @param[in] temperature temperature in degree C
 * @param[out] buff buffer to store register value
 *
 * @note buffer is assumed to have a size of 2
 */
static inline void temp_to_reg_bytes(float temperature, uint8_t *buff)
{
    temperature = clamp_temp(temperature);

    // convert temperature to digital value
    temperature = temperature / 0.0625;
    buff[0] = (int)temperature >> 4;
    buff[1] = (int)temperature << 4;
}

/**
 * @brief read temperature value from a register
 *
 * @param[in] handle TMP105 device handle
 * @param[in] reg register to read temperature from (Temp, T_high, T_low)
 * @param[out] temperature_raw analog raw value of temperature
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t tmp105_read_temp_reg(tmp105_handle_t handle, uint8_t reg, int16_t *temperature_raw)
{
    uint8_t buf[2];

    ESP_RETURN_ON_ERROR(tmp105_read_reg(handle, reg, buf, 2), TAG, "I2C read error");
    *temperature_raw = ((buf[0]) << 4) | (buf[1] >> 4);

    // if raw value > 0x7FF then temperature is -ve
    // use 2's complement in that case
    if (*temperature_raw > 0x7FF)
    {
        *temperature_raw |= 0xF000;
    }

    return ESP_OK;
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
    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");

    ESP_RETURN_ON_ERROR(tmp105_read_temp_reg(handle, TEMPERATURE_REGISTER, temperature_raw), TAG, "Temperature read failed");
    *temperature = ((float)*temperature_raw) * TMP_CONVERSION_FACTOR;

    return ESP_OK;
}

esp_err_t tmp105_set_high_temperature(tmp105_handle_t handle, float temperature)
{
    uint8_t buff[2];

    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");

    temp_to_reg_bytes(temperature, buff);
    ESP_RETURN_ON_ERROR(tmp105_write_reg(handle, T_HIGH_REGISTER, buff, 2), TAG, "I2C write error");

    return ESP_OK;
}

esp_err_t tmp105_set_low_temperature(tmp105_handle_t handle, float temperature)
{
    uint8_t buff[2];

    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");

    temp_to_reg_bytes(temperature, buff);
    ESP_RETURN_ON_ERROR(tmp105_write_reg(handle, T_LOW_REGISTER, buff, 2), TAG, "I2C write error");

    return ESP_OK;
}

esp_err_t tmp105_get_high_temperature(tmp105_handle_t handle, float *temperature)
{
    int16_t temperature_raw;
    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");

    ESP_RETURN_ON_ERROR(tmp105_read_temp_reg(handle, T_HIGH_REGISTER, &temperature_raw), TAG, "Temperature read failed");
    *temperature = ((float)temperature_raw) * TMP_CONVERSION_FACTOR;

    return ESP_OK;
}

esp_err_t tmp105_get_low_temperature(tmp105_handle_t handle, float *temperature)
{
    int16_t temperature_raw;
    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "invalid device handle pointer");

    ESP_RETURN_ON_ERROR(tmp105_read_temp_reg(handle, T_LOW_REGISTER, &temperature_raw), TAG, "Temperature read failed");
    *temperature = ((float)temperature_raw) * TMP_CONVERSION_FACTOR;

    return ESP_OK;
}