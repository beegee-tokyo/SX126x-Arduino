#ifdef NRF52_SERIES
#include "LoRaWan-Arduino.h"
#include <SPI.h>
SPIClass SPI_LORA(NRF_SPIM2, 45, 43, 44);
#endif
