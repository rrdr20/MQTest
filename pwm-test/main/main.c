#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#define TACH_PIN 12

static int num_pulses;
static float rpm;

static void count_pulses(void* arg) {
        num_pulses++;
}

void app_main(void) {
    // GPIO setup
    gpio_config_t io_conf;

    io_conf.pin_bit_mask = (1 << TACH_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;

    gpio_config(&io_conf);

    // Install GPIO service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(TACH_PIN, count_pulses, (void*) TACH_PIN);

    while(1) {
	gpio_intr_disable(TACH_PIN);
	rpm = (num_pulses / 2) * 60;
        printf("RPM: %.0f\n", rpm);
        num_pulses = 0;
	gpio_intr_enable(TACH_PIN);
	vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

