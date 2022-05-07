

#ifndef _WIFI_H
#define _WIFI_H

#include "esp_err.h"

uint8_t _Connection_sts_wifi;

esp_err_t initialize_wifi(const char *wifi_name, const char *wifi_password);
void wait_wifi_Connection();
uint8_t is_wifi_connected();
void disconnect_wifi();

#endif
