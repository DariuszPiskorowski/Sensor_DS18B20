#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin D7 on the ESP8266
#define ONE_WIRE_BUS D4

// WiFi credentials
const char* ssid = "AIHome";
const char* password = "*************";

// MQTT server details
const char* mqtt_server = "aihome.local";
const char* mqtt_user = "*****";
const char* mqtt_password = "*****";

// Initialize objects
WiFiClient espClient;
PubSubClient mqttClient(espClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  mqttClient.setServer(mqtt_server, 1883);
  sensors.begin(); // Start the DS18B20 sensor
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectToMQTT();
  }
  mqttClient.loop();
  readAndSendSensorData();
  delay(15000); // Wait 15 seconds before next read
}

void reconnectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP8266ClientDS18B20", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void readAndSendSensorData() {
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  float temp = sensors.getTempCByIndex(0); // Reads the temperature as Celsius
  if (isnan(temp)) {
    Serial.println("Failed to read from DS18B20 sensor!");
    return;
  }

  // Format temperature with one decimal place
  char tempPayload[10];
  snprintf(tempPayload, sizeof(tempPayload), "%.1f", temp);

  mqttClient.publish("BedRoom/Temperature/temperature", tempPayload);
}
