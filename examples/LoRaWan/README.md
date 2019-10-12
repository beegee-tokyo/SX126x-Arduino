LoRaWan example for ArduinoIDE
===    
Example to be used with with ArduinoIDE. This code acts as a LoRaWan sensor node. It sends periodically data to a LoRaWan gateway using the LoRaWan protocol.

If compiled for a nRF52832
---
If the code is compiled for a nRF52832, the BLE of is activated and the nRF52832 starts advertising with two services. One is for DFU, the OTA update service of Nordic to update the firmware on the chip. The second one is a simple BLE-UART service to send debug messages over BLE to a BLE-UART app like the [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal) for Android    

Required steps before compiling the example code
---
In order to get this code working you need access to a LoRaWan gateway. This could be either of    
- a public LoRaWan gateway, like [TheThingsNetwork](https://thethingsnetwork.org/) provides in several big cities all over the world
- a private LoRaWan gateway with multi-channel capability
- a simple and cheap single channel LoRaWan gateway

In addition you need an account at TheThingsNetwork. You need to create an application there and register your device before the data you send over LoRa can be forwarded by the gateway. It is quite simple and there is a good tutorial at [Sparkfun how to setup your account and device](https://learn.sparkfun.com/tutorials/sparkfun-samd21-pro-rf-hookup-guide#registering-your-node).

The region you live in defines the frequency your LoRaWan gateways will use. So you need to setup your device to work on the correct frequency. The region is setup by editing the file `/src/mac/Commissioning.h`.    
In Arduino IDE you can find the file in _**`<arduinosketchfolder>/libraries/SX126x-Arduino/src/mac`**_    
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

Valid regions are
```
REGION_AS923 -> Asia 923 MHz
REGION_AU915 -> Australia 915 MHz
REGION_CN470 -> China 470 MHz
REGION_CN779 -> China 779 MHz
REGION_EU433 -> Europe 433 MHz
REGION_EU868 -> Europe 868 MHz
REGION_IN865 -> India 865 MHz
REGION_KR920 -> Korea 920 MHz
REGION_US915 -> US 915 MHz
```

Some explanation for the code
---

You need to include the LoRaWan library
```cpp
#include <LoRaWan-Arduino.h>
```
Some additional defines are need to get the LoRaWan setup
```cpp
#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE 60										  /**< Maximum number of events in the scheduler queue. */

#define LORAWAN_ADR_ON 1  /**< LoRaWAN Adaptive Data Rate enabled (the end-device should be static here). */
#define LORAWAN_ADR_OFF 0 /**< LoRaWAN Adaptive Data Rate disabled. */

#define LORAWAN_APP_DATA_BUFF_SIZE 64  /**< Size of the data to be transmitted. */
#define LORAWAN_APP_TX_DUTYCYCLE 10000 /**< Defines the application data transmission duty cycle. 10s, value in [ms]. */
#define APP_TX_DUTYCYCLE_RND 1000	  /**< Defines a random delay for application data transmission duty cycle. 1s, value in [ms]. */
#define JOINREQ_NBTRIALS 3			   /**< Number of trials for the join request. */
```
The LoRaWan application works with callbacks. So you do not need to poll the status from your `loop()`. Instead on different events these callbacks are are used to handle the events
```cpp
// Foward declaration
static void lorawan_has_joined_handler(void);
static void lorawan_rx_handler(lmh_app_data_t *app_data);
static void lorawan_confirm_class_handler(DeviceClass_t Class);
static void send_lora_frame(void);
static uint32_t timers_init(void);
```
To setup the device buffers, structures and EUIs and keys need to be defined
```cpp
TimerEvent_t appTimer;														  ///< LoRa tranfer timer instance.
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];			  ///< Lora user application data buffer.
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; ///< Lora user application data structure.

/**@brief Structure containing LoRaWan parameters, needed for lmh_init()
 */
static lmh_param_t lora_param_init = {LORAWAN_ADR_OFF, DR_4, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER};

/**@brief Structure containing LoRaWan callback functions, needed for lmh_init()
*/
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
										lorawan_rx_handler, lorawan_has_joined_handler, lorawan_confirm_class_handler};

uint8_t nodeDeviceEUI[8] = {0x00, 0x95, 0x64, 0x1F, 0xDA, 0x91, 0x19, 0x0B};

uint8_t nodeAppEUI[8] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x01, 0xE1};

uint8_t nodeAppKey[16] = {0x07, 0xC0, 0x82, 0x0C, 0x30, 0xB9, 0x08, 0x70, 0x0C, 0x0F, 0x70, 0x06, 0x00, 0xB0, 0xBE, 0x09};

uint32_t nodeDevAddr = 0x260116F8;

uint8_t nodeNwsKey[16] = {0x7E, 0xAC, 0xE2, 0x55, 0xB8, 0xA5, 0xE2, 0x69, 0x91, 0x51, 0x96, 0x06, 0x47, 0x56, 0x9D, 0x23};

uint8_t nodeAppsKey[16] = {0xFB, 0xAC, 0xB6, 0x47, 0xF3, 0x58, 0x45, 0xC7, 0x50, 0x7D, 0xBF, 0x16, 0x8B, 0xA8, 0xC1, 0x7C};
```
The `setup()` function is very similar to the simple LoRa example. But after initializing the LoRa chip with `lora_hardware_init(hwConfig)` the next steps are to setup the devices EUIs and keys.
```cpp
// Setup the EUIs and Keys
lmh_setDevEui(nodeDeviceEUI);
lmh_setAppEui(nodeAppEUI);
lmh_setAppKey(nodeAppKey);
lmh_setNwkSKey(nodeNwsKey);
lmh_setAppSKey(nodeAppsKey);
lmh_setDevAddr(nodeDevAddr);
```
If you are using a public gateway or your own full fledged multi-channel gateway, you must delete the following line from the code
```cpp
lmh_setSingleChannelGateway(0, DR_3);
```
**_BUT_** if you are starting with a single channel gateway, you need this line.    
When transmitting packets over LoRaWan protocol, the device sends each packet on a different frequency. This is called frequency hoping. Multi channel LoRaWan gateways are able to listen on 8 or 16 different frequencies at the same time, so they receive the package without knowing on which frequency it was sent. This makes this type of gateways quite expensive. On the other side, single channel LoRaWan gateways can only listen on one specific frequency, that makes them much cheaper. So if you use a DIY single channel LoRaWan gateway, your device should disable the frequency hoping and send only on the frequency that the single channel LoRaWan gateway uses.      
The `lmh_setSingleChannelGateway` functions tells the library to disable frequency hoping. The parameters given are the channel number to use and the datarate. Check the [README.md/LoRaWan single channel gateway](https://github.com/beegee-tokyo/SX126x-Arduino/blob/master/README.md) section of this library to learn more about it.

`lorawan_has_joined_handler()`
This function is called after the device has joined the LoRaWan network.

`lorawan_rx_handler()`
This function is called after a package has been received from the gateway

`tx_lora_periodic_handler`
This function is called by a timer to start sending a message. In this example the timer is set to 10 seconds by the `LORAWAN_APP_TX_DUTYCYCLE` define

`send_lora_frame`
This is the function that actually queues up a package to be sent over LoRaWan to the gateway.