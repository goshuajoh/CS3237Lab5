import paho.mqtt.client as mqtt
from time import sleep

def classify_temperature(temp):
   if temp > 30:
       return "Too Hot"
   elif temp < 25:
       return "Too Cold"
   else:
       return "Just Nice"

def on_connect(client, userdata, flags, rc):
   print("Connected with result code: " + str(rc))
   client.subscribe("weather/temp")
   client.subscribe("weather/humidity")
   client.subscribe("decision/weather")

def on_message(client, userdata, message):
	global latest_temperature, latest_humidity
	payload = str(message.payload.decode())
	topic = message.topic
	if topic == "weather/temp":
		latest_temperature = float(payload)
		print(f"Updated temperature: {latest_temperature}")
		classify = classify_temperature(latest_temperature)
		if classify == "Too Hot" :
			client.publish("decision/weather", "open")
			print("hehe")
		elif classify == "Too Cold":
			client.publish("decision/weather", "close")
			print("xd")
		else:
			client.publish("decision/weather", "partial")
			print("lol")
	elif topic == "weather/humidity":
		latest_humidity = float(payload)
		print(f"Updated humidity: {latest_humidity}")
	

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 8883, 60)
client.loop_forever()
