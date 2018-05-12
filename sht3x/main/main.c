#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"

const static uint32_t I2C_PORT = 0;
const static uint32_t SCL_PIN = 26;
const static uint32_t SDA_PIN = 27;
const static uint32_t CLK_FREQ = 100000;

const static uint8_t SHT_ADDR = 0x45;
const static uint8_t SHT_CMD_MEAS = 0x24;
const static uint8_t SHT_CMD_REP = 0x00;

uint8_t crc8(const uint8_t *data, int len) {
	/*
	 * CRC-8 formula from page 14 of SHT spec pdf
	 * Test data 0xBE, 0xEF should yield 0x92
	 *
	 * Initialization data 0xFF
	 * Polynomial 0x31 (x8 + x5 +x4 +1)
	 * Final XOR 0x00
	 */

	const uint8_t POLYNOMIAL = 0x31;
	uint8_t crc = 0xFF;

	for (int j = len; j; --j) {
		crc ^= *data++;

		for (int i = 8; i; --i) {
			crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
		}
	}
	return crc;
}

void master_init(void) {
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = CLK_FREQ;

    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    
    vTaskDelay(200 / portTICK_PERIOD_MS);
}

void print_data(uint8_t* data, size_t len) {
//  uint16_t temperature = 0;
//  uint16_t humidity = 0;

//  temperature = data[0];
//  temperature <<= 8;
//  temperature |= data[1];

//  humidity = data[3];
//  humidity <<= 8;
//  humidity |= data[4];
//  printf("temperature: %d, humidity: %d\n", temperature, humidity);

    for(int i = 0; i < len; i++) {
        printf("%x ", data[i]);
    }
    printf("\n");
}

void status(void) {
    uint8_t data[3];
    
    // Send status command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_WRITE, 0x1);
    i2c_master_write_byte(cmd, 0xf3, 0x1);
    i2c_master_write_byte(cmd, 0x2d, 0x1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Read status
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_READ, 0x1);
    i2c_master_read_byte(cmd, data, 0x0);
    i2c_master_read_byte(cmd, data + 1, 0x0);
    i2c_master_read_byte(cmd, data + 2, 0x1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    uint16_t status = 0;
    status = data[0];
    status <<= 8;
    status |= data[1];

    uint8_t check = crc8(data, 2);

    print_data(data, sizeof(data));
    printf("status: %d, data: %x, check: %x\n", status, data[2], check);
}

void reset(void) {
    // Send reset command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_WRITE, 0x1);
    i2c_master_write_byte(cmd, 0x30, 0x1);
    i2c_master_write_byte(cmd, 0xA2, 0x1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

void read_sensor(void) {
    uint8_t data[6];

    // Send measurement command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x24, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Read data
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, data, 0x0);
    i2c_master_read_byte(cmd, data + 1, 0x0);
    i2c_master_read_byte(cmd, data + 2, 0x0);
    i2c_master_read_byte(cmd, data + 3, 0x0);
    i2c_master_read_byte(cmd, data + 4, 0x0);
    i2c_master_read_byte(cmd, data + 5, 0x1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    print_data(data, sizeof(data));
}

void app_main(void) {
   master_init();
   reset();
   status();
   while(true) {
       read_sensor();
       status();
       vTaskDelay(5000 / portTICK_PERIOD_MS);
   }
}
