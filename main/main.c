// This flag can be used to compile the code with sensor or without sensor
// depending if it's commented or not
#define HAS_SENSORS

#include <stdlib.h>
#include "mesh_control.h"
#include "mqtt_control.h"
#include "utils.h"
#include "esp_mesh.h"

#ifdef HAS_SENSORS
#include "dht.h"
#include "moisture_sensor.h"
#endif

static const char *MAIN_TAG = "main";

void send_capture_to_root(char* buffer)
{
    mesh_data_t data;
    data.data = (uint8_t*) buffer;
    data.size = strlen(buffer);
    data.proto = MESH_PROTO_BIN;
    data.tos = MESH_TOS_P2P;
    esp_err_t err = esp_mesh_send(NULL, &data, 0, NULL, 0);
    if (err) {
        ESP_LOGE(MAIN_TAG, "send_capture_to_root Error sending data");
    }
}

#ifdef HAS_SENSORS
/**
 * @brief Task for capturing from the DHT22 sensor
 * 
 * @param args arguments for the task
 */
void task_read_temp(void *args)
{
    float temp;
    float humidity;
    float prev_temp = 0;
    long last_capture = 0; // Time in seconds since last publish
    char data[40];
    while (1) {
        dht_read_float_data(DHT_TYPE_AM2301, GPIO_NUM_4, &humidity, &temp);

        // Only sends data if had passed enough time or if there was a big variation
        if ((abs(temp - prev_temp) > config_variation) || (esp_timer_get_time() / 1000000 - last_capture) > config_time) {
            prev_temp = temp;
            last_capture = esp_timer_get_time() / 1000000;
            ESP_LOGI(MAIN_TAG, "Temperature %.4f", temp);
            // The root node sends directly to mqtt
            if (mesh_layer == 1) {
                sprintf(data, "%.4f", temp);
                // Publish to mqtt
                mqtt_app_publish("redes/sensor/temperatura", data);
            } else {
                sprintf(data, "redes/sensor/temperatura;%.4f", temp);
                send_capture_to_root(data);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Task for capturing from the moisture sensor
 * 
 * @param args arguments for the task
 */
void task_read_humidity(void *args)
{
    float humidity;
    float prev_hum = 0;
    long last_capture = 0; // Time in seconds since last publish
    char data[40];
    while (1) {
        humidity = calcular_porcentaje_humedad();

        // Only sends data if had passed enough time or if there was a big variation
        if ((abs(humidity - prev_hum) > config_variation) || (esp_timer_get_time() / 1000000 - last_capture) > config_time) {
            prev_hum = humidity;
            last_capture = esp_timer_get_time() / 1000000;
            ESP_LOGI(MAIN_TAG, "Humidity %.4f", humidity);
            // The root node sends directly to mqtt
            if (mesh_layer == 1) {
                sprintf(data, "%.4f", humidity);
                mqtt_app_publish("redes/sensor/humedad", data);
            } else {
                sprintf(data, "redes/sensor/humedad;%.4f", humidity);
                send_capture_to_root(data);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
#endif

void send_config_to_nodes(char* buffer)
{
    mesh_data_t data;
    data.data = (uint8_t*) buffer;
    data.size = strlen(buffer);
    data.proto = MESH_PROTO_BIN;
    data.tos = MESH_TOS_P2P;
    mesh_addr_t multicast_group = {
        .addr = { 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 }
    };
    esp_err_t err = esp_mesh_send(&multicast_group, &data, MESH_DATA_GROUP, NULL, 0);
    if (err) {
        ESP_LOGE(MAIN_TAG, "send_config_to_nodes Error sending data");
    }
}

/**
 * @brief This callback method will be called every time we receive data
 * from the mqqtt subscription, we use it to manage the sensor config.
 * 
 * @param topic the topic of the subscription
 * @param message the receive data
 */
void config_subscription(char *topic, char *message)
{
    // Send data via mesh
    ESP_LOGI(MAIN_TAG, "Sending config to leaf");
    send_config_to_nodes(message);
}

void app_main(void)
{
    // Connect to mesh network and wait
    ESP_ERROR_CHECK(init_mesh());
    while(!is_mesh_connected)
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Only the root node must be connected to mqtt
    if (mesh_layer == 1) {
        mqtt_app_start(config_subscription);
        while(!is_mqtt_connected)
            vTaskDelay(1000 / portTICK_PERIOD_MS);

        mqtt_app_subscribe("redes/configuracion");
    }

#ifdef HAS_SENSORS
    // Launch capturing tasks
    xTaskCreate(task_read_temp, "task_read_temp", 2048, NULL, 1, NULL);
    xTaskCreate(task_read_humidity, "task_read_humidity", 2048, NULL, 1, NULL);
#endif
}
