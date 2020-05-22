# SX126x-Arduino
----
Arduino library for LoRa communication with Semtech SX126x chips. It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32. ESP8266 and nRF52832. It will not work with other uC's like AVR.    

# Release Notes
## V1.1.2 Fix compile errors when OTAA is selected
## V1.1.1 Some bug fixes
  - Add compatibility with nRF52840 (experimental)        
  - Fix ArduinoIDE compile problems    
  - Fix examples    
## V1.1.0 Fix bug in LoRaWan class switch
## V1.0.9 Added new SetCadParameter function to Radio class
## V1.0.8 Removed credentials
## V1.0.7 Bug fix and additional callback
  - Fixed bug when received package has CRC error
  - Added preamble detection callback
  - Added sensor and gateway example using deep sleep.
## V1.0.6 Bug fix and deep-sleep functionality
  - Updated examples
  - Added check if SX126x is really connected
  - Fixed second bug in the definition of the sync word
  - Added IRQ settings in RadioSetRxDutyCycle
## V1.0.5 Bug fix and deep-sleep functionality
- Fixed bug in the definition of the sync word
- Added possibility to re-init connection to SX1261/2 after CPU wakes up from sleep/deep-sleep
  - **`lora_hardware_re_init()`** to re-initialize SX1262 connection without resetting the LoRa chip
  - **`Radio.ReInit()`** to re-initialize SX1262 connection without resetting the LoRa chip
  - **`Radio.IrqProcessAfterDeepSleep()`** to handle IRQ that woke up the CPU (RX_DONE, TX_DONE, ...)
## V1.0.4 Extended LoRaWan functionality 
- Tested with both Single Channel ([ESP32](https://github.com/beegee-tokyo/SX1262-SC-GW)) and 8 Channel ([Dragino LPS8](https://www.dragino.com/products/lora-lorawan-gateway/item/148-lps8.html)) LoRaWan gateways 
- Added possibility to set LoRaWan keys programmatically
  - **`lmh_setDevEui()`** to set Device EUI
  - **`lmh_setAppEui()`** to set Application EUI
  - **`lmh_setAppKey()`** to set Application key
  - **`lmh_setNwkSKey()`** to set Network session key
  - **`lmh_setAppSKey()`** to set Application session key
  - **`lmh_setDevAddr()`** to set Device address    
- Added possibility to force use of sub band of region
  - **`lmh_setSubBandChannels()`** to set sub band to be used 
- Implemented workarounds for known limitations
  - Optimizing the Inverted IQ Operation, see DS_SX1261-2_V1.2 datasheet chapter 15.4
  - Modulation Quality with 500 kHz LoRa Bandwidth, see DS_SX1261-2_V1.2 datasheet chapter 15.1
  - Implicit Header Mode Timeout Behavior, see DS_SX1261-2_V1.2 datasheet chapter 15.3
  - Better Resistance of the SX1262 Tx to Antenna Mismatch, see DS_SX1261-2_V1.2 datasheet chapter 15.2

## V1.0.3 Added support to connect as LoRaWan node to a single channel LoRaWan gateway
- Added possibility to force single channel gateway connection
  - **`lmh_setSingleChannelGateway()`** to set single channel frequency and data rate
- Added list with channel - frequency per region

## V1.0.2 LoRaWan compatible
- Tested LoRaWan with a single channel LoRaWan gateway
- Added support for single channel gateways
- Added support for Insight SIP ISP4520 SoC (nRF52832 + Sx1261/2 in one package)

## V1.0.1 Added missing nRF52832 platform

## V1.0.0 First release for ArduinoIDE and PlatformIO
- THIS IS WORK IN PROGRESS AND NOT ALL FUNCTIONS ARE INCLUDED NOR TESTED. USE IT AT YOUR OWN RISK!