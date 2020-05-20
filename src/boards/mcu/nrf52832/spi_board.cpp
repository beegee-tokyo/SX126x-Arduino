#ifdef NRF52_SERIES
// #include "boards/mcu/nrf52832/SPI.h"
#include "boards/mcu/board.h"
#include <SPI.h>

// SPIClass SPI_LORA(NRF_SPIM2, PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_MOSI);
extern SPIClass SPI_LORA;

// _hwConfig.PIN_LORA_SCLK, _hwConfig.PIN_LORA_MISO, _hwConfig.PIN_LORA_MOSI, _hwConfig.PIN_LORA_NSS

void initSPI(void)
{
	// SPI_LORA.begin(NRF_SPIM2, _hwConfig.PIN_LORA_MISO, _hwConfig.PIN_LORA_SCLK, _hwConfig.PIN_LORA_MOSI);
	SPI_LORA.begin();
}
#endif