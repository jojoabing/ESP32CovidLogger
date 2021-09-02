#include <stdio.h>
#include <esp_wifi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

//Definition of wifi access data(censord)
#define ESP_WIFI_SSID "NetworkName"
#define ESP_WIFI_PASSWORD "Password"

//Access info Covid data server
#define WEB_SERVER "dj2taa9i652rf.cloudfront.net"
#define WEB_PORT "443"
#define WEB_URL "https://covid19-lake.s3.us-east-2.amazonaws.com/rearc-covid-19-world-cases-deaths-testing/csv/covid-19-world-cases-deaths-testing.csv"

static const char REQUEST[] = "GET " WEB_URL " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";


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

    //alowing time to connect to wifi
    for (int i = 10; i >= 0; i--) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}


/*
Perform HTTPS query
*/
void https_start()
{
    char buffer [512];  //holdsreceived data
    int  length, ret;

    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);

    //create https request
    size_t written_bytes = 0;
    do {
        ret=esp_tls_conn_write(tls,
                                 REQUEST + written_bytes,
                                 sizeof(REQUEST) - written_bytes);
        written_bytes += ret;

        
    } while (written_bytes < sizeof(REQUEST));

    //read https response
    do {
        length = sizeof(buffer) - 1;
        bzero(buffer, sizeof(buffer));
        ret = esp_tls_conn_read(tls, (char *)buffer, length);

        if (ret <= 0) {
            break;
        }

        length = ret;

        for (int i = 0; i < length; i++) {
            putchar(buffer[i]);
            if (buffer[i]=='\n') break;
        }
        putchar('\n');
    } while (1);

esp_tls_conn_delete(tls);
}


/*
Main function
*/
void app_main(void)
{
    esp_netif_init();
    nvs_flash_init();

    wifi_start();
    https_start();
    

    fflush(stdout);

    esp_wifi_disconnect();
}
