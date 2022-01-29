
#ifndef DWELLL_H
#define DWELLL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_spiffs.h"
#include "esp_timer.h"
#include <time.h>
#include "sqlite3.h"
#include <string.h>
#include "esp_log.h"
#include "stdlib.h"


struct timeval now;
//uint8_t channel = 1;
void wifi_sniffer_init(void);
//void wifi_sniffer_set_channel(uint8_t channel);
void *dwellTime(int *brr, int c);

#endif
