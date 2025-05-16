#include <FreeRTOS.h>
#include <task.h>

void vApplicationTickHook(void)
{
}

void vApplicationIdleHook(void)
{
}

void vApplicationMallocFailedHook(void)
{
  taskDISABLE_INTERRUPTS();
  for (;;)
    ;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
  (void)pcTaskName;
  (void)pxTask;

  taskDISABLE_INTERRUPTS();
  for (;;)
    ;
}