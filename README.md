# weatherstation (recent first)
## 4. Home Weather Station (v1.0) with double sensors, more data QA/QC:
- Measure three environmental parameters: light intensity, temperature and humidity
- Each parameter is measured by at least two sensors
- Data is dislayed with standard deviation

## 1. Using Multiplex (MUX) to collect multiple analog inputs through one analog pin on ESP8266
- CD4051 MUX IC acts as a very fast potentiometer that it seqentially reads the analog value of sensors
## 2. Example using 4 temperature sensors (18B20) with an Arduino Nano
- The 18B20 is a simple devide using one-wire bus that is similar to I2C method.  Each devides has one unique address so we can daisy-chain as many sensors as wish. To ouput, in this case the temperature, can be distinguish from others by specifying the address of the target DS18B20.
- This example was setup to demonstate using a Raspberry Pi to collect and log the Print out information making the Raspberry Pi a simple logger. <a href='https://www.instructables.com/id/Set-Up-From-Scratch-a-Raspberry-Pi-to-Log-Data-Fro'>The full tutorial is posted on Instructables.com</a> 
<p align="center">
  <img src="https://github.com/binh-bk/weatherstation/blob/master/18B20_multi_reads/18B20.jpg"/>
</p>

## 3. A complete setup of weather station with:
- a WEMOS ESP8266 using deepsleep mode (with 5 minute interval)
- collect light, temperature and humidity, posts data to the MQTT broker
- sparse out data using Paho-MQTT-Python, store in sqlite database, and display data in a web-based platform
- <a href='https://www.instructables.com/id/Weather-Station-ESP8266-With-Deep-Sleep-SQL-Graphi/'> a detailed writeup on Instructables.com</a> 
### Schematics:
<p align="center">
  <img src="https://github.com/binh-bk/weatherstation/blob/master/esp8266_deepsleep_Aug4/esp8266_deepsleep_ws.png"/>
</p>

### Screenshot
<p align="center">
  <img src="https://github.com/binh-bk/weatherstation/blob/master/esp8266_deepsleep_Aug4/screencapture-mqtt-9999-2018-08-04-23_00_44.png"/>
</p>
## 4. To be added.
