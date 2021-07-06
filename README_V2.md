# _**WHAT'S NEW IN V2**_
_**Some major changes are made in V2 of the SX126x-Arduino library:**_    
_**- The library now supports all LoRaWAN regions without re-compiling**_    
_**- The interrupt handling for SX126x IRQ's are taken into separate tasks for ESP32 and nRF52**_    
_**This requires some code changes in your existing applications. Please read on to learn how to migrate your application to use SX126x-Arduino V2**_

----
----

## Support of all LoRaWAN regions without re-compiling the application

### V1.x
If you want to change the supported LoRaWAN region for your application you had to either    
- _select the region from the Arduino IDE Tools menu (RAK4631 Arduino IDE users)_    

or    
- _define the region in the file `mac/Commissioning.h` (ESP32/nRF52 Arduino IDE users)_   

or    
- _define the region in the platformio.ini of your application in PlatformIO_    

and compile your application for the selected region.

## V2
The library now includes all supported LoRaWAN regions and the region selection is made when you call `lmh_init`.    
The new definition of the function `lmh_init()` is:    
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
The first three parameters are the same as before. Two new parameters have been added.

### eDeviceClass nodeClass
Even this parameter was defined in V1.x, the `lmh_init()` ignored it and initialized the node **ALWAYS** as a node Class A.    
Now you can explicit set your node to _CLASS A_ or _CLASS C_. Please take note that _CLASS B_ is still not supported by the library.

