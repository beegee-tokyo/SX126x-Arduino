#ifdef NRF52_SERIES
// #include "boards/mcu/nrf52832/SPI.h"
#include "boards/mcu/board.h"
#include <SPI.h>

// SPIClass SPI_LORA(NRF_SPIM2, PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_MOSI);
extern SPIClass SPI_LORA;

void initSPI(void)
{
	SPI_LORA.begin();
}
#endif