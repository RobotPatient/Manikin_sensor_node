#include <spi/spi.h>
#include <w25qxx128/w25qxx128.h>
#include <stm32f4xx_hal.h>
uint8_t test_tx_buf[1024];
uint8_t test_rx_buf[1024];

void
init_spi_flash_memory ()
{
    GPIO_InitTypeDef spi_gpio;
    spi_gpio.Pin       = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    spi_gpio.Mode      = GPIO_MODE_AF_PP;
    spi_gpio.Pull      = GPIO_PULLUP;
    spi_gpio.Alternate = GPIO_AF6_SPI3;
    spi_gpio.Speed     = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOC, &spi_gpio); 
    spi_gpio.Pin = GPIO_PIN_3;
    spi_gpio.Pull = GPIO_NOPULL;
    spi_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOC, &spi_gpio); 
    manikin_spi_memory_ctx_t mem_ctx;
    manikin_spi_cs_t cs_pin = {.port= GPIOC, .pin=GPIO_PIN_3};
    mem_ctx.mem_size = 16;
    mem_ctx.spi = SPI3;
    mem_ctx.spi_cs = cs_pin;
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_SPI3_CLK_ENABLE();
    manikin_spi_init(SPI3, (10*10e6));
    w25qxx_init(&mem_ctx);
    HAL_Delay(100);
    w25qxx_erase_sector(&mem_ctx, 0);
    HAL_Delay(1000);
    w25qxx_erase_sector(&mem_ctx, 1);
    HAL_Delay(1000);
    int i = 0;
    while(i < 1023){
    test_tx_buf[i++] = 0x50;
    test_tx_buf[i++] = 0x10;
    }
    w25qxx_write(&mem_ctx, test_tx_buf, 5, 1024);
    HAL_Delay(450);
    w25qxx_read(&mem_ctx, test_rx_buf, 5, 1024);
}