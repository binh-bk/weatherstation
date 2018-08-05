# /usr/bin/python3
# adopted from: https://github.com/bsmaat/mqtt/blob/master/sql/sqlwriter.py
# Binh Nguyen, August 04, 2018,
from time import localtime, strftime, sleep
import paho.mqtt.client as mqtt
import sqlite3, json

mqtt_topic = 'balcony/weatherstation'
mqtt_username = "johndoe"
mqtt_password = "password"
dbFile = "/path/to/databse/weatherstation.db"
mqtt_broker_ip = '192.168.1.50'
dataTuple = [-1,-1, -1, -1, -1, -1]

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(mqtt_topic)
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    theTime = strftime("%Y-%m-%d %H:%M:%S", localtime())

    result = (theTime + "\t" + str(msg.payload))
    print(msg.topic + ":\t" + result)
    jsonDict = json.loads(msg.payload.decode('utf-8'))

    if msg.topic == mqtt_topic:
        dataTuple[0] = jsonDict['ldr']
        dataTuple[1] = jsonDict['TSL2561']
        dataTuple[2] = jsonDict['ds18b20']
        dataTuple[3] = jsonDict['tSHT21']
        dataTuple[4] = jsonDict['hSHT21']
        dataTuple[5] = jsonDict['HIC']
        #return
        print("Data saved", dataTuple)
    if dataTuple[0] != -1:
        writeToDb(theTime, dataTuple[0], dataTuple[1], dataTuple[2], dataTuple[3],\
                  dataTuple[4],dataTuple[5])
    return

def writeToDb(theTime, ldr, tls, ds18b20, t, h , hic):
    conn = sqlite3.connect(dbFile)
    c = conn.cursor()
     #print("Writing ", )
    c.execute("INSERT INTO weatherstation VALUES (?,?,?,?,?,?, ?, ?)", (None, theTime, ldr, tls, ds18b20, t, h , hic))
    conn.commit()

    global dataTuple
    dataTuple = [-1,-1, -1, -1, -1, -1]

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set(username=mqtt_username, password=mqtt_password)
client.connect(mqtt_broker_ip, 1883, 60)
sleep(1)
client.loop_forever()

