#include "board_conf.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hal_msp.h"
#include <common/manikin_bit_manipulation.h>
#include "sampling.h"
#include "stm32f4xx_hal.h"
#include "flash.h"
#include "hal_msp.h"
#include "can_wrapper.h"
#include "cli.h"
#include "SEGGER_RTT.h"
#include "stm32f4xx_hal_gpio.h"
#include "isotp.h"

IsoTpLink g_link;

/* Alloc send and receive buffer statically in RAM */
static uint8_t g_isotpRecvBuf[128];
static uint8_t g_isotpSendBuf[128];

int
main (void)
{
    SystemClock_Config();
    HAL_Init();
    BOARD_GPIO_Init();
    SEGGER_RTT_Init();
    manikin_cli_init();
    MX_USB_DEVICE_Init();

    init_spi_flash_memory();
    init_peripherals_for_sensors();
    HAL_Delay(1);
    init_can();
    isotp_init_link(&g_link, 0x080,
        g_isotpSendBuf, sizeof(g_isotpSendBuf), 
        g_isotpRecvBuf, sizeof(g_isotpRecvBuf));
    start_sensor_sampling();
 
    uint8_t  read_buf[16];
    uint8_t  read_buf2[16];
    uint16_t count = 0;
    while (1)
    {
        check_and_sample_sensor1(read_buf);
        check_and_sample_sensor2(read_buf2);
        if (count < 2)
        {
            count++;
        }
        else
        {
            print_to_can();
            print_to_stdout();
            count = 0;
        }
        isotp_poll(&g_link);
        __WFI();
    }
}
