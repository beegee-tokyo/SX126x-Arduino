/**
 * @file SX126x-Arduino.h
 * 
 * @author Bernd Giesecke
 * @author Insight SIP
 * @author SEMTECH S.A.
 * 
 * Arduino library for LoRa communication with Semtech SX126x chips. 
 * It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32, 
 * ESP8266 and nRF52832. It will not work with other uC's like AVR.
 *
 * \example   DeepSleep\DeepSleep.ino
 * \example   PingPong\PingPong.ino
 * \example   Sensor-Gateway-Deepsleep\LoRa-Gateway\src\main.cpp
 * \example   Sensor-Gateway-Deepsleep\LoRa-TempSensor\src\main.cpp
 */
#ifndef _SX126X_ARDUINO_H
#define _SX126X_ARDUINO_H

#include "boards/mcu/board.h"
#include "radio/radio.h"

#ifdef NRF52_SERIES
#include <SPI.h>
extern SPIClass SPI_LORA;
#endif

#endif // _SX126X_ARDUINO_H