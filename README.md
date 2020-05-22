# SX126x-Arduino
----
Arduino library for LoRa communication with Semtech SX126x chips. It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32, ESP8266 and nRF52832. It will not work with other uC's like AVR.    

----
## Content

|      |      |      |
| :---- | :---- | :---- |
| [General Info](#general-info) | [LoRa](#usage) | &nbsp;&nbsp;[LoRaWan](#lorawan) |
| &nbsp;&nbsp;[Based on](#based-on) | &nbsp;&nbsp;[Basic LoRa communication](#basic-lora-communication) | &nbsp;&nbsp;&nbsp;&nbsp;[ArduinoIDE LoRaWan definitions](#arduinoide-lorawan-definitions) |
| &nbsp;&nbsp;[Licenses](#licenses) | &nbsp;&nbsp;&nbsp;&nbsp;[HW structure definition](#hw-structure-definition) | &nbsp;&nbsp;&nbsp;&nbsp;[PlatformIO LoRaWan definitions](#platformio-lorawan-definitions) |
| [Changelog](#changelog) | &nbsp;&nbsp;&nbsp;&nbsp;[GPIO definitions](#gpio-definitions) | &nbsp;&nbsp;[LoRaWan functions](#lorawan-functions) |
| [Features](#features) | &nbsp;&nbsp;&nbsp;&nbsp;[Example HW configuration](#example-hw-configuration) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialize](#initialize) |
| [Functions](#functions) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialize the LoRa HW](#initialize-the-lora-hw) | &nbsp;&nbsp;&nbsp;&nbsp;[Callbacks](#callbacks) |
| &nbsp;&nbsp;[Module specific setup](#module-specific-setup) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialization for ISP4520 module](#simplified-lora-hw-initialization-for-isp4520-module) | &nbsp;&nbsp;&nbsp;&nbsp;[Join](#join) |
| &nbsp;&nbsp;[Chip selection](#chip-selection) | &nbsp;&nbsp;&nbsp;&nbsp;[Setup the callbacks for LoRa events](#setup-the-callbacks-for-lora-events) | &nbsp;&nbsp;&nbsp;&nbsp;[LoRaWan single channel gateway](#lolawan-single-channel-gateway) |
| &nbsp;&nbsp;[LoRa parameters](#lora-parameters) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialize the radio](#initialize-the-radio) |  &nbsp;&nbsp;&nbsp;&nbsp;[Limit frequency hopping to a sub band](#limit-frequency-hopping-to-a-sub-band) |
| &nbsp;&nbsp;[SPI definition](#mcu-to-sx126x-spi-definition) | &nbsp;&nbsp;&nbsp;&nbsp;[Initialize the radio](#initialize-the-radio) |
| &nbsp;&nbsp;[TXCO and antenna control](#explanation-for-txco-and-antenna-control) | &nbsp;&nbsp;&nbsp;&nbsp;[Start listening for packages](#start-listening-for-packages) | [Installation](#installation) |

----
## General info
I stumbled over the [SX126x LoRa family](https://www.semtech.com/products/wireless-rf/lora-transceivers) in a customer project. Most of the existing Arduino libraries for Semtech's SX127x family are unfortunately not working with this new generation LoRa chip. I found a usefull base library from Insight SIP which is based on the original Semtech SX126x library and changed it to work with the ESP32.   
For now the library is tested with an [eByte E22-900M22S](http://www.ebyte.com/en/product-view-news.aspx?id=437) module connected to an ESP32 and an [Insight SIP ISP4520](https://www.insightsip.com/products/combo-smart-modules/isp4520) which combines a Nordic nRF52832 and a Semtech SX1262 in one module    

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
  - Flexible setup for different modules (antenna control, TXCO control)    
  - Support LoRaWan node class A, B and C tested with single channel LoRaWan gateway    
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
```    
----
### Explanation for TXCO and antenna control

RADIO_TXEN and RADIO_TXEN are used on [eByte E22-900M22S](http://www.ebyte.com/en/product-view-news.aspx?id=437) module to switch the antenna between RX and TX    
DIO2 as antenna switch is used in the example Semtech design as default and might be used by many modules   
DIO3 as antenna switch is used by e.g. [Insight SIP ISP4520](https://www.insightsip.com/products/combo-smart-modules/isp4520) module which integrates a nRF52832 and a SX126x chip   

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
```cpp
// Define LoRa parameters
#define RF_FREQUENCY 868000000  // Hz
#define TX_OUTPUT_POWER 22      // dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
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
```

----
#### Initialize the LoRa HW
```cpp
  lora_hardware_init(hwConfig);
```

----
#### Simplified LoRa HW initialization for ISP4520 module
```cpp
  lora_isp4520_init(SX1262);
```

----
#### Initialize the LoRa HW after CPU woke up from deep sleep
When you want to use the deep sleep function of the ESP32 with external wake up source, you do not want to reset and reconfigure the SX126x chip after its IRQ woke up the ESP32. This re-init function sets up only the required definitions for the communication without resetting the ESP32
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
When you want to use the deep sleep function of the ESP32 with external wake up source, you do not want to reset and reconfigure the SX126x chip after its IRQ woke up the ESP32. Radio.ReInit() sets up only the required communication means resetting the ESP32. 
Radio.IrqProcessAfterDeepSleep() is checking the reason for the wake-up IRQ and calls the event handler
```cpp
  Radio.ReInit(&RadioEvents);

  Radio.IrqProcessAfterDeepSleep();
```

----
#### Start listening for packages
```cpp
  Radio.Rx(RX_TIMEOUT_VALUE);
```

----
### LoRaWan
**YOU NEED BELOW STEPS ONLY IF YOU WANT TO IMPLEMENT THE LORAWAN FUNCTIONALITY, IT IS NOT REQUIRED FOR BASIC LORA COMMUNICATION**   
If you want to use [LoRaWan](https://lora-alliance.org/) communication some additional steps are required.    
You need to define a region. The defined region tells the library which frequency and which channels should be used. Valid regions are:    
- REGION_AS923 -> Asia 923 MHz
- REGION_AU915 -> Australia 915 MHz
- REGION_CN470 -> China 470 MHz
- REGION_CN779 -> China 779 MHz
- REGION_EU433 -> Europe 433 MHz
- REGION_EU868 -> Europe 868 MHz
- REGION_IN865 -> India 865 MHz
- REGION_KR920 -> Korea 920 MHz
- REGION_US915 -> US 915 MHz

In addition you need
- Device EUI if you want to use ABP registration of the device
- Application EUI 
- Application Key, the AES encryption/decryption cipher application key
- Device address
- Network Session Key
- App Session Key

for your node. 

Sparkfun has a nice [tutorial](https://learn.sparkfun.com/tutorials/lorawan-with-prorf-and-the-things-network) how to get these requirements from [TheThingsInternet](https://www.thethingsnetwork.org/)

You can find a lot of information about LoRaWan on the [LoRa Alliance](https://lora-alliance.org/) website.

----
#### ArduinoIDE LoRaWan definitions 
If you are using ArduinoIDE you need to edit the file ```/src/mac/Commissioning.h``` and define the region there.    
 
In Arduino IDE you can find the file in _**`<arduinosketchfolder>/libraries/SX126x-Arduino/src/mac`**_    
In PlatformIO this is usually _**`<user>/.platformio/lib/SX126x-Arduino/src/mac`**_    

The region is set right on the top of the file. Look for    
```
#if !defined(REGION_AS923) && !defined(REGION_AU915) && !defined(REGION_CN470) && !defined(REGION_CN779) && !defined(REGION_EU433) && !defined(REGION_EU868) && !defined(REGION_IN865) && !defined(REGION_KR920) && !defined(REGION_US915) && !defined(REGION_US915_HYBRID)
#define REGION_US915
#endif
```
and change the line
```
#define REGION_US915
```
to the region you want to use, e.g.    
```
#define REGION_EU868
```

----
#### PlatformIO LoRaWan definitions 
If you are using PlatformIO you must define the region in the platformio.ini file of your project.     
Open the platformio.ini file and add a define for the region e.g.    
```
build_flags = -DREGION_AS923
```
Here a example for a complete platformio.ini file
```
[env:esp32dev]
platform = https://github.com/platformio/platform-espressif32.git#feature/stage
board = esp32dev
framework = arduino
upload_port = COM6
upload_speed = 921600
build_flags = 
	-DCORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_ERROR
	-DREGION_EU868
```
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
Initialize LoRaWan
```cpp
 lmh_init(lmh_callback_t *callbacks, lmh_param_t lora_param)
```   
----
#### Specifiy sub bands
For some regions and some gateways you need to specifiy a sub band to be used.  See more info in [Limit frequency hopping to a sub band](#limit-frequency-hopping-to-a-sub-band)
```cpp
 lmh_setSubBandChannels(uint8_t subBand)
```   
----
#### Callbacks
LoRaWan needs callbacks and parameters defined before initialization    
```cpp
/** Lora user application data buffer. */ 
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];
/** Lora user application data structure. */
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};

/**@brief Structure containing LoRaWan parameters, needed for lmh_init() */
static lmh_param_t lora_param_init = {LORAWAN_ADR_ON, 
			LORAWAN_DEFAULT_DATARATE, LORAWAN_PUBLIC_NETWORK, 
			JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER};

/**@brief Structure containing LoRaWan callback functions, needed for lmh_init() */
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
	lorawan_rx_handler, lorawan_has_joined_handler, lorawan_confirm_class_handler};
```    
----
#### Join
Join the LoRaWan network to be able to send and receive data    
```cpp
void lmh_join(void)
```    
----
#### LoRaWan single channel gateway
By default when using LoRaWan communication, the node is using frequency hoping. That means that for each package to be sent a random frequency is chosen out of the predefined frequencies for a region. The frequency (== channels) for each region can be found in the file [CHANNELS.MD](https://github.com/beegee-tokyo/SX126x-Arduino/blob/master/CHANNELS.MD).    
If connecting the node to a single channel gateway this is a problem, because a single channel gateway can receive only on one channel (== frequency). To get around this problem the channel hoping can be disabled and a fixed frequency (channel) and datarate can be set by the function
```cpp
void lmh_setSingleChannelGateway(uint8_t userSingleChannel, int8_t userDatarate);
```
The first paramenter is the channel (frequency) to be used to communicate with the single channel gateway.    
Check the specification of your single channel gateway to find out on which channel (frequency) it is listening and then get the channel number from the file [CHANNELS.MD](https://github.com/beegee-tokyo/SX126x-Arduino/blob/master/CHANNELS.MD).     
The second parameter selects the datarate for the communication. Again check the specification of your single channel gateway to find out what datarate it is using and use it in the function call. It might be that instead of the datarate the spreading factor SF and bandwidth BW are documented. In this case you need to check the file [DATARATE.MD](https://github.com/beegee-tokyo/SX126x-Arduino/blob/master/DATARATE.MD) to find out which datarate to choose.    
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
Depending on the region, there could be between 2 and 12 sub bands to select from. Each sub band consists of 8 frequencies with a fixed distance beteween each. The sub bands are selected by numbers starting with **`1`** for the first sub band of 8 frequencies.   
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
