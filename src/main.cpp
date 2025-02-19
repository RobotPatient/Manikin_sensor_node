#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hal_msp.h"
#include "error_handling.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2c_helper.hpp"
#include "sensor_compression.hpp"

#define BLINK_TASK_PRIO 2
#define USB_TASK_PRIO 3

void usb_print_task(void *pvArg)
{
  /* Test message to print */
  uint8_t *data = (uint8_t *)"\rHello World from USB CDC\n";
  /* USB must be initialized within task! */
  MX_USB_DEVICE_Init();
  while (1)
  {
    CDC_Transmit_FS(data, strlen((char *)data));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void blink_task(void *pvArg)
{
  /* Initialize the on-board led */
  BOARD_GPIO_Init();
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
SensorData data;
void i2c_poll_task(void *pvArg)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
     /*Configure GPIO pin : PB10 & PB11 */
     GPIO_InitTypeDef GPIO_InitStruct;
     GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
     GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
     GPIO_InitStruct.Pull = GPIO_NOPULL;
     GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
     __HAL_RCC_I2C2_CLK_ENABLE();
     static I2C_HandleTypeDef hi2c2;
     hi2c2.Init.ClockSpeed = 100000;
     hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
     hi2c2.Init.OwnAddress1 = 0;
     hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
     hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
     hi2c2.Init.OwnAddress2 = 0;
     hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
     hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
     hi2c2.Instance = I2C2;
     I2CDriver driver(&hi2c2, kI2cSpeed_100KHz);
     driver.Init();
     CompressionSensor compsens;
     compsens.Initialize(&driver);
  while (1)
  {
    data = compsens.GetSensorData();
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Create both the blink and usb print task */
  if (xTaskCreate(blink_task, "Blinky", configMINIMAL_STACK_SIZE, NULL, BLINK_TASK_PRIO, NULL) != pdPASS)
  {
    Error_Handler();
  }

  if (xTaskCreate(usb_print_task, "USB", configMINIMAL_STACK_SIZE, NULL, USB_TASK_PRIO, NULL) != pdPASS)
  {
    Error_Handler();
  }

  if (xTaskCreate(i2c_poll_task, "Sens", configMINIMAL_STACK_SIZE, NULL, BLINK_TASK_PRIO, NULL) != pdPASS)
  {
    Error_Handler();
  }
  vTaskStartScheduler();

  while (1)
  {
  }
}
