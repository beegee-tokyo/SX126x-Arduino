Sensor and Gateway example using deep sleep for battery saving
===    
To be used with with PlatformIO as extension on Atom or Microsoft Visual Studio. There are two examples.    
LoRa-TempSensor is an example for a battery optimized sensor node.    
LoRa-Gatyeway is an example for a battery optimized gateway node.

If compiled for a nRF52832
---
If the code is compiled for a nRF52832, the BLE of is activated and the nRF52832 starts advertising with one service. The service is for DFU, the OTA update service of Nordic to update the firmware on the chip.        

Required steps before compiling the example code
---
In order to get this code working you need access to Adafruit IO. You can get an account an [Adafruit IO](https://io.adafruit.com/)

Some explanation for the code
---

A brief explanation of the code can be found in my tutorial [](https://desire.giesecke.tk/index.php/2020/01/18/using-lora-long-range-communication-with-the-semtech-sx1261-1262-1268-modules/)
