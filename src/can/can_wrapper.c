#include "can_wrapper.h"
#include <stdio.h>
#include "board_conf.h"
#include "common/manikin_types.h"
#include "stm32f405xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_gpio_ex.h"

CAN_HandleTypeDef can_handle;
CAN_FilterTypeDef can_filter;

CAN_RxHeaderTypeDef RxHeader;
uint32_t            TxMailbox;

int
can_phy_hal_init_can_mcu (const uint32_t speed)
{
    can_handle.Instance                  = BOARD_CONF_CAN_INSTANCE;
    can_handle.Init.Prescaler            = 6;
    can_handle.Init.Mode                 = CAN_MODE_NORMAL;
    can_handle.Init.SyncJumpWidth        = CAN_SJW_1TQ;
    can_handle.Init.TimeSeg1             = CAN_BS1_4TQ;
    can_handle.Init.TimeSeg2             = CAN_BS2_3TQ;
    can_handle.Init.TimeTriggeredMode    = DISABLE;
    can_handle.Init.AutoBusOff           = DISABLE;
    can_handle.Init.AutoWakeUp           = DISABLE;
    can_handle.Init.AutoRetransmission   = DISABLE;
    can_handle.Init.ReceiveFifoLocked    = DISABLE;
    can_handle.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&can_handle) != HAL_OK)
    {
        return 1;
    }

    if (HAL_CAN_Start(&(can_handle)) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

int
can_phy_hal_set_filter ()
{

    // configs[can_num].can_filter.FilterBank           = 0;
    // configs[can_num].can_filter.FilterMode           = CAN_FILTERMODE_IDLIST;
    // configs[can_num].can_filter.FilterScale          = CAN_FILTERSCALE_32BIT;

    // // Accept exactly ID 0 and ID 1 (Standard IDs shifted left by 5 bits)

    // configs[can_num].can_filter.FilterIdHigh         = 0x0000; // ID 0 << 5
    // configs[can_num].can_filter.FilterIdLow          = 0x0020; // ID 1 << 5

    // // In IDLIST mode, FilterMask fields are used to specify second pair of
    // IDs (only if 32bit scale) configs[can_num].can_filter.FilterMaskIdHigh =
    // 0x0000; // Not used configs[can_num].can_filter.FilterMaskIdLow      =
    // 0x0000; // Not used

    // configs[can_num].can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    // configs[can_num].can_filter.FilterActivation     = ENABLE;

    // HAL_CAN_ActivateNotification(&(configs[can_num].can_handle),
    // CAN_IT_RX_FIFO0_MSG_PENDING);

    // if (HAL_CAN_ConfigFilter(&(configs[can_num].can_handle),
    //                          &(configs[can_num].can_filter))
    //     != HAL_OK)
    // {
    //     return 1;
    // }
    return 0;
}

void
HAL_CAN_RxFifo0MsgPendingCallback (CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t             rxData[8];

    // Read the message
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK)
    {
        // Process received CAN message
        // Example: print ID and data
        printf("Received CAN ID: 0x%03X DLC: %d Data: ",
               (uint16_t)rxHeader.StdId,
               (uint16_t)rxHeader.DLC);
        for (int i = 0; i < rxHeader.DLC; i++)
        {
            printf("%02X ", rxData[i]);
        }
        printf("\n");
    }
}

void
can_phy_enable_can (uint32_t baudrate)
{
}

void
can_phy_reset_can (void)
{
    HAL_CAN_Init(&can_handle);
    HAL_CAN_Start(&can_handle);
}

void
can_phy_close_can (void)
{
    HAL_CAN_Stop(&can_handle);
}

size_t
can_phy_transmit (uint8_t *data, size_t len)
{
    CAN_TxHeaderTypeDef TxHeader;
    TxHeader.DLC   = len; // Data length
    TxHeader.IDE   = CAN_ID_STD;
    TxHeader.RTR   = CAN_RTR_DATA;
    TxHeader.StdId = 0x321; // Standard ID

    if (HAL_CAN_AddTxMessage(&can_handle, &TxHeader, data, &TxMailbox)
        != HAL_OK)
    {
        return 1;
    }
    return len;
}

manikin_status_t
init_can_pins ()
{
    GPIO_InitTypeDef can_gpio;
    can_gpio.Mode      = GPIO_MODE_AF_PP;
    can_gpio.Pin       = BOARD_CONF_CAN_RX_PIN;
    can_gpio.Pull      = GPIO_NOPULL;
    can_gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    can_gpio.Alternate = BOARD_CONF_CAN_RX_PIN_MUX;
    HAL_GPIO_Init(BOARD_CONF_CAN_RX_PORT, &can_gpio);

    can_gpio.Pin       = BOARD_CONF_CAN_TX_PIN;
    can_gpio.Alternate = BOARD_CONF_CAN_TX_PIN_MUX;
    HAL_GPIO_Init(BOARD_CONF_CAN_TX_PORT, &can_gpio);

    return MANIKIN_STATUS_OK;
}

void
init_can ()
{
    BOARD_CONF_CAN_GPIO_CLK_EN();
    BOARD_CONF_CAN_CLK_EN();

    init_can_pins();

    can_phy_hal_init_can_mcu(BOARD_CONF_CAN_SPEED);
}