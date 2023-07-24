#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

//SIM800L module connections (Serial2)
#define SIM800_RX 16
#define SIM800_TX 17

//NEO-GPS module connections (Serial1)
#define NEOGPS_RX 14
#define NEOGPS_TX 12

#define sim800 Serial2
#define neogps Serial1

//GPRS credential
const char apn[] = "TM";

//Json object. DynamicJsonDocument allocates in the heap.
DynamicJsonDocument doc(140);

// MQTT broker details
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* sensor1_topic = "TX0001AA";

// SIM800L module
TinyGsm modem(sim800);
TinyGsmClient gsmClient(modem);
bool isGprsConnect = false;
bool isApnConnected = false;

// The TinyGPSPlus object
TinyGPSPlus gps;
bool isGpsConnected = false;
unsigned long previousCharProcessed = 0;
unsigned long previousGpsRead = 0;

//MQTT
PubSubClient client(gsmClient);

double lat = 0, lng = 0;

void setup() {
  // Start serial communication
  Serial.begin(115200);
  delay(1000);
  // Start SIM800L module
  Serial.println(F("Starting SIM800L module..."));
  sim800.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print(F("Modem Info: "));
  Serial.println(modemInfo);

  Serial.print(F("Connecting to apn: "));
  Serial.print(apn);
  if (isApnConnected = modem.gprsConnect(apn)) {
    Serial.println(F(" success"));
  } else {
    Serial.println(F(" failed"));
  }

  if (isGprsConnect = modem.isGprsConnected()) {
    Serial.println(F("GPRS connected"));
  } else {
    Serial.println(F("GPRS not connected"));
  }

  delay(1000);

  // Start NEOGPS module
  Serial.println(F("Starting NEOGPS module..."));
  neogps.begin(9600, SERIAL_8N1, NEOGPS_RX, NEOGPS_TX);
  delay(1000);

  // Set MQTT broker server and port
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  previousGpsRead = millis();
  do {
    while (neogps.available()) {
      gps.encode(neogps.read());
    }
  } while (millis() - previousGpsRead < 1000);

  if (gps.charsProcessed() == previousCharProcessed && millis() - previousGpsRead > 5000) {
    Serial.println(F("No GPS detected: check wiring."));
    isGpsConnected = false;
    delay(2000);
  } else {
    isGpsConnected = true;
    previousCharProcessed = gps.charsProcessed();
  }

  if (isGpsConnected) {
    if (gps.location.isValid()) {
      //First run or check if there's a three meters threshold from last coordinates
      if (gps.distanceBetween(lat, lng, gps.location.lat(), gps.location.lng()) > 5) {

        lat = gps.location.lat();
        lng = gps.location.lng();

        doc["device"] = "ESP32";
        doc["sensor"] = "NEO-6M GPS";
        doc["lat"] = lat;
        doc["lng"] = lng;
        doc["last_update"] = gps.date.year() + String("/") + gps.date.month() + String("/") + gps.date.day() + " " + gps.time.hour() + ":" + gps.time.minute() + ":" + gps.time.second();
        doc.garbageCollect();  //https://arduinojson.org/v6/api/jsondocument/garbagecollect/

        char mqtt_message[128];
        serializeJson(doc, mqtt_message);

        if (isGprsConnect) {
          publishMessage(sensor1_topic, mqtt_message, true);
        } else {
          Serial.println(F("Cannot publish. GPRS connection lost"));
        }
      }
    }
  }

  if (!isApnConnected) {
    Serial.print(F("Connecting to apn: "));
    Serial.println(apn);
    isApnConnected = modem.gprsConnect(apn);
  }

  if ((isGprsConnect = modem.isGprsConnected()) && !client.connected()) {
    reconnect();
  }

  client.loop();
}

void reconnect() {
  unsigned long start = millis();
  // Loop until we’re reconnected
  while (!client.connected() && millis() - start < 30000) {
    Serial.print(F("Attempting MQTT connection…"));
    String clientId = "ESP32Client-";  // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println(F("connected"));
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));  // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishMessage(const char* topic, String payload, boolean retained) {
  if (client.publish(topic, payload.c_str(), true))
    Serial.println("Message publised[" + String(topic) + "]: " + payload);
  else
    Serial.println(F("publishMessage error"));
}
