# weatherstation
## 1. Using Multiplex (MUX) to collect multiple analog input through one analog pin on ESP8266
- CD4051 MUX IC acts as a very fast potentiometer that it seqentially reads the analog value of sensors
## 2. Example using 4 temperature sensors (18B20) with an Arduino Nano
- The 18B20 is a simple devide using one-wire bus that is similar to I2C method.  Each devides has one unique address so we can daisy-chain as many sensors as wish. To ouput, in this case the temperature, can be distinguish from others by specifying the address of the target DS18B20.
- This example was setup to demonstate using a Raspberry Pi to collect and log the Print out information making the Raspberry Pi a simple logger. The full tutorial is here: https://www.instructables.com/id/Set-Up-From-Scratch-a-Raspberry-Pi-to-Log-Data-Fro/
<p align="center">
  <img src="https://github.com/binh-bk/weatherstation/blob/master/18B20_multi_reads/18B20.jpg"/>
</p>
## 3. To be added.
