#include <stm32f4xx_hal.h>
#include <i2c/i2c.h>
#include "board_conf.h"
#include "common/manikin_types.h"
#include <sample_timer/sample_timer.h>
#include <common/manikin_bit_manipulation.h>
#include <sample_timer/sample_timer.h>

volatile uint8_t sensor_timer_1_trigger;
volatile uint8_t sensor_timer_2_trigger;
#if BOARD_CONF_USE_SENSOR3
volatile uint8_t sensor_timer_3_trigger;
#endif

manikin_sensor_ctx_t sensor1_ctx;
manikin_sensor_ctx_t sensor2_ctx;

sample_timer_ctx_t timer1_ctx;
sample_timer_ctx_t timer2_ctx;

void
sample_irq (TIM_TypeDef *tim)
{
#if BOARD_CONF_USE_SENSOR1
    if (tim == BOARD_CONF_TIMER_SENSOR_1)
    {
        sensor_timer_1_trigger = 1;
    }
#endif

#if BOARD_CONF_USE_SENSOR2
    if (tim == BOARD_CONF_TIMER_SENSOR_2)
    {
        sensor_timer_2_trigger = 1;
    }
#endif
}

static manikin_status_t
init_i2c0_pins ()
{
    GPIO_InitTypeDef pin_init;
    pin_init.Pin       = BOARD_CONF_I2C0_SDA_PIN;
    pin_init.Mode      = GPIO_MODE_AF_OD;
    pin_init.Pull      = GPIO_NOPULL;
    pin_init.Alternate = BOARD_CONF_I2C0_SDA_PIN_MUX;
    pin_init.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(BOARD_CONF_I2C0_SDA_PORT, &pin_init);

    pin_init.Pin       = BOARD_CONF_I2C0_SCL_PIN;
    pin_init.Alternate = BOARD_CONF_I2C0_SCL_PIN_MUX;
    HAL_GPIO_Init(BOARD_CONF_I2C0_SCL_PORT, &pin_init);

    pin_init.Pin       = BOARD_CONF_SENSOR1_RESET_PIN;
    pin_init.Alternate = 0;
    pin_init.Mode      = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(BOARD_CONF_SENSOR1_RESET_PORT, &pin_init);
    HAL_GPIO_WritePin(BOARD_CONF_SENSOR1_RESET_PORT,
                      BOARD_CONF_SENSOR1_RESET_PIN,
                      GPIO_PIN_SET);

    BOARD_CONF_I2C0_CLK_EN();
    return MANIKIN_STATUS_OK;
}

static manikin_status_t
init_i2c1_pins ()
{
    GPIO_InitTypeDef pin_init;
    pin_init.Pin       = BOARD_CONF_I2C1_SDA_PIN;
    pin_init.Mode      = GPIO_MODE_AF_OD;
    pin_init.Pull      = GPIO_NOPULL;
    pin_init.Alternate = BOARD_CONF_I2C1_SDA_PIN_MUX;
    pin_init.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(BOARD_CONF_I2C1_SDA_PORT, &pin_init);

    pin_init.Pin       = BOARD_CONF_I2C1_SCL_PIN;
    pin_init.Alternate = BOARD_CONF_I2C1_SCL_PIN_MUX;
    HAL_GPIO_Init(BOARD_CONF_I2C1_SCL_PORT, &pin_init);

    return MANIKIN_STATUS_OK;
}

