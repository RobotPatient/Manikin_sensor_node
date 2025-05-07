#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hal_msp.h"
#include "error_handling.h"
#include <common/manikin_bit_manipulation.h>
#include "sampling.h"
#include "stm32f405xx.h"
#include "stm32f4xx_hal.h"
#include "flash.h"
#include "can_wrapper.h"
#include "SEGGER_RTT.h"

uint16_t count = 0;

void
background_transmit_task (uint8_t *read_buf1, uint8_t *read_buf2)
{
    count = 0;
    uint16_t temp[8];
    uint8_t  streambuffer[128];
    for (int i = 0; i < 8; i++)
    {
        temp[i] = ((uint16_t)read_buf2[2 * i] << 8) | read_buf2[2 * i + 1];
    }
    size_t len = snprintf((char *)streambuffer,
                          sizeof(streambuffer),
                          "\r[0]: %d\n",
                          read_buf1[0]);
    CDC_Transmit_FS(streambuffer, len);
    len = snprintf((char *)streambuffer,
                   sizeof(streambuffer),
                   "\r[0]: %d [1]: %d [2]: %d [3]: %d [4]: %d"
                   "[5]: %d [6]: %d [7]: %d\n",
                   temp[0],
                   temp[1],
                   temp[2],
                   temp[3],
                   temp[4],
                   temp[5],
                   temp[6],
                   temp[7]);
    CDC_Transmit_FS(streambuffer, len);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
}





int
main (void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    __PWR_CLK_ENABLE();
    /* Initialize the on-board led */
    BOARD_GPIO_Init();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    SEGGER_RTT_Init();
    MX_USB_DEVICE_Init();
    init_spi_flash_memory();
    init_gpio_for_sensors();
    init_i2c_sensor1();
    HAL_Delay(1);
    init_i2c_sensor2();
    init_can();

    uint8_t read_buf[16];
    uint8_t read_buf2[16];

    while (1)
    {
        check_and_sample_sensor1(read_buf, sizeof(read_buf));
        check_and_sample_sensor2(read_buf2, sizeof(read_buf2));
        if (count < 10)
        {
            count++;
        }
        else
        {
            // can_phy_transmit(0, read_buf, 1);
            background_transmit_task(read_buf, read_buf2);
        }
        __WFI();
    }
}
