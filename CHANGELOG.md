# SX126x-Arduino
----
Arduino library for LoRa communication with Semtech SX126x chips. It is based on Semtech's SX126x libraries and adapted to the Arduino framework for ESP32, ESP8266, nRF52832 and RP2040. It will not work with other uC's like AVR.    

# Release Notes

## V2.0.21 Some fixes
  - Fix public/private network always public
  - Add option to restart MAC stack to change e.g. region without re-init timers

## V2.0.20 Add MAC parameter reset function
  - Add option to reset the MAC counters

## V2.0.19 Fix the release mess
  - Sorry released the wrong branch
  
## V2.0.18 Fix typo
  - Fix typo in RadioTimeOnAir for FSK
  
## V2.0.17 Enhancement
  - Improve RadioTimeOnAir for FSK, thanks to _**@mikedupi**_

## V2.0.16 Fix Confirmed message bug
  - Fix runtime problem in LoRaMacHelper, thanks to _**@avillacis**_

## V2.0.15 Several fixes and improvements
  - Update examples, thanks to _**@DanielBustillos**_    
  - Cleanup RAK4630 initialization    
  - Add lmh_getConfRetries() to readback the confirmed package retry setting    
  - Make RX timeout configurable with `#define RXTIMEOUT_LORA_MAX`, thanks to _**@kisChang**_     
  - Add set default RX gain in SX126xSertRx, thanks to _**@kisChang**_
  - Allow change of TCXO control with hwConfig structure, thanks to _**@dberlin**_
  - Add confirmed failed callback if the degraded datarate becomes insufficient to transmit the payload length, thanks to _**@avillacis**_
  
## V2.0.14 Fix timer and RX timeouts
  - Fix RP2040 timers, thanks to _**@kisChang**_
  - Fix RX window timeouts, thanks to _**@battosai30**_
  
## V2.0.13 Fix RX2 freuency
  - Fix wrong RX2 frequency in AS923-2, AS923-3, AS923-4

## V2.0.12 Fix crash when library debug is enabled
  - Fix crash when library debug is enabled in AS923 region.
  
## V2.0.11 Fix RAK11310 problem
  - Fix the timer problem for the RP2040 (hanging after 1h50 minutes)
  
## V2.0.10 Fix AS923 bug
  - Fix AS923 CFlist channel assignment bug
  
## V2.0.9 Fix EU868 bug
  - In EU868 always the first 8 channels were enabled, even without CFlist or NewChannelReq from LNS

## V2.0.8 Fix bug in lmh_datarate_set
  - Function did change only ADR, but did not update the datarate setting
  
## V2.0.7 Support different antenna switch correct
  - Change antenna switch handling
  
## V2.0.6 Bug fix
  - Correct handling of TX timeout in Radio callbacks
  
## V2.0.5 Bug Fix
  - Add missing declaration for lora_rak13300_init()

## V2.0.4 Bug Fix
  - Correct power regulator setting for RAKwireless RAK11300 module from LDO to DCDC

## V2.0.3 Add new WisBlock Core module
  - Add support for RAKwireless RAK11300 module (Raspberry RP2040 + SX1262 module)

## V2.0.2 Add callbacks for LoRaWAN transmission finished
  - Add callbacks for LoRaWAN TX finished (both confirmed and unconfirmed)
  - Add addional bandwidths for LoRa transmissions. _**Breaks `Radio.TimeOnAir()` for bandwidths other than BW 125, 250 and 500**_
  - Fix minor problem in CF list handling for AS923-x regions

## V2.0.1 Add new regions
  - Implement new regions AS923-2, AS923-3, AS923-4, RU864
  - Test CF list to add additionals channesl on AS923 and RU864

## V2.0.0 Add new features to the library
  - Add support for all LoRaWAN regions without recompilation of the code
  - Add background handling of SX126x IRQ's for better performance
  - Read [WHAT'S NEW IN V2](./README_V2.md) to migrate your application to V2

----    
----    

## V1.3.3 Fix OTAA problems
  - Fix AS923 join problem (use only default channels)
  - Fix OTAA problems (small code bug)
## V1.3.2 Fix join bug when first OTAA join fails
  - When OTAA join failed callback was called, following lmh_join() calls fail always 
## V1.3.1 Improve functionality
  - Add callback for OTAA join failure 
## V1.3.0 Bug fixes for LoRaWAN
  - Fix ADR problem
  - Fix Join problem for some Regions
  - Add some debug output    
  - Add option to set node class during initialization. Defaults to CLASS_A for backward compatibility: **`lmh_error_status lmh_init(lmh_callback_t *callbacks, lmh_param_t lora_param, bool otaa, eDeviceClass class = CLASS_A);`**
## V1.2.1 Add option to control antenna switch power with GPIO
  - Fix wrong control of antenna switch for RAK4631
  - Add option to control power of antenna switch by the library with **`_hwConfig.USE_RXEN_ANT_PWR`**
## V1.2.0 Improvements and linker error fix
  - Duty cycle and adaptive data rate control moved out of Commissioning.h
  - Fixed linker error when header files are included from multiple source files
  - Support for RAKwireless RAK4630 module finished
## V1.1.3 Improve LoRaWan implementation (special thanks to [RAKwireless](https://rakwireless.com) who did a lot of testing with the library)    
  - Rework the timer functions for nRF52 family. OTAA now working better
  - Add option to select between OTAA and ABP for LoRaWan when calling **`lmh_init()`** 
  - Reworked SX126x reset function
  - Support for RAKwireless RAK4630/4631 modules => **`lora_rak4630_init()`**
  - Update region settings
  - **`Commissioning.h`** needs only to be edited for the region. All other LoRaWan settings can be done now from the application side.
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