#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#define TACH_PIN 12
#define LEDC_PIN 13

volatile unsigned int num_pulses;
static float rpm;

static void count_pulses(void* arg) {
    num_pulses++;
}

static void read_tach(void* args) {
    num_pulses = 0;
    rpm = 0;

    // GPIO setup
    gpio_config_t io_conf;

    io_conf.pin_bit_mask = ((uint64_t)1 << TACH_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;

    gpio_config(&io_conf);

    // Install GPIO service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(TACH_PIN, count_pulses, NULL);

    while(true) {
	gpio_intr_disable(TACH_PIN);
	rpm = (num_pulses / 2) * 60;
        printf("RPM: %.0f\n", rpm);
        num_pulses = 0;
	gpio_intr_enable(TACH_PIN);
	vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main(void) {
    xTaskCreate(read_tach, "read_tach", 2048, NULL, 10, NULL);

    // Setup the timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
	.freq_hz = 25000,
	.speed_mode = LEDC_HIGH_SPEED_MODE,
	.timer_num = LEDC_TIMER_0
    };

    ledc_timer_config(&ledc_timer);

    // Setup the chaneel
    // Intital duty cycle set to 50%
    ledc_channel_config_t ledc_channel = {
	.channel = LEDC_CHANNEL_0,
	.duty = 512,
	.gpio_num = LEDC_PIN,
	.intr_type = LEDC_INTR_DISABLE,
	.speed_mode = LEDC_HIGH_SPEED_MODE,
	.timer_sel = LEDC_TIMER_0
    };

    ledc_channel_config(&ledc_channel);

    while(true) {
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

