#ifndef __MESH_CONTROL_H__
#define __MESH_CONTROL_H__

#include <string.h>
#include "mesh_control.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"

/*******************************************************
 *                Constants
 *******************************************************/
#define RX_SIZE          (1500)
#define TX_SIZE          (1460)

#define CONFIG_MESH_ROUTE_TABLE_SIZE 50
#define CONFIG_MESH_TOPOLOGY 0
#define CONFIG_MESH_MAX_LAYER 6
#define CONFIG_MESH_CHANNEL 0
#define CONFIG_MESH_ROUTER_SSID "SQ"
#define CONFIG_MESH_ROUTER_PASSWD "famsotoq20"
#define CONFIG_MESH_AP_AUTHMODE 3
#define CONFIG_MESH_AP_CONNECTIONS 2
#define CONFIG_MESH_NON_MESH_AP_CONNECTIONS 0
#define CONFIG_MESH_AP_PASSWD "RedesIoT"

/*******************************************************
 *                Variable Definitions
 *******************************************************/
bool is_running;
bool is_mesh_connected;
int mesh_layer;
int config_time;
float config_variation;

esp_err_t init_mesh();

#endif
