#! /usr/bin/python3
# Binh Nguyen, Noveber 5, 2018
# Listen to MQTT message, sparse data and store in DB

from time import localtime, strftime, sleep, tzset
import sqlite3, json, os
import paho.mqtt.client as mqtt
import numpy as np

# Set local timezone, won't necessary unless to transport data to server
# os.environ['TZ'] = "Asia/Ho_Chi_Minh"
# tzset()

mqtt_topic = "sensors/weather"
mqtt_username = "awesome_mem"
mqtt_password = "but_forget_pw@$5"
dbFile = "weather.db"
# dbFile = "/mnt/mqtt2/weather.db"
mqtt_broker_ip = '192.168.1.x'
logFile = "operation.log"


def takeTime():
    '''for timestamp'''
    return strftime("%Y-%m-%d %H:%M:%S")

def avg_std(data):
    '''calculate everage and standard deviation'''
    avg = np.average(data)
    std = np.std(data)
    return avg, std

def cal_HIC(Tc, RH):
    '''calculate heat index'''
    T = Tc*9/5 + 32
    HI = -42.379 + 2.04901523*T + 10.14333127*RH - 0.22475541*T*RH - 0.00683783*T*T - 0.05481717*RH*RH + 0.00122874*T*T*RH + 0.00085282*T*RH*RH - 0.00000199*T*T*RH*RH
    return (HI - 32)*5/9.0

def writeToDB(sql_command):
    '''store captured data'''
    conn = sqlite3.connect(dbFile)
    cur = conn.cursor()
    cur.execute(sql_command)
    conn.commit()

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    client.subscribe(mqtt_topic)
    print("Connected with result code "+str(rc))
    return None

def on_message(client, userdata, msg):
    time_ = takeTime()
    try:
        payload = json.loads(msg.payload.decode('UTF-8').lower())
        topic = msg.topic
        print('Time: {}\t Topic{}'.format(time_, topic))
        print('Payload in JSON: {}'.format(payload))
        print("_"*20)
        
        if msg.retain == 0:
            if payload['sensor'] =='node1':
                sql_cmd_one = """INSERT INTO {1[sensor]} VALUES ('{0}', {1[ldr_1]}, {1[ldr_2]}, {1[t6000]}, {1[bh1750]}, {1[tsl1_f]}, {1[tsl2_f]},{1[tsl1_l]}, {1[tsl2_l]},
                        {1[ds_1]:.2f}, {1[ds_2]:.2f},{1[tdht]}, {1[tsht]:.2f},
                        {1[hdht]:.2f}, {1[hsht]:.2f}, {1[hic]:.2f});""".format(time_, payload)
    
                writeToDB(sql_cmd_one)             
                print(sql_cmd_one)

                post_data = dict()
                ldr = [payload['ldr_1'], payload['ldr_2']]
                tsl_lux = [payload['tsl1_l'],payload['tsl2_l']]
                tsl_f = [payload['tsl1_f'],payload['tsl2_f']]
                temp = [payload['ds_1'],payload['ds_2'],payload['tdht'],payload['tsht']]
                humid = [payload['hdht'],payload['hsht']]

                # sql_cmd_create = """CREATE TABLE node1 (time DATETIME, ldr_1 INT, ldr_2 INT, temt6000 INT, bh1750 INT, tsl1_f INT, tsl1_lux INT, tsl2_f INT, tsl2_lux INT, ds_1 REAL, ds_2 REAL, tdht REAL, tsht REAL, hdht REAL, hsht REAL, hic REAL);"""

                post_data['Time'] = time_
                post_data['LDR_AVG'], post_data['LDR_STD'] = avg_std(ldr)
                post_data['LUM_AVG'], post_data['LUM_STD'] = avg_std(tsl_f)
                post_data['LUX_AVG'], post_data['LUX_STD'] = avg_std(tsl_lux)
                post_data['TEM_AVG'], post_data['TEM_STD'] = avg_std(temp)
                post_data['HUM_AVG'], post_data['HUM_STD'] = avg_std(humid)
                post_data['HIC_AVG'] = cal_HIC(post_data['TEM_AVG'], post_data['HUM_AVG'])

                sql_cmd_two = """INSERT INTO node1_post VALUES ('{0[Time]}',
                        {0[LDR_AVG]}, {0[LDR_STD]}, {0[LUM_AVG]}, {0[LUM_STD]},
                        {0[LUX_AVG]}, {0[LUX_STD]},{0[TEM_AVG]:.2f}, {0[TEM_STD]:.2f},{0[HUM_AVG]:.2f}, {0[HUM_STD]:.2f},{0[HIC_AVG]:.2f});""".format(post_data)

                # sql_cmd_create = """CREATE TABLE node1_post (time DATETIME,LDR_AVG REAL, LDR_STD REAL, LUM_AVG REAL, LUM_STD REAL, LUX_AVG REAL, LUX_STD REAL, TEM_AVG REAL, TEM_STD REAL,HUM_AVG REAL, HUM_STD REAL, HIC_AVG REAL);"""

                print(sql_cmd_two)
                writeToDB(sql_cmd_two)
        else:
            print("Retained MSG: ", payload)
            

    except Exception as e:
        print("Exception: " + str(e))
        with open(logFile, 'a') as f:
            f.write('{}: {}\n'.format(time_, e))
    print("-"*40)
    return None

def on_disconnect(client, userdata, rc):
    if rc !=0:
        print("Unexpected disconnection!")
    else:
        print("Disconnecting")
    return None


client = mqtt.Client()
client.username_pw_set(username=mqtt_username, password=mqtt_password)
client.connect(mqtt_broker_ip, 1883, 60)
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect
sleep(1)
client.loop_forever()
