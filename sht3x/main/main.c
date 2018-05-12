#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"

const static uint32_t I2C_PORT = 0;
const static uint32_t SCL_PIN = 26;
const static uint32_t SDA_PIN = 27;
const static uint32_t CLK_FREQ = 100000;

const static uint8_t SHT_ADDR = 0x44;
const static uint8_t SHT_CMD_MEAS_MSB = 0x24;
const static uint8_t SHT_CMD_MEAS_LSB = 0x00;
const static uint8_t SHT_CMD_RESET_MSB = 0x30;
const static uint8_t SHT_CMD_RESET_LSB = 0xA2;
const static uint8_t SHT_CMD_STATUS_MSB = 0xF3;
const static uint8_t SHT_CMD_STATUS_LSB = 0x2D;

const static uint8_t WRITE_ACK = 0x1;
const static uint8_t READ_ACK = 0x0;
const static uint8_t READ_NACK = 0x1;

uint8_t crc8(const uint8_t *data, int len) {
	/*
	 * CRC-8 formula from page 13 of SHT spec pdf March 2017 version 4
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

void init(void) {
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

void status(void) {
    uint8_t data[3];
    
    // Send status command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_WRITE, WRITE_ACK);
    i2c_master_write_byte(cmd, SHT_CMD_STATUS_MSB, WRITE_ACK);
    i2c_master_write_byte(cmd, SHT_CMD_STATUS_LSB, WRITE_ACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Read status
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_READ, WRITE_ACK);
    i2c_master_read_byte(cmd, data, READ_ACK);
    i2c_master_read_byte(cmd, data + 1, READ_ACK);
    i2c_master_read_byte(cmd, data + 2, READ_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    uint16_t status = 0;
    status = data[0];
    status <<= 8;
    status |= data[1];

    if(data[2] != crc8(data, 2)) {
        printf("status read error.\n");
        printf("status: %d\n", status);
    }
}

void reset(void) {
    // Send reset command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_WRITE, WRITE_ACK);
    i2c_master_write_byte(cmd, SHT_CMD_RESET_MSB, WRITE_ACK);
    i2c_master_write_byte(cmd, SHT_CMD_RESET_LSB, WRITE_ACK);
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
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_WRITE, WRITE_ACK);
    i2c_master_write_byte(cmd, SHT_CMD_MEAS_MSB, WRITE_ACK);
    i2c_master_write_byte(cmd, SHT_CMD_MEAS_LSB, WRITE_ACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Read data
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT_ADDR << 1 | I2C_MASTER_READ, WRITE_ACK);
    i2c_master_read_byte(cmd, data, READ_ACK);
    i2c_master_read_byte(cmd, data + 1, READ_ACK);
    i2c_master_read_byte(cmd, data + 2, READ_ACK);
    i2c_master_read_byte(cmd, data + 3, READ_ACK);
    i2c_master_read_byte(cmd, data + 4, READ_ACK);
    i2c_master_read_byte(cmd, data + 5, READ_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    uint16_t sensor_t;
    sensor_t = data[0];
    sensor_t <<= 8;
    sensor_t |= data[1];

    if(data[2] != crc8(data, 2)) {
        printf("temperature read error.\n");
        printf("sensor_t: %d", sensor_t);
    }

    uint16_t sensor_h;
    sensor_h = data[3];
    sensor_h <<= 8;
    sensor_h |= data[4];

    if(data[5] != crc8(data + 3, 2)) {
        printf("humidity read error.\n");
        printf("sensor_h: %d", sensor_h);
    }

    double temp = -49 + (315 * (((double)sensor_t) / 65535));
    double hum = 100 * (((double)sensor_h) / 65535);
    
    printf("t: %.2fF, h: %.2f%%\n", temp, hum);
}

void app_main(void) {
   init();
   reset();
   status();
   while(true) {
       read_sensor();
       vTaskDelay(5000 / portTICK_PERIOD_MS);
   }
}
