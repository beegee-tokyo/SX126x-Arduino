#ifndef _SPI_BOARD_H
#define _SPI_BOARD_H
#if defined ESP8266 || defined ESP32
#include "boards/mcu/espressif/spi_board.h"
#endif
#ifdef NRF52_SERIES
#include "boards/mcu/nrf52832/spi_board.h"
#endif
#endif