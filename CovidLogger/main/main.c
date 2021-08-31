#include <stdio.h>
#include <esp_wifi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

//Definition of wifi access data(censord)
#define ESP_WIFI_SSID "NetworkName"
#define ESP_WIFI_PASSWORD "Password"


/*
Establish the WIFI connection to access point
*/
void wifi_start()
{
    //init phase
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //configuration phase
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    //start phase
    ESP_ERROR_CHECK(esp_wifi_start() );

    //connect phase
    esp_wifi_connect();

}


/*
Main function
*/
void app_main(void)
{
    printf("Hello world!\n");

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    wifi_start();
    

    printf("Restarting now.\n");
    fflush(stdout);

    esp_wifi_disconnect();
}
