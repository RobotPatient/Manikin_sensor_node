#include "board_conf.h"
#include "session_mgmt/session_mgmt.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hal_msp.h"
#include <common/manikin_bit_manipulation.h>
#include <cstdint>
#include "sampling.h"
#include "stm32f4xx_hal.h"
#include "flash.h"
#include "hal_msp.h"
#include "can_wrapper.h"
#include "cli.h"
#include "SEGGER_RTT.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_gpio.h"
#include "isotp.h"

IsoTpLink g_link;
IsoTpLink g_link2;
IsoTpLink comm_link;

TIM_HandleTypeDef htim5;

void MX_TIM5_Init(void)
{
    __HAL_RCC_TIM5_CLK_ENABLE();

    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 9; // 168 MHz / (9+1) = 16.8 MHz
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 0xFFFFFFFF; // Free-running
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&htim5);
    HAL_TIM_Base_Start(&htim5);
}

int
main (void)
{
    SystemClock_Config();
    HAL_Init();
    BOARD_GPIO_Init();
    SEGGER_RTT_Init();
    manikin_cli_init();
    MX_TIM5_Init();
    MX_USB_DEVICE_Init();
    session_mgmt_init();
    init_spi_flash_memory();
    init_peripherals_for_sensors();
    HAL_Delay(1);
    init_can();
    uint8_t  read_buf[16];
    #if BOARD_CONF_USE_SENSOR2
    uint8_t  read_buf2[16];
    #endif
    uint8_t  comm_buf[128];
    uint32_t recv_size;
    uint16_t count = 0;
    while (1)
    {
        int ret = isotp_receive(&comm_link, comm_buf, sizeof(comm_buf), &recv_size);
        if (ISOTP_RET_OK == ret) {
            /* Handle received message */
            SEGGER_RTT_printf(0, "Received %d bytes:\n", recv_size);
            SEGGER_RTT_printf(0, "%s\n", comm_buf);
            session_mgmt_on_can_msg(comm_buf, recv_size);
            
        }
        check_and_sample_sensor1(read_buf);
        #if BOARD_CONF_USE_SENSOR2
        check_and_sample_sensor2(read_buf2);
        #endif
        if (count < 5)
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
        #if BOARD_CONF_USE_SENSOR2
        isotp_poll(&g_link2);
        #endif
        isotp_poll(&comm_link);
        __WFI();
    }
}
