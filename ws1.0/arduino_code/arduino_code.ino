/*
- TS2561 for lux
- ldr for analog of light
- DHT22 for temperature and humidity
- add deep sleep (5 mins = 3e8 useconds)
- Binh Nguyen, Oct 22, 2018.
*/

/*______________        _LIBRARIES FOR EACH SENSOR _        _______________*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter(0x23);

#include "TSL2561.h"
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads; 

#include <DHT.h>
#include "Adafruit_Si7021.h"
Adafruit_Si7021 Si7021 = Adafruit_Si7021();

#include <OneWire.h>
#include <DallasTemperature.h>

/*______________        _ WIFI and MQTT INFORMATION  _        _______________*/
#define wifi_ssid "freewifi" //type your WIFI information inside the quotes
#define wifi_password "nopasswd"
#define mqtt_server "192.168.1.x"
#define mqtt_user "awesome_mem" 
#define mqtt_password "but_forget_pw"
#define mqtt_port 1883

/*______________        _ MQTT TOPICS _        _______________*/
#define publish_topic "sensors/balcony/node1"
#define SENSORNAME "node1"

/*______________        _ PIN DEFINITIONS _        _______________*/
#define temt6000 A0
#define ds18b20 D7
#define TEMP_PRECISION 12
OneWire oneWire(ds18b20);
DallasTemperature tempSensor(&oneWire);
DeviceAddress one, two;
float ds_1, ds_2;

#define DHT_PIN D6
DHT dht(DHT_PIN, DHT22);
#define RED_PIN D5
#define BLUE_PIN D3
TSL2561 tsl1(TSL2561_ADDR_FLOAT);
TSL2561 tsl2(TSL2561_ADDR_HIGH);

/*______________        _ GLOBAL VARIABLES_        _______________*/
uint16_t lux_6000, lux_1750, ldr_1, ldr_2;
uint16_t tsl1_f,  tsl2_f;
uint32_t tsl1_lux, tsl2_lux;
float h_Si7021, t_Si7021, h_dht, t_dht, hic;
char message_buff[250]; //took ~210 char now.
const int BUFFER_SIZE = 300;

bool debug = false;

/*______________        _ START SETUP _        _______________*/
void setup() {

  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  Serial.println("\nStarting :::: " + String(SENSORNAME));
  setup_wifi();
  delay(100);
  Wire.begin();
  led(BLUE_PIN, 200, 3);
  led(RED_PIN, 200, 3);
  
  setup_ADS1115();
  delay(500);
  get_ds18b20();
  get_DHT22();
  get_Si7021();
  get_light();
  get_BH1750();
  
  delay(100);
  led(BLUE_PIN, 800, 3);

  if (debug){
     print_Readout();
  } else {
    Serial.println("\nNo printout, debug = false");
    }
  push_data();
//  sendState();
  led(BLUE_PIN, 300,5);
  delay(200);
  ESP.deepSleep(3e8); // 60 millions micro seconds, 300 seconds, 5 minutes;
}

/*______________        _ START MAIN LOOP _        _______________*/
void loop() {
//  this will do not run
}

/*______________        _ PRINT OUT TO SERIAL MONITOR _        _______________*/
void print_Readout(){
  Serial.printf("\nLight: TEMT6000: %d/1023\t", lux_6000);
  Serial.printf("LDR2: %d\t", ldr_2);
  Serial.printf("LDR1: %d\t", ldr_1);
  Serial.printf("TSL2561:1: %d lux\t", tsl1_lux);
  Serial.printf("full: %d\t", tsl1_f);
   Serial.printf("TSL2561:2: %d lux\t", tsl2_lux);
  Serial.printf("full: %d\t", tsl2_f);
  Serial.printf("BH1750: %d lux\t", lux_1750);

  
  Serial.printf("\nTemperature: ds_1 *C: %0.2f\t", ds_1);
  Serial.printf("ds_2 *C: %0.2f\t", ds_2);
  Serial.printf("t_dht:%0.2f \t", t_dht);
  Serial.printf("t_Si7021:%0.2f\t", t_Si7021);
  
  Serial.printf("\nHumidity: h_Si7021: %0.2f\t", h_Si7021);
  
  Serial.printf("h_dht: %0.2f\n", h_dht);
}

/*______________        _FUNCS TO GET DATA _        _______________*/
/*______________DS18B20_______________*/
void get_ds18b20(){
  tempSensor.setWaitForConversion(false); // Don't block the program while the temperature sensor is reading
  tempSensor.begin();                     // Start the temperature sensor
  delay(500);
  if (tempSensor.getDeviceCount() == 0) {
    Serial.printf("No DS18x20 found on %d\n", ds18b20);
    Serial.flush();
    delay(1000);
  }
//    tempSensor.setResolution(one, TEMP_PRECISION);
//  tempSensor.setResolution(two, TEMP_PRECISION);
  delay(100);
  tempSensor.getAddress(one, 0);
  Serial.print("DS18B20 1: ");
  printAddress_ds18b20(one);
  tempSensor.getAddress(two, 1);
  Serial.print("\tDS18B20 2: ");
  printAddress_ds18b20(two);

  tempSensor.requestTemperatures();
    ds_1 = tempSensor.getTempC(one);
  ds_2 = tempSensor.getTempC(two);
  
  while (!checkValue(ds_1)){
    for(int i=0; i<3; i++){
      delay(100);
      ds_1 = tempSensor.getTempC(one);
    }
  }
  while (!checkValue(ds_2)){
    for(int i=0; i<3; i++){
      delay(100);
      ds_2 = tempSensor.getTempC(two);
    }
  }

}

