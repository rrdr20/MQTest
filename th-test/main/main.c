#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "sdkconfig.h"

const static int DHT_PIN = 25;       // GPIO pin number

const static int RMT_CHANNEL = 0;    // first available channel
const static int RMT_CLK_DIV = 80;   // APB runs at 80MHz (default), could change depending on clock source
const static int RMT_TICK_US = 1;    // set here to 1 for micro second timing, could change depending on clock frequency
const static int RMT_TIMEOUT = 1000; // wait 1 milli second before timing out

static void dht_rmt_init(void) {
    // Setup RMT parameters
    // configure for RX
    rmt_config_t dht_rx;

    dht_rx.gpio_num = DHT_PIN;
    dht_rx.channel = RMT_CHANNEL;
    dht_rx.clk_div = RMT_CLK_DIV;
    dht_rx.rmt_mode = RMT_MODE_RX;
    dht_rx.mem_block_num = 1;
    dht_rx.rx_config.filter_en = false;
    dht_rx.rx_config.filter_ticks_thresh = 100;
    dht_rx.rx_config.idle_threshold = RMT_TIMEOUT / RMT_TICK_US;

    rmt_config(&dht_rx);
    rmt_driver_install(dht_rx.channel, 1024, 0);
}

static void dht_send_start(void) {
    gpio_pullup_en(DHT_PIN);

    // Set pin to output first to signal the DHT
    gpio_set_level(DHT_PIN, 1);
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    ets_delay_us(1000);

    // Pull the signal low for 20ms to allow the DHT to detect
    gpio_set_level(DHT_PIN, 0);
    ets_delay_us(20000);

    // Pull the pin back high and set to input to receive data from DHT
    gpio_set_level(DHT_PIN, 1);
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);
}

void parse_items(rmt_item32_t* item, int num_items, int* humidity, int* temperature) {
    // DHT11 outputs 40bits of data by varying pulses
    // RMT item holds the duration and signal levels
    // First duration (0) is approx. 50us and is always 0
    // Second duration (1) is either 28-35us long 0 or above 7us for 1

    // Start + 40bits + end = 42 items
    // Less could be a bad reading so return and use last value
    if(num_items < 42) {
        return;
    }
    // Skip the first item (no actual data)
    item++;
    
    // Initialize a int to 0 so only need to check on 1 and flip bit
    // 8 of 16 bits are always 0, parsed here as other sensors may provide
    // Read humidity
    int parse_humidity = 0;
    for(int i = 0; i < 16; i++, item++) {
        if(item->duration1 > 35) {
            parse_humidity |= ((uint32_t)1 << (7 - i));
        }
    }
    
    // Repeat for temperature
    int parse_temperature = 0;
    for(int i = 0; i < 16; i++, item++) {
        if(item->duration1 > 35) {
            parse_temperature |= ((uint32_t)1 << (7 - i));
        }
    }
    
    // Finally read 8 items for checksum value
    int checksum = 0;
    for(int i = 0; i < 8; i++, item++) {
        if(item->duration1 > 35) {
            checksum |= ((uint32_t)1 << (7 - i));
        }
    }

    if(parse_humidity + parse_temperature == checksum) {
        *humidity = parse_humidity;
        *temperature = parse_temperature;
    }
}

static void get_data(void* args) {
    static int humidity = 0;
    static int temperature = 0;
    int num_items;
    size_t rx_size = 0;
    rmt_item32_t* item;
    RingbufHandle_t rb = NULL;

    dht_rmt_init();
    rmt_get_ringbuf_handle(RMT_CHANNEL, &rb);

    if(!rb) {
        return;
    }

    while(true) {
        dht_send_start();
        
        rmt_rx_start(RMT_CHANNEL, 1);
        item = xRingbufferReceive(rb, &rx_size, 1000);
        rmt_rx_stop(RMT_CHANNEL);

        if(item != NULL) {
            num_items = (rx_size / sizeof(rmt_item32_t));
            parse_items(item, num_items, &humidity, &temperature);
        }

        vRingbufferReturnItem(rb, (void*) item);

        printf("temp: %dC, RH: %d%%\n", temperature, humidity);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main() {
    xTaskCreate(get_data, "get_data", 2048, NULL, 10, NULL);    
}
