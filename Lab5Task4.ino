#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"

const char *ssid = "WiFi";
const char *pass = "Password";

#define DHTPIN 16        // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT11    // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

char *server = "mqtt://ip_address:8883";

char *subscribeTopic = "weather/esp";
char *publishTopic = "hello/esp";

ESP32MQTTClient mqttClient;  // all params are set later


void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));

  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);

  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  mqttClient.enableDebuggingMessages();

  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(60);
  WiFi.begin(ssid, pass);
  while (WiFi.status()!= WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  if(WiFi.status()==WL_CONNECTED){
    Serial.print("CONNECTED BOIS!");
  }
  WiFi.setHostname("c3test");
  mqttClient.loopStart();
}

void loop() {
  // Delay between measurements.
  delay(delayMS);

  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temperature = event.temperature;
  if (isnan(temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F("°C"));
  }

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;
  if (isnan(humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F("%"));
  }

  mqttClient.publish("weather/temp", String(temperature).c_str());  // Publish temperature
  mqttClient.publish("weather/humidity", String(humidity).c_str());  // Publish humidity
  delay(3000);
}

void onMqttConnect(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client))  // can be omitted if only one client
  {
    mqttClient.subscribe(subscribeTopic, [](const String &payload) {
      Serial.println(String(subscribeTopic)+String(" ")+String(payload.c_str()));
    });

    mqttClient.subscribe("weather/#", [](const String &topic, const String &payload) {
      log_i("%s: %s", topic, payload.c_str());
    });
  }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}
