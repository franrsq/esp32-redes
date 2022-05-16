#include "mqtt_control.h"

static const char *TAG = "mqtt_control";
static esp_mqtt_client_handle_t s_client = NULL;

// pem certificate for secure connection with mqtt broker
static const uint8_t mqtt_mosquitto_pem_start[] = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIID7TCCAtWgAwIBAgIUfz8ttMn0ylRGY5nKco9x1bjSaZwwDQYJKoZIhvcNAQEL\n"
    "BQAwgYUxCzAJBgNVBAYTAkNSMREwDwYDVQQIDAhBbGFqdWVsYTELMAkGA1UEBwwC\n"
    "Q1ExDDAKBgNVBAoMA1RFQzEOMAwGA1UECwwFUmVkZXMxFjAUBgNVBAMMDTM0LjIz\n"
    "OC4xOTguNjYxIDAeBgkqhkiG9w0BCQEWEWZyYW5yc3FAZ21haWwuY29tMB4XDTIy\n"
    "MDUxNjE5NTAzMVoXDTI3MDUxNjE5NTAzMVowgYUxCzAJBgNVBAYTAkNSMREwDwYD\n"
    "VQQIDAhBbGFqdWVsYTELMAkGA1UEBwwCQ1ExDDAKBgNVBAoMA1RFQzEOMAwGA1UE\n"
    "CwwFUmVkZXMxFjAUBgNVBAMMDTM0LjIzOC4xOTguNjYxIDAeBgkqhkiG9w0BCQEW\n"
    "EWZyYW5yc3FAZ21haWwuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"
    "AQEA0DCP4cquCZYmKOwmCKQfzcJxZlcyiU0QovS1DX/C3DOnuXrwTMQh5S0tiLjw\n"
    "BWn1ogCxZ30xCFZYmrkBk3fnF3Rd5PYsj7s97EmYqQJnlXUuATxE5k2KJBOO+783\n"
    "2EcaPkbztWCeEnNNfr2wBRnvAX9S1Dg1ip3VrzuqjRffYTwq8NEmZXDZ2uCx5VRu\n"
    "FFSKNdsoECkOI6h4m0SEIOmvup0qsSdpTu0scyRFFoVRgMfxsnnr/WRXXwkGj46X\n"
    "5hg4u4ceLOw7AQb0AoaGziilR4l0rY14H3n7wufXnmxpcbla/yZ5xug2hq5vfL4S\n"
    "elQNEu2hOnIVFApelmTW12qYVwIDAQABo1MwUTAdBgNVHQ4EFgQUpFWXAhafLBmg\n"
    "rhpyqyUs4GRq2wUwHwYDVR0jBBgwFoAUpFWXAhafLBmgrhpyqyUs4GRq2wUwDwYD\n"
    "VR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAsxqIB5POV7K6igUb4A4w\n"
    "AdGV+rOdRjLEOJuOA/7sCfG6eUkiVfc7/jqbvZPpSAElCrCYu7FRGXvZMZfssGDW\n"
    "JHog+YhLO9jPDHfZTKmRuSXXBLMcrKwBPjx1DU/UAzO5S7PH6TBzDRyoxIqf7vio\n"
    "mW9IQDWAXRW9H3sSxWB1gJcNrbe05sMjvbWIf2T9zpdyxleRVL+flarrn/phvW3A\n"
    "FCSZ2OPRZLWIa+UOlyV2HuR7F0wLQnTAigPzGeNMlVUcklHlIiBLgEoLhtn3xcS+\n"
    "YQt/Yn1Jx4UkavoMfihV5vRCm91xjZVf7ksHSmeODFNebwbuUyBda8UdM/+BGw/A\n"
    "VQ=="
    "\n-----END CERTIFICATE-----";

/**
 * @brief Mqtt event handler function
 * 
 * @param handler_args arguments, here we receive the subscription callback function
 * @param base 
 * @param event_id 
 * @param event_data 
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            is_mqtt_connected = true;
            /*if (esp_mqtt_client_subscribe(s_client, "/topic/ip_mesh/key_pressed", 0) < 0) {
                // Disconnect to retry the subscribe after auto-reconnect timeout
                esp_mqtt_client_disconnect(s_client);
            }*/
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            is_mqtt_connected = false;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
            // Format data and call callback function
            char *topic;
            char *data;
            asprintf(&topic, "%.*s", event->topic_len, event->topic);
            asprintf(&data, "%.*s", event->data_len, event->data);
            ((void(*)(char *, char*))handler_args)(topic, data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

/**
 * @brief Publish to mqtt topic
 * 
 * @param topic topic to publish
 * @param publish_string data to publish
 */
void mqtt_app_publish(char* topic, char *publish_string)
{
    if (s_client) {
        int msg_id = esp_mqtt_client_publish(s_client, topic, publish_string, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish returned msg_id=%d", msg_id);
    }
}

/**
 * @brief Creates a subscription with mqtt
 * 
 * @param topic topic to subscribe
 */
void mqtt_app_subscribe(char* topic)
{
    if (s_client) {
        int msg_id = esp_mqtt_client_subscribe(s_client, topic, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    }
}

/**
 * @brief Creates and setup a connection with mqtt. Also setups a callback function
 * for subscriptions
 * 
 * @param data_handler callback for subscriptions results
 */
void mqtt_app_start(void (*data_handler)(char*, char*))
{
    is_mqtt_connected = false;
    // Creates a secure connectión with mqtt server
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://34.238.198.66:8883",
        .cert_pem = (const char *)mqtt_mosquitto_pem_start,
        .username = "admin",
        .password = "12345",
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    // Setups the event listeners, we pass the callback function
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, data_handler);
    esp_mqtt_client_start(s_client);
}
