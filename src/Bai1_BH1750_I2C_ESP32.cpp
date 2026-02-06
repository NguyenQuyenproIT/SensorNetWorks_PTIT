#include <Arduino.h>
#include "driver/i2c.h"


#define I2C_PORT        I2C_NUM_0 (Bộ I2C0)
#define I2C_SDA_PIN     21
#define I2C_SCL_PIN     22
#define I2C_FREQ_HZ     100000

// 7-bit address (ADDR - GND)
#define BH1750_ADDR     0x23

// Commands (datasheet)
#define BH1750_POWER_ON     0x01
#define BH1750_RESET        0x07
#define BH1750_CONT_HRES    0x10   // quét liên tục

void i2c_init(void)
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_PIN;
    conf.scl_io_num = I2C_SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ_HZ;

    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
}

esp_err_t bh1750_write_cmd(uint8_t cmd)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, (BH1750_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, cmd, true);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, handle, pdMS_TO_TICKS(100));

    i2c_cmd_link_delete(handle);
    return ret;
}

void bh1750_init(void)
{
    esp_err_t err;

    err = bh1750_write_cmd(BH1750_POWER_ON);
    Serial.printf("BH1750 POWER ON: %s\n", esp_err_to_name(err));
    delay(10);

    err = bh1750_write_cmd(BH1750_RESET);
    Serial.printf("BH1750 RESET   : %s\n", esp_err_to_name(err));
    delay(10);

    err = bh1750_write_cmd(BH1750_CONT_HRES);
    Serial.printf("BH1750 MODE    : %s\n", esp_err_to_name(err));
    delay(180);   // datasheet: typ 120 ms
}

uint16_t bh1750_read_raw(void)
{
    uint8_t data[2] = {0};

    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, (BH1750_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(handle, data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(
        I2C_PORT, handle, pdMS_TO_TICKS(100)
    );

    i2c_cmd_link_delete(handle);

    if (ret != ESP_OK) {
        Serial.printf("SENSOR ERROR: %s\n", esp_err_to_name(ret));
        return 0;
    }

    return (data[0] << 8) | data[1];
}

float bh1750_read_lux(void)
{
    uint16_t raw = bh1750_read_raw();
    return raw / 1.2f;
}

void setup(void)
{
    Serial.begin(115200);
    delay(100);
    Serial.println("Start!!");
    
    i2c_init();
    bh1750_init();
}

void loop(void)
{
    float lux = bh1750_read_lux();
    Serial.printf("Light: %.2f lux value\n", lux);
    delay(1000);
}


