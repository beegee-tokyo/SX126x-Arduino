// #define ARDUINO_ARCH_RP2040
#if defined ARDUINO_RAKWIRELESS_RAK11300
#include <SPI.h>
#include "boards/mcu/board.h"

SPIClassRP2040 SPI_LORA(spi1, _hwConfig.PIN_LORA_MISO, _hwConfig.PIN_LORA_NSS, _hwConfig.PIN_LORA_SCLK, _hwConfig.PIN_LORA_MOSI);

void initSPI(void)
{
	SPI_LORA.setRX(_hwConfig.PIN_LORA_MISO);
	SPI_LORA.setCS(_hwConfig.PIN_LORA_NSS);
	SPI_LORA.setSCK(_hwConfig.PIN_LORA_SCLK);
	SPI_LORA.setTX(_hwConfig.PIN_LORA_MOSI);
	SPI_LORA.begin();
}
#endif // ARDUINO_RAKWIRELESS_RAK11300
