#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "Galaxy A10s9100";
const char* password = "hfha7412";

// ThingSpeak API
const char* server = "http://api.thingspeak.com";
const String apiKey = "CITEP7GRN32QTEVB";

// Sensor pins
const int mq135Pin = A0;  // A0 is the only ADC pin available on ESP8266 for MQ-135
const int trigPin = D5;   // GPIO 14 (D5) for Trig
const int echoPin = D6;   // GPIO 12 (D6) for Echo
const int ledPin = D7;    // GPIO 13 (D7) for LED
const int greenLedPin = D8; // GPIO 15 (D8) for a green LED indicator

// Thresholds
const int gasThreshold = 300;    // Adjust based on calibration
const int distanceThreshold = 10; // Distance in cm to trigger the LED

// Deep sleep parameters
#define uS_TO_S_FACTOR 1000000  // Conversion factor for microseconds to seconds
#define TIME_TO_SLEEP  3600       // Time ESP8266 will go to sleep (in seconds), set to one hour

void setup() {

  // Initialize sensor and LED pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  // Blink green LED three times to indicate the system is ready
  for (int i = 0; i < 3; i++) {
    digitalWrite(greenLedPin, HIGH);
    delay(100);
    digitalWrite(greenLedPin, LOW);
    delay(100);
  }

  // Read sensor values and send to ThingSpeak
  readAndSendData();

  // Prepare for deep sleep
  delay(1000); // Delay to ensure the message is sent
  ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR); // Enter deep sleep
}

void loop() {
  // This will not be executed as the ESP8266 will be in deep sleep
}

void readAndSendData() {
  // Connect to WiFi
  connectToWiFi();

  // Read sensor values
  int gasValue = analogRead(mq135Pin);
  long duration, distance;

  // Trigger ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure distance
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Control LED based on sensor readings
  if (distance < distanceThreshold || gasValue > 300) {
    digitalWrite(ledPin, HIGH); // Turn LED on
  } else {
    digitalWrite(ledPin, LOW);  // Turn LED off
  }

//indicate that sensor is giving expected readings
if(gasValue > 5){
    for (int i = 0; i < 3; i++) {
      digitalWrite(greenLedPin, HIGH);
      delay(300);
      digitalWrite(greenLedPin, LOW);
      delay(300);
  }
}

//indicates that the sensor readings are not as expected (gas sensor)
if (gasValue <= 5){
   for (int i = 0; i < 10; i++) {
    digitalWrite(greenLedPin, HIGH);
    delay(300);
    digitalWrite(greenLedPin, LOW);
    delay(300);
  }
  while(1){
    //Doing nothing, (an infinite loop that executes if the sensor isn't working)
  }
}


  // Send data to ThingSpeak
  sendDataToThingSpeak(gasValue, distance);
}

void connectToWiFi() {
  WiFi.begin(ssid, password);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) { // Timeout after 10 seconds
    delay(500);
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(greenLedPin, HIGH); // Indicate successful Wi-Fi connection
    delay(500);
    digitalWrite(greenLedPin, LOW);
  } else {
    // add debug statement to indicate connection failure
  }
}

void sendDataToThingSpeak(int gasValue, int distance) {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client; // Create a WiFiClient object

    String url = server;
    url += "/update?api_key=" + apiKey;
    url += "&field1=" + String(gasValue);
    url += "&field2=" + String(distance);

    http.begin(client, url); // Use the updated API with WiFiClient and URL
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      }
    else {
      // add debug statement to indicate connection failure
    }
    http.end();
  } 
  else {
   // add debug statement to indicate connection failure
  }
}
