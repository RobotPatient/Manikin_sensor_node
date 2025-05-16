#include <manikin_timer_functions.h>
#include <stm32f4xx_hal.h>

int
timer_hal_init (manikin_timer_inst_t timer_inst, uint32_t freq)
{
    TIM_HandleTypeDef timer_handle;
    uint32_t timer_period    = (uint32_t)(SystemCoreClock / (freq * 2)) - 1;
    timer_handle.Instance    = timer_inst;
    timer_handle.Init.Period = timer_period;
    timer_handle.Init.RepetitionCounter = 0;
    timer_handle.Init.Prescaler         = 0;
    timer_handle.Init.ClockDivision     = 0;
    timer_handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    timer_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (timer_inst == TIM2)
    {
        NVIC_EnableIRQ(TIM2_IRQn);
        NVIC_SetPriority(TIM2_IRQn, 0);
    }
    else if (timer_inst == TIM3)
    {
        NVIC_EnableIRQ(TIM3_IRQn);
        NVIC_SetPriority(TIM3_IRQn, 0);
    }
    if (HAL_TIM_Base_Init(&timer_handle) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

int
timer_hal_start (manikin_timer_inst_t timer_inst)
{
    timer_inst->DIER |= TIM_DIER_UIE;
    timer_inst->SR |= TIM_CR1_CEN;
    return 0;
}

int
timer_hal_stop (manikin_timer_inst_t timer_inst)
{
    timer_inst->SR &= ~TIM_CR1_CEN;
    return 0;
}

int
timer_hal_deinit (manikin_timer_inst_t timer_inst)
{
    return 0;
}

size_t
timer_hal_get_tick ()
{
    return 0;
}