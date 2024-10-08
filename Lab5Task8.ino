#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <ESP32Servo.h>

const char *ssid = "Wifi";
const char *pass = "Password";

Servo myservo;

const int CLOSE = 0;
const int SLIGHTOPEN = 90;
const int OPEN = 180;

int servoPin;
int state = 1;

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  20          /* Time ESP32 will go to sleep (in seconds) */

#define DHTPIN 16        // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT11    // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

char *server = "mqtt://ip_address:8883";

char *subscribeTopic = "decision/weather";
char *publishTopic = "hello/esp";

ESP32MQTTClient mqttClient;  // all params are set later

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void decision(String response) {
   if (response == "open" && state !=1 ) {
     myservo.write(OPEN);
     state = 1;
     delay(1000);
   } else if (response == "close" && state !=0) {
     myservo.write(CLOSE);
     state = 0;
     delay(1000);
   } else if(response == "partial" && state!=2) {
     myservo.write(SLIGHTOPEN);
     state = 2;
     delay(1000);
} }

void Sleep(){

  Serial.println("Message sent, entering deep sleep...");

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void setup() {
  Serial.begin(115200);

  print_wakeup_reason();

  myservo.setPeriodHertz(50);
  myservo.attach(18, 1000, 2000);
  myservo.write(OPEN);

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
  } 
  else {
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F("°C"));
  }

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;
  if (isnan(humidity)) {
    Serial.println(F("Error reading humidity!"));
  } 
  else {
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
    mqttClient.subscribe("decision/weather", [](const String &payload) {
      decision(payload);
      delay(100);
      Sleep();  // Go to sleep after processing the response
  });
  }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}
