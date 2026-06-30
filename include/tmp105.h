#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_types.h"
#include "esp_err.h"

#include "i2c_bus.h"


/* TMP105 address: A0 pin low - 0x38, A0 pin high - 0x39 */
#define TMP105_ADDRESS_0 (0x48)
#define TMP105_ADDRESS_1 (0x49)

/**
 * @brief TMP105 device handle type
 *
 */
typedef void *tmp105_handle_t;

/**
 * @brief TMP105 I2C config struct
 *
 */
typedef struct {
    i2c_bus_handle_t bus_inst;    /*!< I2C bus instance handle used to communicate with the TMP105 device */
    uint8_t     i2c_addr;         /*!< I2C address of TMP105 device, can be TMP105_ADDRRES_0 or TMP105_ADDRESS_1 according to A0 pin */
} tmp105_i2c_config_t;

/**
 * @brief Create new TMP105 device handle
 * 
 * @param[in] i2c_conf Config for I2C used by TMP105
 * @param[out] handle_out New TMP105 device handle
 * @return
 *          - ESP_OK                  Device handle creation success.
 *          - ESP_ERR_INVALID_ARG     Invalid device handle or argument.
 *          - ESP_ERR_NO_MEM          Memory allocation failed.
 */
esp_err_t tmp105_create(const tmp105_i2c_config_t *i2c_conf, tmp105_handle_t *handle_out);

/**
 * @brief Delete TMP105 device handle
 * 
 * @param[in] handle TMP105 device handle
 * @return
 *          - ESP_OK                  Device handle deletion success.
 *          - ESP_ERR_INVALID_ARG     Invalid device handle or argument.
 */
esp_err_t tmp105_delete(tmp105_handle_t handle);

/**
 * @brief read temperature
 * 
 * @param[in] handle TMP105 device handle
 * @param[out] temperature_raw raw temperature value
 * @param[out] temperature converted temperature value (in degrees C)
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t tmp105_read_temperature(tmp105_handle_t handle, int16_t *temperature_raw, float *temperature);

/**
 * @brief set high temperature threshold
 * 
 * @param[in] handle TMP105 device handle
 * @param[in] temperature temperature value in degree C
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t tmp105_set_high_temperature(tmp105_handle_t handle, float temperature);

/**
 * @brief set low temperature threshold
 * 
 * @param[in] handle TMP105 device handle
 * @param[in] temperature temperature value in degree C
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t tmp105_set_low_temperature(tmp105_handle_t handle, float temperature);

/**
 * @brief read high temperature threshold
 * 
 * @param[in]  handle TMP105 device handle
 * @param[out] temperature temperature value in degree C
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t tmp105_get_high_temperature(tmp105_handle_t handle, float *temperature);

/**
 * @brief read low temperature threshold
 * 
 * @param[in]  handle TMP105 device handle
 * @param[out] temperature temperature value in degree C
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t tmp105_get_low_temperature(tmp105_handle_t handle, float *temperature);

#ifdef __cplusplus
}
#endif
