LORAWAN OTAA esp32+e22 for PlatformIO
===    
Example to be used with PlatformIO. This example is to show how to connect to the Helium network or TTN
by OTAA activation. It is written only for the ESP32 and will not work without changes on the ESP8266 nor nRF52. The LoRa module used is the e22-900m30s but it can be used with any e22 module with minor changes.

Required steps before compiling the example code
---
In order to get this code working you need an reacheable to a LoRaWan gateway. This could be either of    
- [Helium](https://www.helium.com)
- [TheThingsNetwork](https://thethingsnetwork.org/)

In addition you need an account to any of both services and create a device or application:
- For Helium you can follow [this guide](https://docs.helium.com/use-the-network/console/quickstart/)
- For TTN you can follow [this guide](https://www.thethingsnetwork.org/docs/devices/registration/)

From both services you will need 3 keys for linking your lora device: nodeDeviceEUI, nodeAppEUI and nodeAppKey.

The region you live in defines the frequency your LoRaWan gateways will use. So you need to setup your device to work on the correct frequency. You can searh for the frequencies used in your region in [this 
resource](https://docs.helium.com/lorawan-on-helium/frequency-plans/).

Valid regions in this library are:
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

Pinout of the esp32 and the LoRa module
---

You need the connect all the e22 pins to the esp32 pins excepting the ant pin. Be sure of having an antenna
atacched to the module.

| e22          	| esp32 	|
|--------------	|-------	|
| NRST         	| 4     	|
| NSS          	| 5     	|
| SCK          	| 18    	|
| MISO         	| 19    	|
| DIO_1        	| 21    	|
| BUSY         	| 22    	|
| MOSI         	| 23    	|
| TXEN         	| 26    	|
| RXEN         	| 27    	|
| All VCC pins 	| 3.3   	|
| All GND pins 	| GND   	|
| ant          	| none  	|

Some explanation for the code
---
You need to include the LoRaWan library
```cpp
#include <LoRaWan-Arduino.h>
```
Some additional defines are need to get the LoRaWan setup
```cpp
#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE  // Maximum size of scheduler events
#define SCHED_QUEUE_SIZE 60  // Maximum number of events in the scheduler queue

#define LORAWAN_APP_DATA_BUFF_SIZE 256  // Size of the data to be transmitted
#define LORAWAN_APP_TX_DUTYCYCLE 5000  // Defines the application data transmission duty cycle. 10s, value in [ms]
#define APP_TX_DUTYCYCLE_RND 1000  // Defines a random delay for application data transmission duty cycle. 1s, value in [ms]
#define JOINREQ_NBTRIALS 3  // Number of trials for the join request
```

The LoRaWan application works with callbacks. So you do not need to poll the status from your `loop()`. Instead on different events these callbacks are are used to handle the events.

```cpp
// Foward declaration
/** LoRaWAN callback when join network finished */
static void lorawan_has_joined_handler(void);
/** LoRaWAN callback when join network failed */
static void lorawan_join_fail_handler(void);
/** LoRaWAN callback when data arrived */
static void lorawan_rx_handler(lmh_app_data_t *app_data);
/** LoRaWAN callback after class change request finished */
static void lorawan_confirm_class_handler(DeviceClass_t Class);
/** LoRaWAN callback after class change request finished */
static void lorawan_unconfirm_tx_finished(void);
/** LoRaWAN callback after class change request finished */
static void lorawan_confirm_tx_finished(bool result);
/** LoRaWAN Function to send a package */
static void send_lora_frame(void);
static uint32_t timers_init(void);
```

To setup the device buffers, structures and EUIs and keys need to be defined

```cpp
TimerEvent_t appTimer;  // LoRa tranfer timer instance
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];  // Lora user application data buffer
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};  // Lora user application data structure

/**@brief Structure containing LoRaWan parameters, needed for lmh_init() */
static lmh_param_t lora_param_init = {LORAWAN_ADR_OFF, DR_3, LORAWAN_PUBLIC_NETWORK, 
                                        JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER,
                                        LORAWAN_DUTYCYCLE_OFF};

/**@brief Structure containing LoRaWan callback functions, needed for lmh_init() */
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
										lorawan_rx_handler, lorawan_has_joined_handler, 
										lorawan_confirm_class_handler, lorawan_join_fail_handler,
										lorawan_unconfirm_tx_finished, lorawan_confirm_tx_finished};
```

The EUI and keys need to be in msb (big endian).

```cpp
uint8_t nodeDeviceEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t nodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t nodeAppKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
```

The `setup()` function is very similar to the simple LoRa example. But after initializing the LoRa chip with `lora_hardware_init(hwConfig)` the next steps are to setup the devices EUIs and keys all in msb (big endian).

```cpp
lmh_setDevEui(nodeDeviceEUI);
lmh_setAppEui(nodeAppEUI);
lmh_setAppKey(nodeAppKey);
```

Lastly you need to select `CLASS_C` and your region bandwith.

```cpp
lmh_init(&lora_callbacks, lora_param_init, doOTAA, CLASS_C, LORAMAC_REGION_US915);
```

For Helium US915 region you need to select subband 2. Other subbands configurations can be found in the [LoRaMacHelper file](https://github.com/beegee-tokyo/SX126x-Arduino/blob/1c28c6e769cca2b7d699a773e737123fc74c47c7/src/mac/LoRaMacHelper.cpp)
 The `lmh_setSingleChannelGateway` functions tells the library to disable frequency hoping. The parameters given are the channel number to use and the datarate. Check the [README.md/LoRaWan single channel gateway](https://github.com/beegee-tokyo/SX126x-Arduino/blob/master/README.md) section of this library to learn more about it.
