// #define ARDUINO_ARCH_RP2040
#if defined ARDUINO_ARCH_RP2040 && not defined ARDUINO_RAKWIRELESS_RAK11300
#include <SPI.h>
#include "boards/mcu/board.h"

// SPIClass SPI_LORA = &SPI;
MbedSPI SPI_LORA(12, 11, 10);

void initSPI(void)
{
	// SPI_LORA.setRX(_hwConfig.PIN_LORA_MISO);
	// SPI_LORA.setCS(_hwConfig.PIN_LORA_NSS);
	// SPI_LORA.setSCK(_hwConfig.PIN_LORA_SCLK);
	// SPI_LORA.setTX(_hwConfig.PIN_LORA_MOSI);
	SPI_LORA.begin();
}
#endif