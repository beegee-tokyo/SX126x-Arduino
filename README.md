SX126x-Arduino
===

General info
--------
Arduino library for LoRa communication with Semtech SX126x chips. It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32 and nRF52832. It will not work with other uC's like AVR or Espressif 8266 (yet).    

I stumbled over the [SX126x LoRa family](https://www.semtech.com/products/wireless-rf/lora-transceivers) in a customer project. The existing Arduino libraries for Semtech's SX127x family are unfortunately not working with this new generation LoRa chip. I found a usefull base library from Insight SIP which is based on the original Semtech SX126x library and changed it to work with the ESP32.   
For now the library is tested with an [eByte E22-900M22S](http://www.ebyte.com/en/product-view-news.aspx?id=437) module connected to an ESP32 and an [Insight SIP ISP4520](https://www.insightsip.com/products/combo-smart-modules/isp4520) which combines a Nordic nRF52832 and a Semtech SX1262 in one module    

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
- 2019-07-31: Added LoRaWan support
- 2019-07-28: Restructure of folders, added nRF52832 support    
- 2019-07-26: First commit.    

Features
--------
  - Support SX1261, SX1262 and SX1268 chips    
  - Support of LoRa protocol and FSK protocol (theoretical, I did not test FSK at all)    
  - Flexible setup for different modules (antenna control, TXCO control)    
  - Support LoRaWan node class A, B and C (theoretical, I have no LoRaWan gateway to test it at all)    

Functions
-----
WORK IN PROGRESS    
**_Check out the example provided with this library to learn the basic functions._**    
See [examples](https://github.com/beegee-tokyo/SX126x-ESP32/tree/master/examples)    

Module specific setup    
--------
To adapt the library to different modules and region specific ISM frequencies some defines are used. The following list is not complete yet and will be extended    

**_Chip selection_**    
```
#define SX1261_CHIP // if your module has a SX1261 chip    
#define SX1262_CHIP // if your module has a SX1262 or SX1268 chip    
```
**_LoRa parameters_**    
Check the SX126x datasheets for the meanings 
```
#define RF_FREQUENCY 915000000  // 915 MHz for US, 868MHz for EU
#define TX_OUTPUT_POWER 14      // max 22dBm for US, max 14dBm for EU
#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12] was 7
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000
```
**_MCU to SX126x SPI definition_**   
The hardware configuration is given to the library by a structure with the following elements
```
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
**Explanation for TXCO and antenna control:**    
RADIO_TXEN and RADIO_TXEN are used on [eByte E22-900M22S](http://www.ebyte.com/en/product-view-news.aspx?id=437) module to switch the antenna between RX and TX    
DIO2 as antenna switch is used in the example Semtech design as default and might be used by many modules   
DIO3 as antenna switch is used by e.g. [Insight SIP ISP4520](https://www.insightsip.com/products/combo-smart-modules/isp4520) module which integrates a nRF52832 and a SX126x chip  

Usage
-----
See [examples](https://github.com/beegee-tokyo/SX126x-ESP32/examples).    
There is one example for [ArduinoIDE](https://github.com/beegee-tokyo/SX126x-ESP32/tree/master/examples/PingPong) and one example for [PlatformIO](https://github.com/beegee-tokyo/SX126x-ESP32/tree/master/examples/PingPongPio) available.    
The PingPong examples show how to define the HW connection between the MCU and the SX126x chip/module.     
Another example is only partly tested. It is for LoRaWan and I could only test it as far as I know the application is sending out packages. But as I don't own a LoRaWan gateway I cannot test the functionality. Theoretically it should support Class A, B and C nodes. The examples can be found here: [ArduinoIDE](https://github.com/beegee-tokyo/SX126x-ESP32/tree/master/examples/LoRaWan) and one example for [PlatformIO](https://github.com/beegee-tokyo/SX126x-ESP32/tree/master/examples/LoRaWanPio)    
To use these examples you need to edit the header file ```Commissioning.h``` in the library folder ```src/mac```    

Structure to define the connection between the MCU and the SX126x 
```
hw_config hwConfig;
```
GPIO definitions for an ESP32. Change it to the connections between the ESP32 and the SX126x in your specific HW design    
```
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
LoRa definitions. Check the SX126x datasheet for explanations    
```
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
Fill the structure with the HW configuration
```
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
Initialize the LoRa HW
```
  lora_hardware_init(hwConfig);
```
Setup the callbacks for LoRa events
```
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;
  RadioEvents.CadDone = OnCadDone;
```
Initialize the radio and set the TX and RX parameters
```
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
Start listening for packages
```
  Radio.Rx(RX_TIMEOUT_VALUE);
```
Initialize LoRaWan
```
  lmh_init(lmh_callback_t *callbacks, lmh_param_t lora_param)
```   
LoRaWan needs callbacks and paramters defined before initialization    
```
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];			  ///< Lora user application data buffer.
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; ///< Lora user application data structure.

/**@brief Structure containing LoRaWan parameters, needed for lmh_init()
 */
static lmh_param_t lora_param_init = {LORAWAN_ADR_ON, LORAWAN_DEFAULT_DATARATE, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER};

/**@brief Structure containing LoRaWan callback functions, needed for lmh_init()
*/
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
										lorawan_rx_handler, lorawan_has_joined_handler, lorawan_confirm_class_handler};
```    
Join the LoRaWan network    
```
void lmh_join(void)
```    


For LoRaWan you need to define the connection in the file ```/src/mac/Commissioning.h```.    
**YOU NEED TO THIS ONLY IF YOU WANT TO IMPLEMENT THE LORAWAN FUNCTIONALITY, IT IS NOT REQUIRED FOR BASIC LORA COMMUNICATION**    
In Arduino IDE you can find the file in _**`<arduinosketchfolder>/libraries/SX126x-Arduino/src/mac`**_    
In PlatformIO this is usually _**`<user>/.platformio/lib/SX126x-Arduino/src/mac`**_    

You can find a lot of information about LoRaWan on the [LoRa Alliance](https://lora-alliance.org/) website.    
```
/**@brief Define your region here 
 * Required because each region has different regulations
 * The LoRa Alliance offers documentation for the regional parameters
 * Latest revision when this library was created
 * https://lora-alliance.org/resource-hub/lorawanr-regional-parameters-v11rb 
 * Check https://lora-alliance.org/resource-hub for any updates
 * 
 * Choose a matching region from below
 * REGION_AS923 -> Asia 923 MHz
 * REGION_AU915 -> Australia 915 MHz
 * REGION_CN470 -> China 470 MHz
 * REGION_CN779 -> China 779 MHz
 * REGION_EU433 -> Europe 433 MHz
 * REGION_EU868 -> Europe 868 MHz
 * REGION_IN865 -> India 865 MHz
 * REGION_KR920 -> Korea 920 MHz
 * REGION_US915 -> US 915 MHz
 */
#define REGION_US915

/**@brief Define activation procedure here
 * More information https://www.thethingsnetwork.org/forum/t/what-is-the-difference-between-otaa-and-abp-devices/2723
 * When set to 1 the application uses the Over-the-Air activation procedure
 * When set to 0 the application uses the Personalization activation procedure
 */
#define OVER_THE_AIR_ACTIVATION         0

/**@brief Indicates if the end-device is to be connected to a private or public network
 */
#define LORAWAN_PUBLIC_NETWORK          true

/**@brief Select if a hard coded device ID is used or an automatic generated one
 * When set to 1 DevEui is LORAWAN_DEVICE_EUI
 * When set to 0 DevEui is automatically generated by calling BoardGetUniqueId function
 */
#define STATIC_DEVICE_EUI               0

/**@brief Mote device IEEE EUI (big endian)
 *
 * @remark see STATIC_DEVICE_EUI comments
 */
#define LORAWAN_DEVICE_EUI              {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

/**@brief Application IEEE EUI (big endian)
 */
#define LORAWAN_APPLICATION_EUI         {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

/**@brief AES encryption/decryption cipher application key
 */
#define LORAWAN_APPLICATION_KEY         {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}

/**@brief Current network ID
 */
#define LORAWAN_NETWORK_ID              (uint32_t)0

/**@brief Select if a hard coded device address is used or an automatic generated one
 * When set to 1 DevAdd is LORAWAN_DEVICE_ADDRESS
 * When set to 0 DevAdd is automatically generated using
 *         a pseudo random generator seeded with a value derived from
 *         BoardUniqueId value
 */
#define STATIC_DEVICE_ADDRESS           1

/**@brief Device address on the network (big endian)
 *
 * @remark In this application the value is automatically generated using
 *         a pseudo random generator seeded with a value derived from
 *         BoardUniqueId value if LORAWAN_DEVICE_ADDRESS is set to 0
 */
#define LORAWAN_DEVICE_ADDRESS          (uint32_t)0x00000000

/**@brief AES encryption/decryption cipher network session key
 */
#define LORAWAN_NWKSKEY                 {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}

/**@brief AES encryption/decryption cipher application session key
 */
#define LORAWAN_APPSKEY                 {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}
```    

Installation
------------

In Arduino IDE open Sketch->Include Library->Manage Libraries then search for _**SX126x-Arduino**_    
In PlatformIO open PlatformIO Home, switch to libraries and search for _**SX126x-Arduino**_. Or install the library in the terminal with _**`platformio lib install SX126x-Arduino`**_    

For manual installation [download](https://github.com/beegee-tokyo/SX126x-Arduino) the archive, unzip it and place the SX126x-ESP32 folder into the library directory.    
In Arduino IDE this is usually _**`<arduinosketchfolder>/libraries/`**_    
In PlatformIO this is usually _**`<user/.platformio/lib>`**_    
