#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include <stm32f4xx_hal.h>

HAL_StatusTypeDef HAL_InitTick( uint32_t TickPriority )
{
    ( void ) TickPriority;
    ( void ) SysTick_Config( SystemCoreClock / 1000 );
    return HAL_OK;
}


void HAL_Delay( uint32_t ulDelayMs )
{
    if( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED )
    {
        vTaskDelay( pdMS_TO_TICKS( ulDelayMs ) );
    }
    else
    {
        uint32_t ulStartTick = HAL_GetTick();
        uint32_t ulTicksWaited = ulDelayMs;

        /* Add a freq to guarantee minimum wait */
        if( ulTicksWaited < HAL_MAX_DELAY )
        {
            ulTicksWaited += ( uint32_t ) ( HAL_GetTickFreq() );
        }

        while( ( HAL_GetTick() - ulStartTick ) < ulTicksWaited )
        {
            __NOP();
        }
    }
}