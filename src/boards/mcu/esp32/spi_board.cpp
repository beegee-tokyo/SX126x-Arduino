#ifdef ESP32
#include <SPI.h>
#include "boards/mcu/board.h"

SPIClass SPI_LORA;

void initSPI(void)
{
    SPI_LORA.begin(_hwConfig.PIN_LORA_SCLK, _hwConfig.PIN_LORA_MISO, _hwConfig.PIN_LORA_MOSI, _hwConfig.PIN_LORA_NSS);
}
#endif