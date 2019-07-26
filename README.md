SX126x-ESP32
===

General info
--------
Arduino library for LoRa communication with Semtech SX126x chips. It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32. It will not work with other uC's like AVR or Espressif 8266 (yet).    

I stumbled over the [SX126x LoRa family](https://www.semtech.com/products/wireless-rf/lora-transceivers) in a customer project. The existing Arduino libraries for Semtech's SX127x family are unfortunately not working with this new generation LoRa chip. I found a usefull base library from Insight SIP which is based on the original Semtech SX126x library and changed it to work with the ESP32.   
For now the library is tested with an [eByte E22-900M22S](http://www.ebyte.com/en/product-view-news.aspx?id=437) module    

__**Check out the example provided with this library to learn the basic functions.**__

THIS IS WORK IN PROGRESS AND NOT ALL FUNCTIONS ARE INCLUDED NOR TESTED. USE IT AT YOUR OWN RISK!
=== 

Based on    
-------- 
- Semtech open source code for SX126x chips [SX126xLib](https://os.mbed.com/teams/Semtech/code/SX126xLib/)    
- Insight SIP open source code for ISP4520 module [LIBRARY - Source Code Examples](https://www.insightsip.com/fichiers_insightsip/pdf/ble/ISP4520/ISP4520_Source_Code.zip)    

Licenses    
--------
Library published under MIT license    

Semtech revised BSD license    
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

Changelog
--------
- 2019-07-26: First commit.    

Features
--------
  - Support SX1261, SX1262 and SX1268 chips    
  - Support of LoRa protocol and FSK protocol (theoretical, I did not test FSK at all)    

Functions
-----
WORK IN PROGRESS    
**_Check out the example provided with this library to learn the basic functions._**

Module specific setup    
--------
To adapt the library to different modules and region specific ISM frequencies some defines are used. The following list is not complete yet and will be extended    

**_Chip selection_**    
```
#define SX1261_CHIP // if your module has a SX1261 chip    
#define SX1262_CHIP // if your module has a SX1262 or SX1268 chip    
```
**_LoRa parameters_**    
```
#define RF_FREQUENCY 915000000  // Hz
#define TX_OUTPUT_POWER 14      // dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz, \
                                //  1: 250 kHz, \
                                //  2: 500 kHz, \
                                //  3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12] was 7
#define LORA_CODINGRATE 1       // [1: 4/5,       \
                                //  2: 4/6, was 1 \
                                //  3: 4/7,       \
                                //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000
```
**_ESP32 to SX126x SPI definition_**   
```
#define PIN_LORA_RESET 4  // LORA RESET
#define PIN_LORA_NSS 5    // LORA SPI CS
#define PIN_LORA_SCLK 18  // LORA SPI CLK
#define PIN_LORA_MISO 19  // LORA SPI MISO
#define PIN_LORA_DIO_1 21 // LORA DIO_1
#define PIN_LORA_BUSY 22  // LORA SPI BUSY
#define PIN_LORA_MOSI 23  // LORA SPI MOSI
#define RADIO_TXEN 26     // LORA ANTENNA TX ENABLE (eByte E22 module)
#define RADIO_RXEN 27     // LORA ANTENNA RX ENABLE (eByte E22 module)
#define BUFFER_SIZE 64    // Define the payload size here
```

Usage
-----
See [examples](https://github.com/beegee-tokyo/SX126x-ESP32/examples).    

Installation
------------

**_Library is not yet published for Arduino IDE nor PlatformIO_**    
In Arduino IDE open Sketch->Include Library->Manage Libraries then search for _**SX126x-ESP32**_    
In PlatformIO open PlatformIO Home, switch to libraries and search for _**SX126x-ESP32**_. Or install the library in the terminal with _**`platformio lib install xxxx`**_    

For manual installation [download](https://github.com/beegee-tokyo/SX126x-ESP32) the archive, unzip it and place the SX126x-ESP32 folder into the library directory.    
In Arduino IDE this is usually _**`<arduinosketchfolder>/libraries/`**_    
In PlatformIO this is usually _**`<user/.platformio/lib>`**_    
