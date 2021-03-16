#if defined ESP8266 || defined ESP32
#include <SPI.h>
#include "boards/mcu/board.h"

SPIClass SPI_LORA;

void initSPI(void)
{
#ifdef ESP8266
	SPI_LORA.pins(_hwConfig.PIN_LORA_SCLK, _hwConfig.PIN_LORA_MISO, _hwConfig.PIN_LORA_MOSI, _hwConfig.PIN_LORA_NSS);
	SPI_LORA.begin();
	SPI_LORA.setHwCs(false);
#else
	SPI_LORA.begin(_hwConfig.PIN_LORA_SCLK, _hwConfig.PIN_LORA_MISO, _hwConfig.PIN_LORA_MOSI, _hwConfig.PIN_LORA_NSS);
#endif
}
#endif