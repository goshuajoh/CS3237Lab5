#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  20          /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

const char *ssid = "Wifi";
const char *pass = "Password";
char *server = "mqtt://ip_address:8883";

char *subscribeTopic = "weather/esp";
char *publishTopic = "weather/esp";

ESP32MQTTClient mqttClient;

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

void setup() {
  Serial.begin(115200);
  print_wakeup_reason();
  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(60);
  WiFi.begin(ssid, pass);
  while (WiFi.status()!= WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  if(WiFi.status()==WL_CONNECTED){
    Serial.println("CONNECTED BOIS!");
  }
  WiFi.setHostname("c3test");
  mqttClient.loopStart();
  delay(3000);
  mqttClient.publish("weather/hehe", "I have been sent for fun!");

  Serial.println("Message sent, entering deep sleep...");

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {}

void onMqttConnect(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client))  // can be omitted if only one client
  {
    mqttClient.subscribe(subscribeTopic, [](const String &payload) {
      Serial.println(String(subscribeTopic)+String(" ")+String(payload.c_str()));
    });

    mqttClient.subscribe("hello/#", [](const String &topic, const String &payload) {
      log_i("%s: %s", topic, payload.c_str());
    });
  }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}
