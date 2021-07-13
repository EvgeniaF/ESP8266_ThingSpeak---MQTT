#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <SPI.h>

#define DHTTYPE DHT11
int sensePin=2;
DHT HT(sensePin, DHTTYPE);

float humidity;
float temp;

#ifndef STASSID
#define STASSID "" //internet router name
#define STAPSK  "" //internet router password
#endif
const char* ssid     = STASSID;
const char* password = STAPSK;

char mqttUserName[] = "ArduinoMQTTDemo";        // Use any name.
char mqttPass[] = "";      // Change to your MQTT API Key from Account > MyProfile.   
char writeAPIKey[] = "";    // Change to your channel write API key.
long channelID = ;                    // Change to your channel ID.
int fieldNumber = 1;                         // Field to use is publishing to only one field.

static const char alphanum[] ="0123456789"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz";  // For random generation of client ID.

WiFiClient client;                           // Initialize the Wi-Fi client library.

PubSubClient mqttClient(client);    // Initialize the PubSubClient library.
const char* server = "mqtt.thingspeak.com"; 

unsigned long lastConnectionTime = 0; 
const unsigned long postingInterval = 25L * 1000L; // Post data every 25 seconds.



void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect...");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  mqttClient.setServer(server, 1883);   // Set the MQTT broker details.
  HT.begin();
  
  delay(1000);
  
}


void loop() {

 // Reconnect if MQTT client is not connected.
  if (!mqttClient.connected()) 
  {
    reconnect();
  }

  mqttClient.loop();   // Call the loop continuously to establish connection to the server.

  // If interval time has passed since the last connection, publish data to ThingSpeak.
  if (millis() - lastConnectionTime > postingInterval) 
  {
    mqttPublishFeed(); // Publish two values simultaneously.
    // mqttPublishField(fieldNumber); // Use this function to publish to a single field
    
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Temperature: ");
    Serial.println(temp);
  }
}

void reconnect() {
  char clientID[9];

  // Loop until reconnected.
  while (!mqttClient.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Generate ClientID
    for (int i = 0; i < 8; i++) {
        clientID[i] = alphanum[random(51)];
    }
    clientID[8]='\0';

    // Connect to the MQTT broker.
    if (mqttClient.connect(clientID,mqttUserName,mqttPass)) 
    {
      Serial.println("connected");
    } else 
    {
      Serial.print("failed, rc=");
      // Print reason the connection failed.
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttPublishFeed() {
  
  temp = HT.readTemperature(); // Read temperature from DHT sensor.
  humidity = HT.readHumidity();  // Read humidity from DHT sensor.
  
  // Create data string to send to ThingSpeak.
  String data = String("field1=") + String(temp, DEC) + "&field2=" + String(humidity, DEC);
  int length = data.length();
  const char *msgBuffer;
  msgBuffer=data.c_str();
  Serial.println(msgBuffer);
  
  // Create a topic string and publish data to ThingSpeak channel feed. 
  String topicString = "channels/" + String( channelID ) + "/publish/"+String(writeAPIKey);
  length = topicString.length();
  const char *topicBuffer;
  topicBuffer = topicString.c_str();
  mqttClient.publish( topicBuffer, msgBuffer );
  lastConnectionTime = millis();
}
