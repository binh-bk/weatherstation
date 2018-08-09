/*
 * A simple weather station using a photoresistor a TSL2561 for luminosity
 * DS18B20 to collect temperature, SHT21 for the temperature and the humidity
 * ESP8266 is put in deepleep with interval of 5 minutes (3e8 microseconds)
 * This way the whole system can be powered by a lithium battery
 * Binh Nguyen, August 4th, 2018
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
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "SHT21.h"
SHT21 SHT21;

#include <OneWire.h>
#include <DallasTemperature.h>

/*______________        _ WIFI and MQTT INFORMATION  _        _______________*/
#define wifi_ssid "wifi_SSID" //type your WIFI information inside the quotes
#define wifi_password "wifi_password"
#define mqtt_server "mqtt_ip_address"
#define mqtt_user "esp8266" 
#define mqtt_password "mqtt_password"
#define mqtt_port 1883

/*______________        _ MQTT TOPICS _        _______________*/
#define publish_topic "balcony/esp8266"
#define SENSORNAME "weatherstation"

/*______________        _ PIN DEFINITIONS _        _______________*/
#define ldrPin A0
#define ds18b20 D7  
OneWire oneWire(ds18b20);
DallasTemperature tempSensor(&oneWire);

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

/*______________        _ GLOBAL VARIABLES_        _______________*/
uint16_t lux, ldr;
float h, t, hic, t18;
char message_buff[100];
const int BUFFER_SIZE = 300;

/*______________        _ START SETUP _        _______________*/
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Node named " + String(SENSORNAME));
  setup_wifi();
  delay(100);
  Wire.begin();
  pinMode(ldrPin, INPUT);
  SHT21.begin();
  if(!tsl.begin()) {
    Serial.print("TSL2561 not found");
    while(1);
  }
  delay(100);
  
  ldr = analogRead(ldrPin);
  
  tsl.enableAutoRange(true);  
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);  
  delay(100);
  sensors_event_t event;
  tsl.getEvent(&event);
  if (event.light) lux = event.light;
  else Serial.println("Sensor overload");

  h = SHT21.getHumidity();
  t = SHT21.getTemperature();
  tempSensor.setWaitForConversion(false); 
  tempSensor.begin();                     
  delay(100);
  if (tempSensor.getDeviceCount() == 0) {
    Serial.printf("DS18x20 not found on pin %d\n", ds18b20);
    Serial.flush();
    delay(1000);
  }
  delay(100);
  tempSensor.requestTemperatures();
  t18 = tempSensor.getTempCByIndex(0);
  
  Serial.printf("\nLight: %d lux\t", lux);
  Serial.printf("LDR: %d /1024\t", ldr);
  Serial.printf("T: %0.2f *C\t", t);
  Serial.printf("H:%0.2f \t", h);
  Serial.printf("HIC: %0.2f \t", hic);
  delay(100);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  delay(100);
  
  ESP.deepSleep(3e8); // 300 millions micro seconds, 300 seconds, 5 minutes;
}

/*______________        _ START MAIN LOOP _        _______________*/
void loop() { 
}
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
  Serial.println(WiFi.localIP());
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

/*______________        _START SEND STATE* _        _______________*/
void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  hic = heatIndex(t, h);
  root["sensor"] = SENSORNAME;
  root["ldr"] = ldr;
  root["tsl2561"] = lux;
  root["ds18b20"] = t18;
  root["tsht21"] = t;
  root["hsht21"] = h;
  root["hic"] = hic;

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));
  String sPayload = "";
  root.printTo(sPayload);
  char* cPayload = &sPayload[0u];

  client.publish(publish_topic, buffer, true);
  Serial.println("To MQTT: " + String(buffer));
}

/*______________        _ RECONNECT _        _______________*/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
//     Attempt to connect
    if (client.connect(SENSORNAME, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      sendState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  Serial.println("connected to MQTT server");
}

/*_____________________CONVERT HEATINDEX_______________________*/
float heatIndex(float Tc, float RH){
float T = Tc*9/5 + 32;
float HI = -42.379 + 2.04901523*T + 10.14333127*RH - 0.22475541*T*RH \
  - 0.00683783*T*T - 0.05481717*RH*RH + 0.00122874*T*T*RH \
  + 0.00085282*T*RH*RH - 0.00000199*T*T*RH*RH;
return (HI - 32)*5/9;
}
