#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "MQTTClient.h"

const int CONNECTED_BIT = BIT0;
static EventGroupHandle_t wifi_event_group;

static const char *TAG = "MCU-MQ";

static esp_err_t event_handler(void *ctx, system_event_t *event) {
    switch(event->event_id) {
	case SYSTEM_EVENT_STA_START:
	    esp_wifi_connect();
	    break;
	case SYSTEM_EVENT_STA_GOT_IP:
	    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
	    break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
	    esp_wifi_connect();
	    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
	    break;
	default:
	    break;
    }
    return ESP_OK;
}

static void initialize_wifi(void) {
    tcpip_adapter_init();

    wifi_event_group = xEventGroupCreate();
    esp_event_loop_init(event_handler, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    wifi_config_t sta_config = {
	.sta = {
	    .ssid = "ssid",
	    .password = "password",
	},
    };

    esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config);
    esp_wifi_start();
}

void mqtt_task(void *none) {
    MQTTClient client;
    Network network;
    MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;

    // Need to implement way to pass in data.
    // Method should be more dynamic than menuconfig. 

    char* mqtt_server = "192.168.15.240";
    unsigned char read_buf[64];
    unsigned char send_buf[64];
    int rc = 0;
    int count = 0; 

    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, send_buf, sizeof(send_buf), read_buf, sizeof(read_buf));

    if((rc = NetworkConnect(&network, mqtt_server, 1883)) != 0) {
	ESP_LOGE(TAG, "NetworkConnect error: %d", rc);
    }

    // TODO: Update connect_data with parameters, e.g. keepalive.
    // Code below works for basica verifcation only.

    if((rc = MQTTConnect(&client, &connect_data)) != 0) {
	ESP_LOGE(TAG, "MQTTConnect error: %d", rc);
    }
    else {
	ESP_LOGI(TAG, "MQTT Connected");
    }

    while(++count) {
        MQTTMessage msg;
	char payload[32];

	sprintf(payload, "message from mcu: %d", count);

	msg.qos = 1;
	msg.retained = 0;
	msg.payload = payload;
	msg.payloadlen = strlen(payload);
	
	if((rc = MQTTPublish(&client, "test/mcu/", &msg)) != 0) {
	    ESP_LOGE(TAG, "MQTTPublish error: %d", rc);
	}

        ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
	
	vTaskDelay(3*1000/portTICK_PERIOD_MS);

    }
    vTaskDelete(NULL);	
}

void app_main() {
    ESP_LOGI(TAG, "Startup..");
    ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    nvs_flash_init();
    initialize_wifi();

    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

    xTaskCreate(&mqtt_task,"mqtt_task", 4096, NULL, 5, NULL);
}

