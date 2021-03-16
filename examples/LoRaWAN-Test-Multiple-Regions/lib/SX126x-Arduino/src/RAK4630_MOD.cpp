#ifdef NRF52_SERIES
#include <variant.h>
#ifdef _VARIANT_RAK4630_
#include <SPI.h>
SPIClass SPI_LORA(NRF_SPIM2, 45, 43, 44);
#endif
#endif