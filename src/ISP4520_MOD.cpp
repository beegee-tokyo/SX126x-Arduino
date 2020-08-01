#ifdef NRF52_SERIES
#include <variant.h>
#ifdef _VARIANT_ISP4520_
#include <SPI.h>
SPIClass SPI_LORA(NRF_SPIM2, 25, 23, 26);
#endif
#endif