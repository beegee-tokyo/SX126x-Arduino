# SX126x-Arduino [![Build Status](https://github.com/beegee-tokyo/SX126x-Arduino/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/beegee-tokyo/SX126x-Arduino/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](https://beegee-tokyo.github.io/SX126x-Arduino/)
----
Arduino library for LoRa communication with Semtech SX126x chips. It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32, ESP8266 and nRF52832. It will not work with other uC's like AVR.    
LoRaWAN version: **`MAC V1.0.2`** and Regional Parameters version: **`PHY V1.0.2 REV B`**    

# _**IMPORTANT: RAK11300 module (RP2040) support is only tested with the [ArduinoCore Mbed BSP](https://github.com/arduino/ArduinoCore-mbed). It will not work with other BSP's for the Raspberry RP2040.**_

_**IMPORTANT: READ [WHAT'S NEW IN V2](./README_V2.md)**_
_**Some major changes are made in V2 of the SX126x-Arduino library:**_    
_**- The library now supports all LoRaWAN regions without re-compiling**_    
_**- The interrupt handling for SX126x IRQ's are taken into separate tasks for ESP32, nRF52 and RP2040**_    
_**This requires some code changes in your existing applications. Please read [WHAT'S NEW IN V2](./README_V2.md) to learn how to migrate your application to use SX126x-Arduino V2**_

----
## Content

|      |      |      |
| :---- | :---- | :---- |
| [General Info](#general-info) | [LoRa](#usage) | &nbsp;&nbsp;[LoRaWan](#lorawan) |
| &nbsp;&nbsp;[Based on](#based-on) | &nbsp;&nbsp;[Basic LoRa communication](#basic-lora-communication) | &nbsp;&nbsp;&nbsp;&nbsp;[LoRaWAN region definitions](#lorawan-region-definitions) |
| &nbsp;&nbsp;[Licenses](#licenses) | &nbsp;&nbsp;&nbsp;&nbsp;[HW structure definition](#hw-structure-definition) | &nbsp;&nbsp;[LoRaWan functions](#lorawan-functions)  |
| [Changelog](#changelog) | &nbsp;&nbsp;&nbsp;&nbsp;[GPIO definitions](#gpio-definitions) |  &nbsp;&nbsp;&nbsp;&nbsp;[Initialize](#initialize) |
| [Features](#features) | &nbsp;&nbsp;&nbsp;&nbsp;[Example HW configuration](#example-hw-configuration) | &nbsp;&nbsp;&nbsp;&nbsp;[Callbacks](#callbacks) |
| [Functions](#functions) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialize the LoRa HW](#initialize-the-lora-hw) | &nbsp;&nbsp;&nbsp;&nbsp;[Join](#join) |
| &nbsp;&nbsp;[Module specific setup](#module-specific-setup) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialization for specific modules](#simplified-lora-hw-initialization-for-specific-modules) | &nbsp;&nbsp;&nbsp;&nbsp;[LoRaWan single channel gateway](#lorawan-single-channel-gateway) |
| &nbsp;&nbsp;[Chip selection](#chip-selection) | &nbsp;&nbsp;&nbsp;&nbsp;[Setup the callbacks for LoRa events](#setup-the-callbacks-for-lora-events) | &nbsp;&nbsp;&nbsp;&nbsp;[Limit frequency hopping to a sub band](#limit-frequency-hopping-to-a-sub-band) |
| &nbsp;&nbsp;[LoRa parameters](#lora-parameters) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialize the radio](#initialize-the-radio) |   |
| &nbsp;&nbsp;[SPI definition](#mcu-to-sx126x-spi-definition) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialize the radio after CPU woke up from deep sleep](#initialize-the-radio-after-cpu-woke-up-from-deep-sleep) | [Examples](./examples/README.md) | 
| &nbsp;&nbsp;[TXCO and antenna control](#explanation-for-txco-and-antenna-control) | &nbsp;&nbsp;&nbsp;&nbsp;[Start listening for packets](#start-listening-for-packets) | [Installation](#installation) |

----
## General info
I stumbled over the [SX126x LoRa family](https://www.semtech.com/products/wireless-rf/lora-transceivers) in a customer project. Most of the existing Arduino libraries for Semtech's SX127x family are unfortunately not working with this new generation LoRa chip. I found a usefull base library from Insight SIP which is based on the original Semtech SX126x library and changed it to work with the ESP32.   
For now the library is tested with an [eByte E22-900M22S](http://www.ebyte.com/en/product-view-news.aspx?id=437) module connected to an ESP32 and an [Insight SIP ISP4520](https://www.insightsip.com/products/combo-smart-modules/isp4520) which combines a Nordic nRF52832 and a Semtech SX1262 in one module. It is as well tested with an [RAKwireless WisCore RAK4630](https://store.rakwireless.com/products) module    

__**Check out the example provided with this library to learn the basic functions.**__

Especially for the deep sleep support on the ESP32 check out the example DeepSleep.    
===

THIS IS WORK IN PROGRESS AND NOT ALL FUNCTIONS ARE INCLUDED NOR TESTED. USE IT AT YOUR OWN RISK!
=== 
----
### Based on    
- Semtech open source code for SX126x chips [SX126xLib](https://os.mbed.com/teams/Semtech/code/SX126xLib/)    
- Insight SIP open source code for ISP4520 module [LIBRARY - Source Code Examples](https://www.insightsip.com/fichiers_insightsip/pdf/ble/ISP4520/ISP4520_Source_Code.zip)    
----
### Licenses    
Library published under MIT license    

Semtech revised BSD license for codeparts used from Semtech S.A.   
```
--- Revised BSD License ---
Copyright (c) 2013, SEMTECH S.A.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Semtech corporation nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL SEMTECH S.A. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
----
## Changelog
[Code releases](CHANGELOG.md)
- 2023-10-01
  - Fix public/private network always public
  - Add option to restart MAC stack to change e.g. region without re-init timers
- 2023-08-27
  - Add function to reset MAC counters
- 2023-05-16
  - Fix typo in RadioTimeOnAir for FSK
  - Improve RadioTimeOnAir for FSK, thanks to _**@mikedupi**_
- 2023-04-17
  - Fix runtime problem in LoRaMacHelper, thanks to _**@avillacis**_
- 2023-01-28
  - Update examples, thanks to _**@DanielBustillos**_    
  - Cleanup RAK4630 initialization    
  - Add lmh_getConfRetries() to readback the confirmed package retry setting    
  - Make RX timeout configurable with `#define RXTIMEOUT_LORA_MAX`, thanks to _**@kisChang**_     
  - Add set default RX gain in SX126xSertRx, thanks to _**@kisChang**_
  - Allow change of TCXO control with hwConfig structure, thanks to _**@dberlin**_
  - Add confirmed failed callback if the degraded datarate becomes insufficient to transmit the payload length, thanks to _**@avillacis**_
  - Fix RX window timeouts, thanks to _**@battosai30**_
- 2022-10-11
  - Fix RP2040 timers, thanks to @kisChang
- 2022-09-01
  - Fix wrong RX2 frequency in AS923-2, AS923-3, AS923-4
- 2022-07-13
  - Fix crash when library debug is enabled in AS923 region.
- 2022-03-21
  - Fix RAK11310 timer problem
- 2022-03-03
  - Fix AS923 CFlist channel assignment bug
- 2022-02-09
  - Fix EU868 bug that enabled always all 8 channels
- 2021-12-18
  - Fix bug in lmh_datarate_set. Function did change only ADR, but did not update the datarate setting
- 2021-12-09
  - Make antenna switch control compatible with different switches
- 2021-11-26
  - Improve the examples and correct outdated information
- 2021-11-01
  - Correct handling of TX timeout in Radio callbacks
- 2021-09-20
  - Add missing declaration for lora_rak13300_init()
- 2021-08-11:
  - Correct power regulator setting for RAKwireless RAK11300 module from LDO to DCDC
- 2021-07-22:
  - Add support for RAKwireless RAK11300 module (Raspberry RP2040 + SX1262 module)
- 2021-06-30:  
  - Add callbacks for LoRaWAN TX finished (both confirmed and unconfirmed)
  - Add addional bandwidths for LoRa transmissions. _**Breaks `Radio.TimeOnAir()` for bandwidths other than BW 125, 250 and 500**_
  - Fix minor problem in CF list handling for AS923-x regions
- 2021-05-15:
  - Implement new regions AS923-2, AS923-3, AS923-4, RU864
  - Test CF list to add additionals channesl on AS923 and RU864
- 2021-04-10:
  - Add support for all LoRaWAN regions without recompilation of the code
  - Add background handling of SX126x IRQ's for better performance
  - Read [WHAT'S NEW IN V2](./README_V2.md) to migrate your application to V2
- 2021-03-10:  
  - Fix AS923 OTAA join problem
- 2021-02-26:
  - Fix join bug when first OTAA join fails
  - When OTAA join failed callback was called, following lmh_join() calls fail always 
- 2021-02-11:
  - Add callback for OTAA join failure 
- 2021-02-02:
  - Fix ADR problem
  - Fix Join problem for some Regions
  - Add some debug output
  - Add option to set node class during initialization. Defaults to CLASS_A for backward compatibility: **`lmh_error_status lmh_init(lmh_callback_t *callbacks, lmh_param_t lora_param, bool otaa, eDeviceClass class = CLASS_A);`**
- 2020-09-29:
  - Fix wrong control of antenna switch for RAK4631
  - Add option to control power of antenna switch by the library with **`_hwConfig.USE_RXEN_ANT_PWR`**
- 2020-08-01:
  - Fixed linker error when header files are included from multiple source files
- 2020-07-09:
  - Duty cycle and adaptive data rate control moved out of Commissioning.h
- 2020-06-25:
  - Rework the timer functions for nRF52 family. OTAA now working better
  - Add option to select between OTAA and ABP for LoRaWan when calling **`lmh_init()`** 
  - Reworked SX126x reset function
  - Support for RAKwireless RAK4630/4631 => **`lora_rak4630_init()`**  
- 2020-06-14:
  - Fix Travis CI & documentation
  - Add option to select LDO instead of DCDC for SX126x chip in **`hwConfig`** struct
- 2020-05-22:
  - Fix compiler errors when OTAA is selected
- 2020-05-20:
  - Add compatibility with nRF52840 (experimental) 
  - Fix ArduinoIDE compile problems
  - Fix examples
- 2020-03-28:
  - Fix bug in LoRaWan Class switch 
- 2020-03-10:
  - Added new SetCadParameter function to Radio class 
- 2020-01-16:
  - Fix bug in receive callbacks in case a CRC error is detected.
  - Added Preamble detection callback
  - Added two more examples for a sensor node and a gateway node with deep sleep usage.
- 2019-12-28:
  - Updated examples
- 2019-12-12:
  - Added check if SX126x is really connected
  - Fixed second bug in the definition of the sync word
  - Added IRQ settings in RadioSetRxDutyCycle
- 2019-12-09:
  - Fixed bug in the definition of the sync word
  - Added possibility to re-init connection to SX1261/2 after CPU wakes up from sleep/deep-sleep
    - **`lora_hardware_re_init()`** to re-initialize SX1262 connection without resetting the LoRa chip
    - **`Radio.ReInit()`** to re-initialize SX1262 connection without resetting the LoRa chip
    - **`Radio.IrqProcessAfterDeepSleep()`** to handle IRQ that woke up the CPU (RX_DONE, TX_DONE, ...)
- 2019-11-09:
  - Added Workarounds for limitations as written in DS_SX1261-2_V1.2 datasheet
  - Tested with both Single Channel ([ESP32](https://github.com/beegee-tokyo/SX1262-SC-GW)) and 8 Channel ([Dragino LPS8](https://www.dragino.com/products/lora-lorawan-gateway/item/148-lps8.html)) LoRaWan gateways
  - Added possibility to force use of sub band of region `lmh_setSubBandChannels()`
- 2019-10-12:
  - On PlatformIO no more need to edit `Commissioning.h`. Everything is done with functions and build flags
  - On ArduinoIDE reduced edititing of `Commissioning.h`. Only the region has to be setup by #define
  - Replaced LoRaWan definitions by function calls `lmh_setDevEui`, `lmh_setAppEui`, `lmh_setAppKey`, `lmh_setNwkSKey`, `lmh_setAppSKey`, `lmh_setDevAddr`, `lmh_setSingleChannelGateway`
  - Updated LoRaWan examples
  - Added CHANNEL.MD and DATARATE.MD lists 
  - Beautify README.MD
- 2019-10-11: Added support for LoRaWan connection to single channel Gateway (no frequency hopping)
- 2019-10-09:    
  - Tested LoRaWan with a single channel LoRaWan gateway.    
  - Added support for single channel gateways    
  - Added support for Insight SIP ISP4520 SoC (nRf52832 + SX1261/2 in one package)    
- 2019-08-01: Added Espressif ESP8266 support
- 2019-07-31: Added LoRaWan support (only partly tested)
- 2019-07-28: Restructure of folders, added nRF52832 support    
- 2019-07-26: First commit.    
----
## Features
  - Support SX1261, SX1262 and SX1268 chips    
  - Support of LoRa protocol and FSK protocol (theoretical, I did not test FSK at all)    
  - Flexible setup for different modules (antenna control, TCXO control)    
  - Support LoRaWan node class A and C tested with single channel LoRaWan gateway    
----
## Functions
WORK IN PROGRESS    
**_Check out the example provided with this library to learn the basic functions._**    
See [examples](https://github.com/beegee-tokyo/SX126x-Android/tree/master/examples)   

----
### Module specific setup    
To adapt the library to different modules and region specific ISM frequencies some defines are used. The following list is not complete yet and will be extended    

----
### Chip selection   
```cpp
#define SX1261_CHIP // if your module has a SX1261 chip    
#define SX1262_CHIP // if your module has a SX1262 or SX1268 chip    
```
----
### MCU to SX126x SPI definition
The hardware configuration is given to the library by a structure with the following elements
```cpp
  hwConfig.CHIP_TYPE = SX1262_CHIP;         // SX1261_CHIP for Semtech SX1261 SX1262_CHIP for Semtech SX1262/1268
  hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // GPIO pin connected to NRESET of the SX126x    
  hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;     // GPIO pin connected to NSS of the SX126x    
  hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // GPIO pin connected to SCK of the SX126x    
  hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // GPIO pin connected to MISO of the SX126x    
  hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // GPIO pin connected to DIO 1 of the SX126x    
  hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // GPIO pin connected to BUSY of the SX126x    
  hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // GPIO pin connected to MOSI of the SX126x    
  hwConfig.RADIO_TXEN = RADIO_TXEN;         // GPIO pin used to enable the RX antenna of the SX126x    
  hwConfig.RADIO_RXEN = RADIO_RXEN;         // GPIO pin used to enable the TX antenna of the SX126x    
  hwConfig.USE_DIO2_ANT_SWITCH = false;     // True if DIO2 is used to switch the antenna from RX to TX
  hwConfig.USE_DIO3_TCXO = true;            // True if DIO3 is used to control the voltage of the TXCO oscillator
  hwConfig.USE_DIO3_ANT_SWITCH = false;     // True if DIO3 is used to enable/disable the antenna
  hwConfig.USE_LDO = false;                 // False if SX126x DCDC converter is used, true if SX126x LDO is used
  hwConfig.USE_RXEN_ANT_PWR = false;        // If set to true RADIO_RXEN pin is used to control power of antenna switch
```    
----
### Explanation for LDO and DCDC selection

The hardware of the SX126x chips can be designed to use either an internal _**LDO**_ or an internal _**DCDC converter**_. The DCDC converter provides better current savings and will be used in most modules. If there are problems to get the SX126x to work, check which HW configuration is used and set **`USE_LDO`** accordingly.   
If **`USE_LDO`** is not set in the hwConfig, DCDC is used as default.    

----
### Explanation for TXCO and antenna control

- RADIO_TXEN and RADIO_RXEN are used on [eByte E22-900M22S](http://www.ebyte.com/en/product-view-news.aspx?id=437) module to switch the antenna between RX and TX    
- DIO2 as antenna switch is used in the example Semtech design as default and might be used by many modules   
- DIO3 as antenna switch is used by e.g. [Insight SIP ISP4520](https://www.insightsip.com/products/combo-smart-modules/isp4520) module which integrates a nRF52832 and a SX126x chip   
- Some modules use DIO3 to control the power supply of the TXCO.    
- Some modules use DIO2 to switch the antenna between RX and TX and a separate GPIO to power the antenna switch on or off. Switching the antenna switch off can reduce the power consumption. The GPIO used to control the antenna power is defined as RADIO_RXEN. LOW == power off, HIGH == power on.
----
## Usage
See [examples](https://github.com/beegee-tokyo/SX126x-Android/examples).    
There is one example for [ArduinoIDE](https://github.com/beegee-tokyo/SX126x-Android/tree/master/examples/PingPong) and one example for [PlatformIO](https://github.com/beegee-tokyo/SX126x-Android/tree/master/examples/PingPongPio) available.    
The PingPong examples show how to define the HW connection between the MCU and the SX126x chip/module.     
Another example is for LoRaWan and is tested with a Single Channel ([ESP32](https://github.com/beegee-tokyo/SX1262-SC-GW)) and a 8 Channel ([Dragino LPS8](https://www.dragino.com/products/lora-lorawan-gateway/item/148-lps8.html)) LoRaWan gateways. The examples can be found here: [ArduinoIDE](https://github.com/beegee-tokyo/SX126x-Android/tree/master/examples/LoRaWan) and one example for [PlatformIO](https://github.com/beegee-tokyo/SX126x-Android/tree/master/examples/LoRaWanPio)    

----
### Basic LoRa communication
----

#### HW structure definition
Structure to define the connection between the MCU and the SX126x 
```cpp
hw_config hwConfig;
```

----
#### GPIO definitions
GPIO definitions for an ESP32. Change it to the connections between the ESP32 and the SX126x in your specific HW design    
```cpp
// ESP32 - SX126x pin configuration
int PIN_LORA_RESET = 4;  // LORA RESET
int PIN_LORA_NSS = 5;    // LORA SPI CS
int PIN_LORA_SCLK = 18;  // LORA SPI CLK
int PIN_LORA_MISO = 19;  // LORA SPI MISO
int PIN_LORA_DIO_1 = 21; // LORA DIO_1
int PIN_LORA_BUSY = 22;  // LORA SPI BUSY
int PIN_LORA_MOSI = 23;  // LORA SPI MOSI
int RADIO_TXEN = 26;     // LORA ANTENNA TX ENABLE
int RADIO_RXEN = 27;     // LORA ANTENNA RX ENABLE
```

----
#### LoRa definitions
Check the SX126x datasheet for explanations    
The bandwidth can be set to any bandwidth supported by the SX126x:    
| Index | Bandwidth | Index | Bandwidth |
| --- | --- | --- | --- |
| 0 | 125 kHz | 5 | 31.25 kHz |
| 1 | 250 kHz | 6 | 20.83 kHz |
| 2 | 500 kHz | 7 | 15.63 kHz |
| 3 | 62.5 kHz | 8 | 10.42 kHz |
| 4 | 41.67 kHz | 9 | 7.81 kHz |

```cpp
// Define LoRa parameters
#define RF_FREQUENCY 868000000  // Hz
#define TX_OUTPUT_POWER 22      // dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3 ... 9 see table]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000

#define BUFFER_SIZE 64 // Define the payload size here
```

----
#### Example HW configuration
Fill the structure with the HW configuration
```cpp
  // Define the HW configuration between MCU and SX126x
  hwConfig.CHIP_TYPE = SX1262_CHIP;         // Example uses an eByte E22 module with an SX1262
  hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
  hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;     // LORA SPI CS
  hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // LORA SPI CLK
  hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // LORA SPI MISO
  hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
  hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // LORA SPI BUSY
  hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // LORA SPI MOSI
  hwConfig.RADIO_TXEN = RADIO_TXEN;         // LORA ANTENNA TX ENABLE
  hwConfig.RADIO_RXEN = RADIO_RXEN;         // LORA ANTENNA RX ENABLE
  hwConfig.USE_DIO2_ANT_SWITCH = false;     // Example uses an eByte E22 module which uses RXEN and TXEN pins as antenna control
  hwConfig.USE_DIO3_TCXO = true;            // Example uses an eByte E22 module which uses DIO3 to control oscillator voltage
  hwConfig.USE_DIO3_ANT_SWITCH = false;     // Only Insight ISP4520 module uses DIO3 as antenna control
  hwConfig.USE_LDO = false;                 // Set to true if SX126x uses LDO instead of DCDC converter
  hwConfig.USE_RXEN_ANT_PWR = false;        // Antenna power is not controlled by a GPIO
```
----
#### Module specific initialization
----
- If you use a microcontroller and a separate board with the SX126x transceiver you need to define the hwConfig structure to define the GPIO's used to connect the two chips.
- If you use the Insight SIP4520 or the RAKwireless RAK4630/4631 modules the connections between the chips are fixed. In this case you do not need the hwConfig structure and can instead use simplified initialzation functions as shown below.
----
#### Module specific header files
----
- If you use a microcontroller and a separate board with the SX126x transceiver use the generic header files **`SX126x-Arduino.h`** and **`LoRaWan-Arduino.h`**
- If you use the RAKwireless RAK4630/4631 modules use the module specific header files **`SX126x-RAK4630.h`** and **`LoRaWan-RAK4630.h`**
- If you use the Insight SIP4520 modules use the module specific header files **`SX126x-ISP4520.h`** and **`LoRaWan-ISP4520.h`**
----
#### Initialize the LoRa HW
```cpp
  lora_hardware_init(hwConfig);
```
----
#### Simplified LoRa HW initialization for specific modules
Some modules integrate an MCU and the SX126x LoRa transceiver and have a fixed connection between them. In these cases a simplified initialization can be used.

#### Simplified LoRa HW initialization for ISP4520 module
The ISP4520 module has the nRF52832 and SX1261 or SX1262 chips integrated in a module. Therefore the hardware configuration is fixed. To initialize the LoRa chip you need only to specify if the module is based on a SX1261 (ISP4520 EU version) or on a SX1262 (ISP4520 US version).
```cpp
  lora_isp4520_init(SX1262);
```
----
#### Simplified LoRa HW initialization for RAK4630/4631 module
The RAK4630/4631 module has the nRF52840 and SX1262 chips integrated in a module. Therefore the hardware configuration is fixed.    
```cpp
  lora_rak4630_init();
```
----
#### Simplified LoRa HW initialization for RAK11300/11310 module
The RAK11300/11310 module has the RP2040 and SX1262 chips integrated in a module. Therefore the hardware configuration is fixed.    
```cpp
  lora_rak11300_init();
```
----
#### Simplified LoRa HW initialization for RAK13300 module
The RAK13300 module is an IO module that has a LoRa SX1262 LoRa transceiver. It is made for the RAK11200 ESP32 module and the hardware configuration is fixed.    
```cpp
  lora_rak13300_init();
```
----
#### Initialize the LoRa HW after CPU woke up from deep sleep
When you want to use the deep sleep function of the ESP32 with external wake up source, you do not want to reset and reconfigure the SX126x chip after its IRQ woke up the ESP32. This re-init function sets up only the required definitions for the communication without resetting the SX126x
```cpp
  lora_hardware_re_init(hwConfig);
```

----
#### Setup the callbacks for LoRa events
```cpp
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;
  RadioEvents.CadDone = OnCadDone;
```

----
#### Initialize the radio
Initialize the radio and set the TX and RX parameters
```cpp
  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
```

----
#### Initialize the radio after CPU woke up from deep sleep
When you want to use the deep sleep function of the ESP32 with external wake up source, you do not want to reset and reconfigure the SX126x chip after its IRQ woke up the ESP32. Radio.ReInit() sets up only the required communication without resetting the SX1262. 
Radio.IrqProcessAfterDeepSleep() is checking the reason for the wake-up IRQ and calls the event handler
```cpp
  Radio.ReInit(&RadioEvents);

  Radio.IrqProcessAfterDeepSleep();
```

----
#### Start listening for packets
```cpp
  Radio.Rx(RX_TIMEOUT_VALUE);
```

----
### LoRaWan
**YOU NEED BELOW STEPS ONLY IF YOU WANT TO IMPLEMENT THE LORAWAN FUNCTIONALITY, IT IS NOT REQUIRED FOR BASIC LORA COMMUNICATION**   
If you want to use [LoRaWan](https://lora-alliance.org/) communication some additional steps are required.    
You need to define a region. The defined region tells the library which frequency and which channels should be used. Valid regions are:    
- _**LORAMAC_REGION_AS923**_ -> Asia 923 MHz    
- _**LORAMAC_REGION_AU915**_ -> Australia 915 MHz    
- _**LORAMAC_REGION_CN470**_ -> China 470 MHz    
- _**LORAMAC_REGION_CN779**_ -> China 779 MHz    
- _**LORAMAC_REGION_EU433**_ -> Europe 433 MHz    
- _**LORAMAC_REGION_EU868**_ -> Europe 868 MHz    
- _**LORAMAC_REGION_IN865**_ -> India 865 MHz    
- _**LORAMAC_REGION_KR920**_ -> Korea 920 MHz    
- _**LORAMAC_REGION_US915**_ -> US 915 MHz    
- _**LORAMAC_REGION_AS923_2**_ -> Asia 923 MHz with frequency shift of -1.8MHz (not tested)   
- _**LORAMAC_REGION_AS923_3**_ -> Asia 923 MHz with frequency shift of -6.6MHz (e.g. Philippines) (in use)    
- _**LORAMAC_REGION_AS923_4**_ -> Asia 923 MHz with frequency shift of -5.9MHz (Israel) (not tested)    
- _**LORAMAC_REGION_RU864**_ -> Russia 864 MHz (not tested)    

More information:    
- **[Channel plan per Region](./CHANNELS.MD)**    
- **[Max packet size per Region and Datarate](./MAX_PACKET_SIZE.md)**
- **[LoRaWan Datarate to SF and BW table](./DATARATE.MD)**    

In addition you need
- Device EUI if you want to use ABP registration of the device
- Application EUI 
- Application Key, the AES encryption/decryption cipher application key
- Device address
- Network Session Key
- App Session Key    

for your node. 

Sparkfun has a nice [tutorial](https://learn.sparkfun.com/tutorials/lorawan-with-prorf-and-the-things-network) how to get these requirements from [TheThingsInternet](https://www.thethingsnetwork.org/)

In addition you must define several LoRaWan parameters.
- Enable or disable adaptive data rate
- Set the default or a specific data rate
- Define if you want to connect to a public or private network
- Specify the number of join trials in case you use OTAA
- Specify the TX power
- Enable or disable the duty cycle transmissions. For EU retion the ETSI mandates duty cycled transmissions.

You can find a lot of information about LoRaWan on the [LoRa Alliance](https://lora-alliance.org/) website.

----

#### LoRaWan region definitions 
The LoRaWAN region is set during the lmh_init() call.    
See [lmh_init()](#initialize) for details.

----
### LoRaWan functions
----
#### Set EUIs and keys
To be able to send data over a gateway to an IoT application like TheThingsNetwork you need to set the EUIs and Keys for the device, the application and the sessions.
If you are using ABP activation all 6 values need to be set. If you are using OTAA activation, only the device EUI, the application EUI and the application key are required.    
For the difference between ABP and OTAA activation read the [TheThingsNetwork Wiki](https://www.thethingsnetwork.org/docs/lorawan/address-space.html).    
The EUIs, keys and address should be defined in your code like this:    
```cpp
// Device EUI
uint8_t nodeDeviceEUI[8] = {0x00, 0x95, 0x64, 0x1F, 0xDA, 0x91, 0x19, 0x0B};
// Application EUI
uint8_t nodeAppEUI[8] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x01, 0xE1};
// Application key for AES encryption
uint8_t nodeAppKey[16] = {0x07, 0xC0, 0x82, 0x0C, 0x30, 0xB9, 0x08, 0x70, 0x0C, 0x0F, 0x70, 0x06, 0x00, 0xB0, 0xBE, 0x09};
// Device address
uint32_t nodeDevAddr = 0x260116F8;
// Network session key for AES encryption
uint8_t nodeNwsKey[16] = {0x7E, 0xAC, 0xE2, 0x55, 0xB8, 0xA5, 0xE2, 0x69, 0x91, 0x51, 0x96, 0x06, 0x47, 0x56, 0x9D, 0x23};
// Application session key for AES encryption
uint8_t nodeAppsKey[16] = {0xFB, 0xAC, 0xB6, 0x47, 0xF3, 0x58, 0x45, 0xC7, 0x50, 0x7D, 0xBF, 0x16, 0x8B, 0xA8, 0xC1, 0x7C};
```
Then, just before initializing the library set these values with
```cpp
// Setup the EUIs and Keys
lmh_setDevEui(nodeDeviceEUI);
lmh_setAppEui(nodeAppEUI);
lmh_setAppKey(nodeAppKey);
lmh_setNwkSKey(nodeNwsKey);
lmh_setAppSKey(nodeAppsKey);
lmh_setDevAddr(nodeDevAddr);
```
----
#### Connection to a single channel gateway
If the node talks to a single channel gateway you can fix the frequency and data rate and avoid frequency hopping. See more info in [LoRaWan single channel gateway](#lorawan-single-channel-gateway)
```cpp
 lmh_setSingleChannelGateway(uint8_t userSingleChannel, int8_t userDatarate)
```   
----
#### Initialize
Initialize LoRaWan.      
```cpp
/**@brief Lora Initialisation
 *
 * @param callbacks   Pointer to structure containing the callback functions
 * @param lora_param  Pointer to structure containing the parameters
 * @param otaa        Choose OTAA (true) or ABP (false) activation
 * @param nodeClass   Choose node class CLASS_A, CLASS_B or CLASS_C, default to CLASS_A
 * @param region      Choose LoRaWAN region to set correct region parameters, defaults to EU868
 *
 * @retval error status
 */
	lmh_error_status lmh_init(lmh_callback_t *callbacks, lmh_param_t lora_param, bool otaa, 
	                          eDeviceClass nodeClass = CLASS_A, 
	                          LoRaMacRegion_t region = LORAMAC_REGION_EU868);
```   
Valid regions are:
LoRaMacRegion_t region
This parameter selects the LoRaWAN region for your application. Allowed values for the region are:    
- _**LORAMAC_REGION_AS923**_    
- _**LORAMAC_REGION_AU915**_    
- _**LORAMAC_REGION_CN470**_    
- _**LORAMAC_REGION_CN779**_    
- _**LORAMAC_REGION_EU433**_    
- _**LORAMAC_REGION_EU868**_    
- _**LORAMAC_REGION_IN865**_    
- _**LORAMAC_REGION_KR920**_    
- _**LORAMAC_REGION_US915**_    
- _**LORAMAC_REGION_AS923_2**_
- _**LORAMAC_REGION_AS923_3**_
- _**LORAMAC_REGION_AS923_4**_
- _**LORAMAC_REGION_RU864**_    

----

**REMARK**    
> **`CN779-787 devices may not be produced, imported or installed after 2021-01-01; deployed devices may continue to operate through their normal end-of-life.`**    

----
#### Specifiy sub bands
For some regions and some gateways you need to specifiy a sub band to be used.  See more info in [Limit frequency hopping to a sub band](#limit-frequency-hopping-to-a-sub-band)
```cpp
 lmh_setSubBandChannels(uint8_t subBand)
```   
**_RAKwireless RAK4630/RAK4631_**    
The subbands for each region are automatically preset to match with the RAKwireless gateways default settings. In this case you do not need to define the sub bands.        

----
#### Callbacks
LoRaWan needs callbacks and parameters defined before initialization    
```cpp
/** Lora user application data buffer. */ 
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];
/** Lora user application data structure. */
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};

/**@brief Structure containing LoRaWan parameters, needed for lmh_init()
 * 
 * Set structure members to
 * LORAWAN_ADR_ON or LORAWAN_ADR_OFF to enable or disable adaptive data rate
 * LORAWAN_DEFAULT_DATARATE OR DR_0 ... DR_5 for default data rate or specific data rate selection
 * LORAWAN_PUBLIC_NETWORK or LORAWAN_PRIVATE_NETWORK to select the use of a public or private network
 * JOINREQ_NBTRIALS or a specific number to set the number of trials to join the network
 * LORAWAN_DEFAULT_TX_POWER or a specific number to set the TX power used
 * LORAWAN_DUTYCYCLE_ON or LORAWAN_DUTYCYCLE_OFF to enable or disable duty cycles
 *                   Please note that ETSI mandates duty cycled transmissions. 
 */
static lmh_param_t lora_param_init = {LORAWAN_ADR_ON, 
			LORAWAN_DEFAULT_DATARATE, LORAWAN_PUBLIC_NETWORK, 
			JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER};

/**@brief Structure containing LoRaWan callback functions, needed for lmh_init() */

static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
			lorawan_rx_handler, lorawan_has_joined_handler, lorawan_confirm_class_handler,
			lorawan_join_failed_handler, lorawan_unconfirmed_finished, lorawan_confirmed_finished};
```    

_**The following callbacks are implemented in the library, but you can override them in your application code:**_    
`BoardGetBatteryLevel` is an empty pre-defined callback in the library. Every board has a different method to read the battery level (or none at all). If you want the LoRaWAN node to report it's battery level, you should write your own function to read and return the battery level. Keep in mind that the LoRaWAN server expects the battery level as a value between 0 and 255. 100% battery level equals 255.     
`BoardGetUniqueId` is a pre-defined callback used by the library to get a unique ID of the board. This is used when the device EUI is assigned automatically. In most use cases this is not used.    
`BoardGetRandomSeed` is used together with `BoardGetUniqueId`. In most use cases this is not used.    
_**The following callbacks have to be implemented in your application code:**_    
`lorawan_rx_handler` is called when a Downlink was received from the LoRaWAN server. See examples how to implement it.    
`lorawan_has_joined_handler` is called after the node has successfully joined the network. Keep in mind that when ABP join method is used, this callback is called immediately after `lmh_join()`.    
`lorawan_confirm_class_handler` is called if you change the nodes class with `lmh_class_request()`.    
`lorawan_join_failed_handler` is called if the join process failed. Failing te join process can have multiple reasons. A few can be      
- No gateway in range
- Gateway not connected to a LoRaWAN server
- Wrong DevEUI, AppEUI or AppKey

`lorawan_unconfirmed_finished` is called after a unconfirmed packet send is finished. It is called after RX1 window and RX2 window timed out or after a downlink from the LoRaWAN server was received.     
`lorawan_confirmed_finished` is called after a confirmed packet send is finished. It has a paramter that tells if a confirmation (`ACK`) was received from the LoRaWAN server or not.


----
#### Join
Join the LoRaWan network to be able to send and receive data. Default connection type is     
```cpp
void lmh_join(void)
```    
----
#### LoRaWan single channel gateway
By default when using LoRaWan communication, the node is using frequency hoping. That means that for each package to be sent a random frequency is chosen out of the predefined frequencies for a region. The frequency (== channels) for each region can be found in the file [CHANNELS.MD](./CHANNELS.MD).    
If connecting the node to a single channel gateway this is a problem, because a single channel gateway can receive only on one channel (== frequency). To get around this problem the channel hoping can be disabled and a fixed frequency (channel) and datarate can be set by the function
```cpp
void lmh_setSingleChannelGateway(uint8_t userSingleChannel, int8_t userDatarate);
```
The first paramenter is the channel (frequency) to be used to communicate with the single channel gateway.    
Check the specification of your single channel gateway to find out on which channel (frequency) it is listening and then get the channel number from the file [CHANNELS.MD](./CHANNELS.MD).     
The second parameter selects the datarate for the communication. Again check the specification of your single channel gateway to find out what datarate it is using and use it in the function call. It might be that instead of the datarate the spreading factor SF and bandwidth BW are documented. In this case you need to check the file [DATARATE.MD](./DATARATE.MD) to find out which datarate to choose.    
E.g. the [things4u ESP-1ch-Gateway-v5.0](https://github.com/things4u/ESP-1ch-Gateway-v5.0/tree/master/ESP-sc-gway) single channel gateway when setup to US915 region is listening on 902.30 Mhz with a bandwidth of 125kHz and a spreading factor of 7.
In **_CHANNEL.MD_** you can find that 902.30 MHz is channel 0 and in **_DATARATE.MD_** you can find that SF7 and BW 125 kHz would be for region US915 the data rate DR_3.
In this example we fix the communication to the channel 0 with the datarate DR_3 (SF7 and BW125);
```cpp
// Setup connection to a single channel gateway
lmh_setSingleChannelGateway(0, DR_3);
```
----
#### Limit frequency hopping to a sub band    
While testing the LoRaWan functionality I discovered that for some regions and some LoRaWan gateways it is required to limit the frequency hopping to a specific sub band of the region.    
E.g. in the settings of the LoRaWan gateway I bought for testing ([Dragino LPS8](https://www.dragino.com/products/lora-lorawan-gateway/item/148-lps8.html)) you have not only to define the region, but as well one of 8 sub bands. The gateway will listen only on the selected sub band.    
The problem is that if the LoRa node uses all available frequencies for frequency hopping, then for sure some of the packets will be lost, because they are sent on frequencies outside of the sub band on which the gateway is listening.    
Depending on the region, there could be between 2 and 12 sub bands to select from. Each sub band consists of 8 frequencies with a fixed distance between each. The sub bands are selected by numbers starting with **`1`** for the first sub band of 8 frequencies.   
_**You have to check with your LoRaWan gateway if you need to setup a sub band**_    
Example to limit the frequency hopping to sub band #1
```cpp
// For some regions we might need to define the sub band the gateway is listening to
/// \todo This is for Dragino LPS8 gateway. How about other gateways???
if (!lmh_setSubBandChannels(1))
{
	Serial.println("lmh_setSubBandChannels failed. Wrong sub band requested?");
}
```
----
## Installation
In Arduino IDE open Sketch->Include Library->Manage Libraries then search for _**SX126x-Arduino**_    
In PlatformIO open PlatformIO Home, switch to libraries and search for _**SX126x-Arduino**_. Or install the library in the terminal with _**`platformio lib install SX126x-Arduino`**_    

For manual installation [download](https://github.com/beegee-tokyo/SX126x-Arduino) the archive, unzip it and place the SX126x-Android folder into the library directory.    
In Arduino IDE this is usually _**`<arduinosketchfolder>/libraries/`**_    
In PlatformIO this is usually _**`<user/.platformio/lib>`**_    

----