void printAddress_ds18b20(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

bool checkValue(float tempRead){
  if (tempRead == -127 || tempRead >= 50){
    return false;
  }
  return true;
}

/*______________DHT22_______________*/
void get_DHT22(){
  dht.begin();
  delay(3000);
  h_dht = dht.readHumidity();
  t_dht = dht.readTemperature();
  while (isnan(h_dht) || isnan(t_dht)) {
    for (int i=0;i<3;i++){
      delay(3000);
      led(BLUE_PIN, 500, 3);
      h_dht = dht.readHumidity();
      t_dht = dht.readTemperature();
    }
  }
}

void get_Si7021(){
  Si7021.begin();
  delay(100);
  h_Si7021 = Si7021.readHumidity();
  t_Si7021 = Si7021.readTemperature();
  
}

/*______________LIGHTS_______________*/
void get_light(){
  pinMode(temt6000, INPUT);
  delay(100);
  lux_6000 = analogRead(temt6000);
  ldr_1 = int(ads.readADC_SingleEnded(0)*0.125*1024/3300);
  ldr_2 = int(ads.readADC_SingleEnded(1)*0.125*1024/3300);

  if (!tsl1.begin() || !tsl2.begin()) Serial.print("TSL2561s not connected");

  tsl1.setGain(TSL2561_GAIN_0X);  
  tsl1.setTiming(TSL2561_INTEGRATIONTIME_101MS);  
  tsl2.setGain(TSL2561_GAIN_0X);  
  tsl2.setTiming(TSL2561_INTEGRATIONTIME_101MS); 
  delay(100);

  tsl1_f = tsl1.getFullLuminosity();
  uint16_t ir, full;
  ir = tsl1_f >> 16;
  full = tsl1_f & 0xFFFF;
  tsl1_lux = tsl1.calculateLux(full, ir);
  
  tsl2_f = tsl2.getFullLuminosity();
  ir = tsl2_f >> 16;
  full = tsl2_f & 0xFFFF;
  tsl2_lux = tsl2.calculateLux(full, ir);
}

void get_BH1750(){
  lightMeter.begin(BH1750_ONE_TIME_HIGH_RES_MODE);
  lux_1750 = lightMeter.readLightLevel();
}

void setup_ADS1115(){
  ads.setGain(GAIN_ONE);
  ads.begin();

}
/*______________SETUP LED_______________*/
void led(int ledPin, int onTime, int repeat){
  for (int i=0; i<repeat; i++){
    digitalWrite(ledPin, HIGH);
    delay(onTime);
    digitalWrite(ledPin, LOW);
    int offTime = 1000 - onTime;
    delay(offTime);
  }
}

/*______________        _ PROCESS AND SENDING DATA _        _______________*/
/*______________        _ SETUP WIFI _        _______________*/
void setup_wifi() {
  delay(10);
  Serial.printf("Connecting to %s", wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  delay(100); 
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    i++;
    Serial.printf(" %i ", i);
    if (i == 5){
      WiFi.mode(WIFI_STA);
      WiFi.begin(wifi_ssid, wifi_password);
      delay(1000);
    }
    if (i >=10){
      ESP.restart();
      Serial.println("Resetting ESP");
    }
  }
  Serial.printf("\nWiFi connected: \t");
  Serial.print(WiFi.localIP());
  Serial.print("\twith MAC:\t");
  Serial.println(WiFi.macAddress());
}

/*______________        _START SEND STATE _        _______________*/
void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  hic = heatIndex(t_dht, h_dht);
  root["sensor"] = SENSORNAME;
  root["ldr_1"] = ldr_1;
  root["ldr_2"] = ldr_2;
  root["T6000"] = lux_6000;
  root["TSL1_L"] = tsl1_lux;
  root["TSL1_F"] = tsl1_f;
  root["TSL2_L"] = tsl2_lux;
  root["TSL2_F"] = tsl2_f;
  root["BH1750"] = lux_1750;
  
  root["ds_1"] = ds_1;
  root["ds_2"] = ds_2;
  root["tSHT"] = t_Si7021;
  root["tDHT"] = t_dht;
  
  root["hSHT"] = h_Si7021;
  root["hDHT"] = h_dht;
  root["HIC"] = hic;
  
  int l = root.measureLength() + 1;
  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  client.publish(publish_topic, buffer, false);
  Serial.println("To MQTT: L: " + String(l) +" " + String(buffer));
}

/*______________Connect to MQTT_______________*/
void push_data(){
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  led(BLUE_PIN, 500, 3);
  delay(100);
  if (client.connect(SENSORNAME, mqtt_user, mqtt_password)){
    sendState();
    led(BLUE_PIN, 150, 3);
    
  } else{     //attempt to connect client to broker
    for (int _try=0; _try <3; _try ++){
      delay(100);
    
      if (WiFi.status() != WL_CONNECTED) {
        setup_wifi();
      };
      if (client.connect(SENSORNAME, mqtt_user, mqtt_password)){
        sendState();
        led(BLUE_PIN, 150, 3);
        break;
      } else {
        led(RED_PIN, 100, 5); //equal to delay 5 seconds
      }
      if (_try >=3){
        led(RED_PIN, 500, 3);
        Serial.print("\n: Restarting the ESP");
        ESP.reset();
      }
    }
  }
 }  

/*______________        _ START CALLBACK _        _______________*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  sendState();
}

/*_____________________CONVERT HEATINDEX_______________________*/
float heatIndex(float Tc, float RH){
float T = Tc*9/5 + 32;
float HI = -42.379 + 2.04901523*T + 10.14333127*RH - 0.22475541*T*RH \
  - 0.00683783*T*T - 0.05481717*RH*RH + 0.00122874*T*T*RH \
  + 0.00085282*T*RH*RH - 0.00000199*T*T*RH*RH;
return (HI - 32)*5/9.0;
}