static manikin_status_t
init_i2c_sensor1 ()
{
    sensor_timer_1_trigger = 0;
    manikin_i2c_init(BOARD_CONF_I2C0_INSTANCE, BOARD_CONF_I2C0_SPEED);
    sensor1_ctx.i2c         = BOARD_CONF_I2C0_INSTANCE;
    sensor1_ctx.i2c_addr    = BOARD_CONF_SENSOR1_ADDR;
    timer1_ctx.frequency    = BOARD_CONF_SENSOR1_SAMPLE_RATE_HZ;
    timer1_ctx.timer        = BOARD_CONF_TIMER_SENSOR_1;
    timer1_ctx.watchdog     = WWDG;
    manikin_status_t status = BOARD_CONF_SENSOR1_INIT(&sensor1_ctx);
    if (status == MANIKIN_STATUS_OK)
    {
        sample_timer_init(&timer1_ctx);
        sample_timer_start(&timer1_ctx);
    }
    sensor_timer_1_trigger = 0;
    return MANIKIN_STATUS_OK;
}

static manikin_status_t
init_i2c_sensor2 ()
{
    manikin_i2c_init(BOARD_CONF_I2C1_INSTANCE, BOARD_CONF_I2C1_SPEED);
    sensor2_ctx.i2c         = BOARD_CONF_I2C1_INSTANCE;
    sensor2_ctx.i2c_addr    = BOARD_CONF_SENSOR2_ADDR;
    timer2_ctx.frequency    = BOARD_CONF_SENSOR2_SAMPLE_RATE_HZ;
    timer2_ctx.timer        = BOARD_CONF_TIMER_SENSOR_2;
    timer2_ctx.watchdog     = WWDG;
    manikin_status_t status = BOARD_CONF_SENSOR2_INIT(&sensor2_ctx);
    if (status == MANIKIN_STATUS_OK)
    {
        sample_timer_init(&timer2_ctx);
        sample_timer_start(&timer2_ctx);
    }
    sensor_timer_2_trigger = 0;
    return MANIKIN_STATUS_OK;
}

#if BOARD_CONF_USE_SENSOR3
static manikin_status_t
init_i2c_sensor3 ()
{
    sensor_timer_3_trigger = 0;
    return MANIKIN_STATUS_OK;
}
#endif

manikin_status_t
init_peripherals_for_sensors ()
{
#if BOARD_CONF_USE_SENSOR1
    __HAL_RCC_GPIOB_CLK_ENABLE();
    init_i2c0_pins();
    BOARD_CONF_I2C0_CLK_EN();
    BOARD_CONF_TIMER_SENSOR_1_EN();
    init_i2c_sensor1();
#endif

#if BOARD_CONF_USE_SENSOR2
    init_i2c1_pins();
    BOARD_CONF_I2C1_CLK_EN();
    BOARD_CONF_TIMER_SENSOR_2_EN();
    init_i2c_sensor2();
#endif

    return MANIKIN_STATUS_OK;
}

manikin_status_t
check_and_sample_sensor1 (uint8_t *data_buf)
{
    if (sensor_timer_1_trigger)
    {
        manikin_status_t status
            = sample_timer_start_cb_handler(&timer1_ctx, &sensor1_ctx);
        if (status == MANIKIN_STATUS_OK)
        {
            status
                = BOARD_CONF_SENSOR1_SAMPLE(&sensor1_ctx, (uint8_t *)data_buf);
        }
        sample_timer_end_cb_handler(&timer1_ctx, &sensor1_ctx, status);
        sensor_timer_1_trigger = 0;
    }
    return MANIKIN_STATUS_OK;
}

manikin_status_t
check_and_sample_sensor2 (uint8_t *data_buf)
{
    if (sensor_timer_2_trigger)
    {
        manikin_status_t status
            = sample_timer_start_cb_handler(&timer2_ctx, &sensor2_ctx);
        if (status == MANIKIN_STATUS_OK)
        {
            status = BOARD_CONF_SENSOR2_SAMPLE(&sensor2_ctx, data_buf);
        }
        sample_timer_end_cb_handler(&timer2_ctx, &sensor2_ctx, status);
        sensor_timer_2_trigger = 0;
    }
    return MANIKIN_STATUS_OK;
}

manikin_status_t
check_and_sample_sensor3 (uint8_t *data_buf)
{
    return MANIKIN_STATUS_OK;
}