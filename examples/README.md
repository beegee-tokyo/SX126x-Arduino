# Examples

## Structure
All examples are available for ArduinoIDE and PlatformIO.    
The folders ending in ...Pio are for PlatformIO, other folders are for ArduinoIDE.    

## PingPong
This is the simplest example to show the basic communication between two devices over LoRa.    
This example just sends a data package (PING) and waits for a response (PONG) from another LoRa node.

## LoRaWan
This example shows the basics how to setup a LoRaWan compatible LoRa node.    
It is way more complex than the PingPong example and I strongly suggest you start to understand the basics of LoRa first with the PingPong example.    
In addition you should make yourself familiar with the way LoRaWan works.

## DeepSleep
This is a simple example for a receiver node that is optimized for lower power consumption.   
It is written for the ESP32 and is not tested on the ESP8266 nor on the nRF52.    
It uses the DIO1 interrupt output of the SX126x to wake the ESP32 from deep sleep once a LoRa package was received by the SX126x.    
In addition the example uses the **`Radio.SetRxDutyCycle()`** function which keeps the SX126x most of the time sleeping and wakes it only up for short periods of time to check for incoming data packages.
