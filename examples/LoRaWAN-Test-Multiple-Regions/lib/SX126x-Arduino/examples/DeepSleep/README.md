PingPong for ArduinoIDE
===    
Example to be used with ArduinoIDE. This examples is to show how to minimize the energy consumption of a LoRa gateway or receiver node. It is written only for the ESP32 and will not work without changes on the ESP8266 nor nRF52.    
Principal of function:
- After reset initialize the SX126x. 
- Start the low power receiving mode of the SX126x **`Radio.SetRxDutyCycle`** which keeps the SX126x chip most of the time in sleep mode and wakes it only long enough to catch the header of a data packet and receives it. Carefully read the SX1261-2 datasheet (SX1261-2_V1.2.pdf), chapter 13.1.7 and the application note for RxDutyCycle (SX1261_AN1200.36_SX1261-2_RxDutyCycle_V1.0.pdf) to understand how the RxDutyCycle mode works and how to calculate the sleep and wake times.
- Then the ESP32 is set into deep sleep mode, waiting for an external wake-up (the interrupt from the SX126x)
- Once the SX126x received a data package it will wake up the ESP32 from deep sleep.
- On wake up the ESP32 reads the data package, processes it (that's for you to implement) and restarts receiving of the SX126x and puts himself into deep sleep again.

Some explanation for the code
---
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
Initialize the LoRa HW after CPU woke up from deep sleep    
When you want to use the deep sleep function of the ESP32 with external wake up source, you do not want to reset and reconfigure the SX126x chip after its IRQ woke up the ESP32. This re-init function sets up only the required definitions for the communication without resetting the ESP32
```
  lora_hardware_re_init(hwConfig);
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
Initialize the radio after CPU woke up from deep sleep    
When you want to use the deep sleep function of the ESP32 with external wake up source, you do not want to reset and reconfigure the SX126x chip after its IRQ woke up the ESP32. Radio.ReInit() sets up only the required communication means resetting the ESP32. 
Radio.IrqProcessAfterDeepSleep() is checking the reason for the wake-up IRQ and calls the event handler
```
  Radio.ReInit(&RadioEvents);

  Radio.IrqProcessAfterDeepSleep();
```

