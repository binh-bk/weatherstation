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

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(mqtt_topic)
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    theTime = strftime("%Y-%m-%d %H:%M:%S", localtime())

    topic = msg.topic
    payload = json.dumps(msg.payload.decode('utf-8'))
    sql_cmd = sql_cmd = """INSERT INTO weatherdata VALUES ({0}, '{1}',\
        {2[ldr]}, {2[tsl2561]},{2[ds18b20]}, {2[tsht21]},{2[hsht21]})""".format(None, time_, payload)
    writeToDB(sql_cmd)
    print(sql_cmd)
    return None

def writeToDb(sql_cmd):
    conn = sqlite3.connect(dbFile)
    cur = conn.cursor()
    cur.execute(sql_command)
    conn.commit()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set(username=mqtt_username, password=mqtt_password)
client.connect(mqtt_broker_ip, 1883, 60)
sleep(1)
client.loop_forever()
