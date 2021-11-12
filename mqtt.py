

from  paho.mqtt import client as mqtt_client
import json
import mysql.connector



broker = ""

port = 1883
topic = "capilano/basement/air"
client_id = "capilano4"


db = mysql.connector.connect(host="localhost",
                     user="*****",
                     password="*****",
                     database="sensors")

cur = db.cursor()


def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        s = msg.payload.decode()

        d = json.loads(s)

        myTime = d['myTime']
        co2 = d['CO2*']
        tvoc = d['TVOC*']
        h2 = d['H2*']
        eth = d['ETH*']
        no2 = d['NO2']
        nh3 = d['NH3']
        co = d['CO']

        sql = "INSERT INTO sensors(sn_co2, sn_tvoc, sn_h2, sn_eth, sn_no2, sn_nh3, sn_co) VALUES ("
        sql +=  str(co2) + "," + str(tvoc) + "," + str(h2) + "," + str(eth) + "," + str(no2) + "," + str(nh3) + "," + str(co) + ");"
        print(cur.execute(sql))
        print(sql)
        db.commit()



    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()


if __name__ == '__main__':
    run()
