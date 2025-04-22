#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hal_msp.h"
#include "error_handling.h"
#include <i2c/i2c.h>
#include <vl6180x/vl6180x.h>
#include <sdp810/sdp810.h>
#include <ads7138/ads7138.h>
#include <sample_timer/sample_timer.h>
#include <common/manikin_bit_manipulation.h>
#include <sample_timer/sample_timer.h>

volatile uint8_t sensor_timer_1_trigger;
volatile uint8_t sensor_timer_2_trigger;
volatile uint8_t sensor_timer_3_trigger;

manikin_sensor_ctx_t sensor1_ctx;

manikin_sensor_ctx_t sensor2_ctx;

sample_timer_ctx_t timer1_ctx;
sample_timer_ctx_t timer2_ctx;

void
sample_irq (TIM_TypeDef *tim)
{
    if (tim == TIM2)
    {
        sensor_timer_1_trigger = 1;
    }
    else if (tim == TIM3)
    {
        sensor_timer_2_trigger = 1;
    }
}
manikin_status_t
init_gpio_for_sensors ()
{
    /*Configure GPIO pin : PB10 & PB11 */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin       = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin       = GPIO_PIN_1;
    GPIO_InitStruct.Alternate = 0;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_Delay(100);
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();
    return MANIKIN_STATUS_OK;
}

manikin_status_t
init_i2c_sensor1 ()
{
    sensor_timer_1_trigger = 0;
    manikin_i2c_init(I2C2, MANIKIN_I2C_SPEED_400KHz);
    sensor1_ctx.i2c         = I2C2;
    sensor1_ctx.i2c_addr    = 0x29;
    timer1_ctx.frequency    = 100;
    timer1_ctx.timer        = TIM2;
    timer1_ctx.watchdog     = WWDG;
    manikin_status_t status = vl6180x_init_sensor(&sensor1_ctx);
    if (status == MANIKIN_STATUS_OK)
    {
        sample_timer_init(&timer1_ctx);
        sample_timer_start(&timer1_ctx);
    }
    return MANIKIN_STATUS_OK;
}

manikin_status_t
init_i2c_sensor2 ()
{
    manikin_i2c_init(I2C1, MANIKIN_I2C_SPEED_400KHz);
    sensor2_ctx.i2c         = I2C1;
    sensor2_ctx.i2c_addr    = 0x10;
    timer2_ctx.frequency    = 50;
    timer2_ctx.timer        = TIM3;
    timer2_ctx.watchdog     = WWDG;
    manikin_status_t status = ads7138_init_sensor(&sensor2_ctx);
    if (status == MANIKIN_STATUS_OK)
    {
        sample_timer_init(&timer2_ctx);
        sample_timer_start(&timer2_ctx);
    }
    sensor_timer_2_trigger = 0;
    return MANIKIN_STATUS_OK;
}

manikin_status_t
init_i2c_sensor3 ()
{
    sensor_timer_3_trigger = 0;
    return MANIKIN_STATUS_OK;
}

manikin_status_t
check_and_sample_sensor1 (uint8_t *data_buf, size_t data_buf_size)
{
    if (sensor_timer_1_trigger)
    {
        manikin_status_t status
            = sample_timer_start_cb_handler(&timer1_ctx, &sensor1_ctx);
        if (status == MANIKIN_STATUS_OK)
        {
            status = vl6180x_read_sensor(&sensor1_ctx, (uint8_t *)data_buf);
        }
        sample_timer_end_cb_handler(&timer1_ctx, &sensor1_ctx, status);
        sensor_timer_1_trigger = 0;
    }
    return MANIKIN_STATUS_OK;
}

manikin_status_t
check_and_sample_sensor2 (uint8_t *data_buf, size_t data_buf_size)
{
    if (sensor_timer_2_trigger)
    {
        manikin_status_t status
            = sample_timer_start_cb_handler(&timer2_ctx, &sensor2_ctx);
        if (status == MANIKIN_STATUS_OK)
        {
            status = ads7138_read_sensor(&sensor2_ctx, data_buf);
        }
        sample_timer_end_cb_handler(&timer2_ctx, &sensor2_ctx, status);
        sensor_timer_2_trigger = 0;
    }
    return MANIKIN_STATUS_OK;
}

manikin_status_t
check_and_sample_sensor3 (uint8_t *data_buf, size_t data_buf_size)
{
    return MANIKIN_STATUS_OK;
}