### LoRaMacRegion_t region
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
# IMPORTANT NOTE 1: 
IF YOU DO NOT SET THE TWO NEW PARAMETERS, YOUR APPLICATION WILL BE SETUP AS _CLASS&nbsp;A_ NODE USING _EU868&nbsp;REGION_ PARAMETERS!
# IMPORTANT NOTE 2:
You can set the LoRaWAN region one time after an MCU reset or start-up. Changing the region during run-time is not (yet) supported. It is suggested that you store LoRaWAN settings in non-volitale memory on the MCU. There is an example how to do this for nRF52 and how to setup the LoRaWAN region and other settings over BLE in the [RAK4631-LoRa-BLE-Config](https://github.com/beegee-tokyo/RAK4631-LoRa-BLE-Config) example. I will provide a similar example for the ESP32 in the future.
# IMPORTANT NOTE 4 (FOR PLATFORMIO USERS):  
If you are using PlatformIO, the entry for the region in platformio.ini **must** be removed!     
# IMPORTANT NOTE 3:
For users of the Arduino BSP _`RAK-nRF52-Arduino`_    
The old **`RAK nRF52 Arduino BSP`** sets automatically the region that you selected from the Tools menu. This will cause a compile error when using the **`SX126x-Arduino V2 library`**. To get around this problem, please update the **`RAK nRF52 Arduino BSP`** to version **`0.21.20`** or newer! You can find the installation instructions in the [BSP repository](https://github.com/RAKWireless/RAKwireless-Arduino-BSP-Index)    

Another workaround is to do the following two patches:    
Edit the file    
`C:\Users\<YOUR_NAME>\AppData\Local\Arduino15\packages\raknrf\hardware\nrf52\0.21.11\boards.txt`    
and remove the line   
> `menu.region=Region`    

from the file.    
     
Edit the file    
`C:\Users\<YOUR_NAME>\AppData\Local\Arduino15\packages\raknrf\hardware\nrf52\0.21.11\platform.txt`    
and replace the lines 
>`# build.logger_flags and build.sysview_flags and intentionally empty,`    
>`# to allow modification via a user's own boards.local.txt or platform.local.txt files.`    
>`build.flags.nrf= -DSOFTDEVICE_PRESENT -DARDUINO_NRF52_ADAFRUIT -DNRF52_SERIES -DLFS_NAME_MAX=64 {compiler.optimization_flag} {build.debug_flags} {build.region_flags} {build.logger_flags} {build.sysview_flags} "-I{build.core.path}/cmsis/Core/Include" "-I{nordic.path}" "-I{nordic.path}/nrfx" "-I{nordic.path}/nrfx/hal" "-I{nordic.path}/nrfx/mdk" "-I{nordic.path}/nrfx/soc" "-I{nordic.path}/nrfx/drivers/include" "-I{nordic.path}/nrfx/drivers/src" "-I{nordic.path}/softdevice/{build.sd_name}_nrf52_{build.sd_version}_API/include" "-I{nordic.path}/softdevice/{build.sd_name}_nrf52_{build.sd_version}_API/include/nrf52" "-I{rtos.path}/Source/include" "-I{rtos.path}/config" "-I{rtos.path}/portable/GCC/nrf52" "-I{rtos.path}/portable/CMSIS/nrf52" "-I{build.core.path}/sysview/SEGGER" "-I{build.core.path}/sysview/Config" "-I{build.core.path}/TinyUSB" "-I{build.core.path}/TinyUSB/Adafruit_TinyUSB_ArduinoCore" "-I{build.core.path}/TinyUSB/Adafruit_TinyUSB_ArduinoCore/tinyusb/src"`    

with the lines 
>`# build.logger_flags and build.sysview_flags and intentionally empty,`    
>`# to allow modification via a user's own boards.local.txt or platform.local.txt files.`    
>`build.flags.nrf= -DSOFTDEVICE_PRESENT -DARDUINO_NRF52_ADAFRUIT -DNRF52_SERIES -DLFS_NAME_MAX=64 {compiler.optimization_flag} {build.debug_flags} {build.logger_flags} {build.sysview_flags} "-I{build.core.path}/cmsis/Core/Include" "-I{nordic.path}" "-I{nordic.path}/nrfx" "-I{nordic.path}/nrfx/hal" "-I{nordic.path}/nrfx/mdk" "-I{nordic.path}/nrfx/soc" "-I{nordic.path}/nrfx/drivers/include" "-I{nordic.path}/nrfx/drivers/src" "-I{nordic.path}/softdevice/{build.sd_name}_nrf52_{build.sd_version}_API/include" "-I{nordic.path}/softdevice/{build.sd_name}_nrf52_{build.sd_version}_API/include/nrf52" "-I{rtos.path}/Source/include" "-I{rtos.path}/config" "-I{rtos.path}/portable/GCC/nrf52" "-I{rtos.path}/portable/CMSIS/nrf52" "-I{build.core.path}/sysview/SEGGER" "-I{build.core.path}/sysview/Config" "-I{build.core.path}/TinyUSB" "-I{build.core.path}/TinyUSB/Adafruit_TinyUSB_ArduinoCore" "-I{build.core.path}/TinyUSB/Adafruit_TinyUSB_ArduinoCore/tinyusb/src"`      


----
----

## Background handling of SX126x IRQ's

### V1.x
One big problem of `SX126x-Arduino library` was that the handling of SX126x IRQ's is done by a call from the main loop. Heavy MCU load in the `loop()`, or lengthy calls to sub-routines could influence the IRQ handling up to a level where LoRaWAN join sequences failed or downlink packets from the LoRaWAN server were lost.    
    
### V2
In **V2** of the library the function `Radio.IrqProcess();` is no longer needed on ESP32 and nRF52 MCU's (as well on RAK4631 and ISP4520 modules). The callback functions for the events are staying the same, but they are called from the background IRQ handler. They work exactly as before, they can call functions from your application, they can use `Serial.println()` for debug output, ...

# IMPORTANT NOTE 1: 
AVOID LENGTHY TIME CONSUMING ACTIONS INSIDE THE CALLBACKS. USE THE CALLBACKS TO SET FLAGS TO TRIGGER ACTIONS IN THE LOOP()!
# IMPORTANT NOTE 2: 
IF YOU ARE USING AN ESP8266 YOU STILL NEED TO CALL `Radio.IrqProcess();`. THE BACKGROUND HANDLING IS _**NOT**_ IMPLEMENTED FOR THE ESP8266 (YET).