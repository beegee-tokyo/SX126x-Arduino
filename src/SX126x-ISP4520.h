#ifdef NRF52_SERIES
#include "SX126x-Arduino.h"
#include <SPI.h>
SPIClass SPI_LORA(NRF_SPIM2, 25, 23, 26);
#endif
