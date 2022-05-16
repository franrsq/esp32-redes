#ifndef __MQTT_CONTROL_H__
#define __MQTT_CONTROL_H__

#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_tls.h"

#include "mqtt_client.h"

bool is_mqtt_connected;

void mqtt_app_start(void (*f)(char*, char*));
void mqtt_app_publish(char*, char*);
void mqtt_app_subscribe(char*);

#endif
