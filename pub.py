import paho.mqtt.client as mqtt
from time import sleep

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    print("Waiting for 2 seconds")
    sleep(2)

    print("Sending message.")
    client.publish("foo", "Reverse Uno Time!")


client = mqtt.Client()
client.on_connect = on_connect
client.connect("localhost", 8883, 60)
client.loop_forever()
