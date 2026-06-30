#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tmp105.h"
#include "esp_system.h"
#include "esp_log.h"

#include "sdkconfig.h"

static const char *TAG = "TMP105 reader";

#define I2C_SCL_IO CONFIG_I2C_SCL /*!< gpio number for I2C master clock */
#define I2C_SDA_IO CONFIG_I2C_SDA /*!< gpio number for I2C master data  */
#define I2C_PORT 0                /*!< I2C port number for master dev */
#define I2C_FREQ_HZ 100000        /*!< I2C master clock frequency */

#define TMP105_ADDR TMP105_ADDRESS_0

void app_main()
{
    int16_t temp_raw;
    float temp_deg;

    i2c_master_bus_handle_t i2c_bus_handle;
    tmp105_handle_t tmp_handle = NULL;

    const i2c_config_t i2c_bus_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };

    i2c_bus_handle = i2c_bus_create(I2C_PORT, &i2c_bus_conf);
    if (i2c_bus_handle == NULL){
        ESP_LOGE(TAG,  "i2c_bus create returned NULL");
        abort();
    }

    tmp105_i2c_config_t i2c_conf = {
        .bus_inst = i2c_bus_handle,
        .i2c_addr = TMP105_ADDR,
    };

    ESP_ERROR_CHECK(tmp105_create(&i2c_conf, &tmp_handle));

    while (true)
    {
        vTaskDelay(CONFIG_READ_INTERVAL / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(tmp105_read_temperature(tmp_handle, &temp_raw, &temp_deg));
        printf("Temperature Raw: %" PRId16 "; \tTemperature: %f deg C \r\n", temp_raw, temp_deg);
    }
}