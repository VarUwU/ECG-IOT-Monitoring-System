#include <ESP8266WiFi.h> // Include the ESP8266WiFi library

#define WIFISSID "Alexahome" // Set your WiFi SSID
#define PASSWORD "12345678" // Set your WiFi password
#define TOKEN "BBFF-YKxITsj1YPeTMxw7mq8lvYFBpXnCxD" // Set your Ubidots token
#define MQTT_CLIENT_NAME "myecgsensor" // Set your MQTT client name

#define VARIABLE_LABEL "myecg" // Set the variable label for your ECG sensor data
#define DEVICE_LABEL "esp8266" // Set the device label for your ESP8266 module
#define SENSOR A0 // Set the analog pin for the ECG sensor data

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
char str_sensor[10]; // Space to store the ECG sensor data

WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  // Callback function for MQTT client
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Reconnect to MQTT broker if connection lost
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected to MQTT broker.");
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(". Retrying in 2 seconds...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);

  pinMode(SENSOR, INPUT); // Set the ECG sensor pin as input

  Serial.println();
  Serial.print("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Clear the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Add the variable label to the payload

  float myecg = analogRead(SENSOR);

  /* Convert the ECG sensor data to a string and add it to the payload */
  dtostrf(myecg, 4, 2, str_sensor);
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor);

  Serial.println("Publishing data to Ubidots Cloud...");
  client.publish(topic, payload);
  client.loop();
  delay(10); // Wait for 10 milliseconds
}
