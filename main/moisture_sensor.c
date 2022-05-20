#include "moisture_sensor.h"

/**
 * @brief Initializes the moisture sensor
 * 
 * @return esp_err_t status of the initialization
 */
esp_err_t init_adc_config()
{
    esp_err_t err = ESP_OK;
    err = adc1_config_width(ADC_WIDTH);
    if (err != ESP_OK)
    {
        ESP_LOGE("ADC", "Hubo un error al momento de asignar el ancho del adc");
    }
    err = adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
    if (err != ESP_OK)
    {
        ESP_LOGE("ADC", "Hubo un error al configurar atenuacion del adc");
    }
    return err;
}

/**
 * @brief Takes 10 reads from the sensor and gets the averge
 * 
 * @return int the average of the last 10 reads
 */
int filtrar_datos()
{
    uint16_t contador = 0;
    int sum_valores = 0;
    while( contador++ < NUM_SAMPLES) {
        sum_valores += adc1_get_raw(ADC_CHANNEL);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    return sum_valores/NUM_SAMPLES;
}

/**
 * @brief Convert the measures of the sensor to %
 * 
 * @return float the converted value
 */
float calcular_porcentaje_humedad(){
    int valor_digital = filtrar_datos();
    return 0.0448430 * valor_digital - 12.107623;
}