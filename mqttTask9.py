import paho.mqtt.client as mqtt
from time import sleep

# Coefficients
# Closed
coef_class_0 = [-0.24630314, -0.187149249]
intercept_class_0 = 20.54311434

# Partial
coef_class_1 = [0.07598246, -1.25580148]
intercept_class_1 = 112.8756424

# Open
coef_class_2 = [0.92881765,  -0.29628888]
intercept_class_2 = 14.65794324

latest_temperature = None
latest_humidity = None

def calculate_decision(temperature, humidity):
    decision_function_class_0 = (coef_class_0[0] * temperature) + (coef_class_0[1] * humidity) + intercept_class_0
    decision_function_class_1 = (coef_class_1[0] * temperature) + (coef_class_1[1] * humidity) + intercept_class_1
    decision_function_class_2 = (coef_class_2[0] * temperature) + (coef_class_2[1] * humidity) + intercept_class_2
    if decision_function_class_0 > decision_function_class_1 and decision_function_class_0 > decision_function_class_2:
        decision = "close"
    elif decision_function_class_1 > decision_function_class_0 and decision_function_class_1 > decision_function_class_2:
        decision = "partial"
    else:
        decision = "open"
    
    return decision

def publish_decision(client):
    global latest_temperature, latest_humidity 
    if latest_temperature is not None and latest_humidity is not None:
        decision = calculate_decision(latest_temperature, latest_humidity)
        print(f"Decision: {decision}")
        client.publish("decision/weather", decision)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("weather/temp")
    client.subscribe("weather/humidity")

def on_message(client, userdata, message):
    global latest_temperature, latest_humidity
    
    topic = message.topic
    payload = str(message.payload.decode())
    print(f"Received message from {topic}: {payload}")
    if topic == "weather/temp":
        latest_temperature = float(payload)
        print(f"Updated temperature: {latest_temperature}")
    elif topic == "weather/humidity":
        latest_humidity = float(payload)
        print(f"Updated humidity: {latest_humidity}")
    publish_decision(client)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 8883, 60)
client.loop_forever()
