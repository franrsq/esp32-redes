
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_log.h"
#include "wifi.h"
#include "mqtt_control.h"
#include "moisture_sensor.h"
#include "dht_sensor.h"

int tiempo = 120;
float variacion = 0.5;

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

float calc_abs(float a) {
    if (a < 0)
        return -1 * a;
    return a;
}

void task_captura_humedad(void *args)
{
    float humedad_ant = 0;
    float humedad = 0;
    long ultima_captura = 0;
    char data[20];
    while (1) {
        humedad = calcular_porcentaje_humedad();
        if (calc_abs(humedad_ant - humedad) > variacion || (esp_timer_get_time()/1000000 - ultima_captura) > tiempo) {
            //ENVIAR
            //Agregar a cola de envío
            humedad_ant = humedad;
            ultima_captura = esp_timer_get_time()/1000000;
            ESP_LOGI("t_Hu", "Cambio humedad %.4f", humedad );
            sprintf(data, "%.4f", humedad);
            publish_data("redes/sensor/humedad", data);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void task_captura_temperatura(void *args)
{
    float temp_ant = 0;
    float temp;
    float humedad;
    long ultima_captura = 0;
    char data[20];
    while (1) {
        dht_read_float_data(DHT_TYPE_AM2301, GPIO_NUM_4, &humedad, &temp);
        if (calc_abs(temp_ant - temp) > variacion || (esp_timer_get_time()/1000000 - ultima_captura) > tiempo) {
            //ENVIAR
            //Agregar a cola de envío
            temp_ant = temp;
            ultima_captura = esp_timer_get_time()/1000000;
            ESP_LOGI("t_Tm", "Cambio temperatura %.4f", temp );
            sprintf(data, "%.4f", temp);
            publish_data("redes/sensor/temperatura", data);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void manejar_subscripcion(const char *topic, const char *message) {
    char** tokens;
    tokens = str_split(message, ',');
    tiempo = atoi(tokens[0]);
    variacion = strtof(tokens[1], NULL);
    ESP_LOGI("config", "%d %f", tiempo, variacion);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    initialize_wifi("SQ", "famsotoq20");
    wait_wifi_Connection();
    init_adc_config();
    mqtt_app_start(manejar_subscripcion);
    
    while(!is_connected)
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    subscribe_topic("redes/configuracion");
    xTaskCreate(task_captura_humedad, "task_captura_hum", 2048, NULL, 1, NULL);
    xTaskCreate(task_captura_temperatura, "task_captura_temp", 2048, NULL, 1, NULL);
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}
