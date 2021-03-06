/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef __SENSOR_LPS33HW_H__
#define __SENSOR_LPS33HW_H__

#include "os/mynewt.h"
#include "sensor/sensor.h"
#if MYNEWT_VAL(BUS_DRIVER_PRESENT)
#include "bus/drivers/i2c_common.h"
#endif
#include "hal/hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LPS33HW_I2C_ADDR (0x5C)

#define LPS33HW_SPI_READ_CMD_BIT (0x80)

#define LPS33HW_INT_LEVEL (0x80)
#define LPS33HW_INT_OPEN (0x40)
#define LPS33HW_INT_LATCH_EN (0x20)
#define LPS33HW_INT_RD_CLEAR (0x10)

enum lps33hw_output_data_rates {
    LPS33HW_ONE_SHOT            = 0x00,
    LPS33HW_1HZ                 = 0x01,
    LPS33HW_10HZ                = 0x02,
    LPS33HW_25HZ                = 0x03,
    LPS33HW_50HZ                = 0x04,
    LPS33HW_75HZ                = 0x05
};

enum lps33hw_low_pass_config {
    LPS33HW_LPF_DISABLED        = 0x00, /* Bandwidth = data rate / 2 */
    LPS33HW_LPF_ENABLED_LOW_BW  = 0x02, /* Bandwidth = data rate / 9 */
    LPS33HW_LPF_ENABLED_HIGH_BW = 0x03  /* Bandwidth = data rate / 20 */
};

struct lps33hw_int_cfg {
    uint8_t pin;
    unsigned int data_rdy : 1;
    unsigned int pressure_low : 1;
    unsigned int pressure_high : 1;
    unsigned int active_low : 1;
    unsigned int open_drain : 1;
    unsigned int latched : 1;
};

struct lps33hw_cfg {
    sensor_type_t mask;
    struct lps33hw_int_cfg int_cfg;
    enum lps33hw_output_data_rates data_rate;
    enum lps33hw_low_pass_config lpf;
    unsigned int autozero : 1;
    unsigned int autorifp : 1;
};

struct lps33hw_private_driver_data {
    struct sensor_read_ctx user_ctx;
};

struct lps33hw {
#if MYNEWT_VAL(BUS_DRIVER_PRESENT)
    union {
        struct bus_i2c_node i2c_node;
        struct bus_spi_node spi_node;
    };
#else
    struct os_dev dev;
#endif
    struct sensor sensor;
    struct lps33hw_cfg cfg;
    os_time_t last_read_time;
    struct lps33hw_private_driver_data pdd;
#if MYNEWT_VAL(BUS_DRIVER_PRESENT)
    bool node_is_spi;
#endif
#if MYNEWT_VAL(LPS33HW_ONE_SHOT_MODE)
    sensor_type_t type;
    sensor_data_func_t data_func;
    struct os_callout lps33hw_one_shot_read;
#endif
};

/**
 * Set the output data rate.
 *
 * @param The interface object associated with the lps33hw.
 * @param Data rate value.
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_set_data_rate(struct sensor_itf *itf,
    enum lps33hw_output_data_rates rate);

/**
 * Configure the Low Pass Filter.
 *
 * @param The interface object associated with the lps33hw.
 * @param Low pass filter config value
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_set_lpf(struct sensor_itf *itf,
    enum lps33hw_low_pass_config lpf);

/**
 * Software reset.
 *
 * @param Ptr to the sensor
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_reset(struct sensor *sensor);

/*
 * Get pressure.
 *
 * @param The interface object associated with the lps33hw.
 * @param Pressure.
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_get_pressure(struct sensor_itf *itf, float *pressure);

/*
 * Get temperature.
 *
 * @param The interface object associated with the lps33hw.
 * @param Temperature.
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_get_temperature(struct sensor_itf *itf, float *temperature);

/*
 * Set pressure reference.
 *
 * @param The interface object associated with the lps33hw.
 * @param The pressure reference.
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_set_reference(struct sensor_itf *itf, float reference);

/*
 * Set pressure threshold.
 *
 * @param The interface object associated with the lps33hw.
 * @param The pressure reference.
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_set_threshold(struct sensor_itf *itf, float threshold);

/*
 * Set pressure RPDS.
 *
 * @param The interface object associated with the lps33hw.
 * @param The pressure offset, unsure of units.
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_set_rpds(struct sensor_itf *itf, uint16_t rpds);

/**
 * Initialise gpio interrupt and setup handler. Call lp33hw_config_interrupt
 * first
 *
 * @param The sensor object associated with this lps33hw.
 * @param Handler, called back in the ISR.
 * @param Argument to be passed to the handler.
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_enable_interrupt(struct sensor *sensor,
    hal_gpio_irq_handler_t handler, void * arg);

/**
 * Disable gpio interrupt.
 *
 * @param The sensor object associated with this lps33hw.
 *
 * @return 0 on success, non-zero error on failure.
 */
void lps33hw_disable_interrupt(struct sensor *sensor);

/**
 * Configure interrupt.
 *
 * @param The device object associated with this lps33hw.
 * @param Interrupt config struct
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_config_interrupt(struct sensor *sensor, struct lps33hw_int_cfg cfg);

/**
 * Expects to be called back through os_dev_create().
 *
 * @param The device object associated with this pressure sensor
 * @param Argument passed to OS device init, unused
 *
 * @return 0 on success, non-zero error on failure.
 */
int lps33hw_init(struct os_dev *, void *);
int lps33hw_config(struct lps33hw *, struct lps33hw_cfg *);

#if MYNEWT_VAL(LPS33HW_CLI)
int lps33hw_shell_init(void);
#endif

#if MYNEWT_VAL(BUS_DRIVER_PRESENT)
/**
 * Create I2C bus node for LPS33HW sensor
 *
 * @param node        Bus node
 * @param name        Device name
 * @param i2c_cfg     I2C node configuration
 * @param sensor_itf  Sensors interface
 *
 * @return 0 on success, non-zero on failure
 */
int
lps33hw_create_i2c_sensor_dev(struct bus_i2c_node *node, const char *name,
                              const struct bus_i2c_node_cfg *i2c_cfg,
                              struct sensor_itf *sensor_itf);

/**
 * Create SPI bus node for LPS33HW sensor
 *
 * @param node        Bus node
 * @param name        Device name
 * @param spi_cfg     SPI node configuration
 * @param sensor_itf  Sensors interface
 *
 * @return 0 on success, non-zero on failure
 */
int
lps33hw_create_spi_sensor_dev(struct bus_spi_node *node, const char *name,
                              const struct bus_spi_node_cfg *spi_cfg,
                              struct sensor_itf *sensor_itf);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_LPS33HW_H__ */
