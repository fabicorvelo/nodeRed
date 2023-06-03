#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#define LIGHT_SENSOR_PIN 35
#define ANALOG_THRESHOLD 900

const int ledPin = 4;
const int ledInt = 15;

const char* ssid = "abc";
const char* password = "abc123";
const char* mqtt_server = "abc.duckdns.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

Adafruit_BME280 bme;
float temperature = 0;
float humidity = 0;
float luminosity = 0;

void setup() {
  Serial.begin(115200);
  if (!bme.begin(0x76)) {
    Serial.println("Unable to locate a working BME280 sensor; check wiring!");
    while (1)
      ;
  }
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(ledPin, OUTPUT);
  pinMode(ledInt, OUTPUT);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Wi-Fi connection to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "signals/nodectd") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    } else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Trying to connect to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println(" Connected.");
      client.subscribe("signals/nodectd");
    } else {
      Serial.print(" Failed, erro = ");
      Serial.print(client.state());
      Serial.println(" Retry in five seconds.");
      delay(5000);
    }
  }
  Serial.println("");
  Serial.println("------------");
  Serial.println("");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    int luminosity = analogRead(LIGHT_SENSOR_PIN);
    temperature = bme.readTemperature();
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("sensors/temperature", tempString);
    humidity = bme.readHumidity();
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("sensors/humidity", humString);
    char lumString[8];
    dtostrf(luminosity, 1, 2, lumString);
    Serial.print("luminosity: ");
    Serial.println(lumString);
    client.publish("sensors/luminosity", lumString);
    Serial.println("");
    Serial.println("------------");
    Serial.println("");
    if (luminosity < ANALOG_THRESHOLD)
      digitalWrite(ledInt, HIGH);
    else
      digitalWrite(ledInt, LOW);
  }
}
