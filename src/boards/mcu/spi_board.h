#ifndef _SPI_BOARD_H
#define _SPI_BOARD_H
#if defined ESP8266 || defined ESP32
#include "boards/mcu/espressif/spi_board.h"
#elif defined(NRF52_SERIES)
#include "boards/mcu/nrf52832/spi_board.h"
#elif defined(ARDUINO_RAKWIRELESS_RAK11300)
#include "boards/mcu/rak11300/spi_board.h"
#elif defined(ARDUINO_ARCH_RP2040)
#include "boards/mcu/rp2040/spi_board.h"
#else
#pragma error "Board not supported"
#endif
#endif