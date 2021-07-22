/**
 * @file LoRaWan-Arduino.h
 * 
 * @author Bernd Giesecke
 * @author Insight SIP
 * @author SEMTECH S.A.
 * 
 * Arduino library for LoRa communication with Semtech SX126x chips. 
 * It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32, 
 * ESP8266 and nRF52832. It will not work with other uC's like AVR.
 */
#ifndef _LORAWAN_ARDUINO_H
#define _LORAWAN_ARDUINO_H

#include "boards/mcu/board.h"
#include "radio/radio.h"
#include "mac/LoRaMacHelper.h"

#ifdef NRF52_SERIES
#include <SPI.h>
extern SPIClass SPI_LORA;
#endif

#endif // _LORAWAN_ARDUINO_H