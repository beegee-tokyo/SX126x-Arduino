# PingPong for PlatformIO

Example to be used with with PlatformIO as extension on Atom or Microsoft Visual Studio. It is a simple Ping - Pong between two LoRa nodes. One node sends a PING and the other node responds with a PONG. It is a simple application that can be usefull to test the range your LoRa setup can reach.
Principal of function:
- Listen for incoming packets.
- If receive timeout (3s) occurs start a CAD (channel activity detection)
- - If the CAD shows no activity, send a PING package
- - Restart listening for incoming packets
- If a PING package was received start a CAD (channel activity detection)
- - If the CAD shows no activity, send a PONG response
- - Restart listening for incoming messages
- If a PONG package was received start a CAD (channel activity detection)
- - If the CAD shows no activity, send the next PING package
- - Restart the listening for incoming packets

BLE of is activated and the RAk4631 starts advertising with two services. One is for DFU, the OTA update service of Nordic to update the firmware on the chip. The second one is a simple BLE-UART service to send debug messages over BLE to a BLE-UART app like the [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal) for Android    

## Some explanation for the code

This code is made for the RAK4631.
It uses the specific initialization routine for the RAK4631 and no pin definitions are required.

Initialize the LoRa HW
```
  lora_rak4630_init();
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
Start listening for packets
```
  Radio.Rx(RX_TIMEOUT_VALUE);
```

## Using semaphores for low power consumption'

Instead of running the loop endless, a semaphore is used to wake up the MCU on a LoRa event.    
The loop waits for the semaphore to released and then handles the event.