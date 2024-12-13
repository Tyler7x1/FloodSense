#include <WiFi.h>
#include <HTTPClient.h>

// Twilio credentials
const char* twilioSID = "SID";
const char* twilioAuthToken = "AuthToken";
const char* twilioPhoneNumber = "Number";
const char* alertRecipient = "Number";

// Pin Definitions
#define TRIG_PIN 18
#define ECHO_PIN 19
#define TEMP_PIN 34
#define RED_LED 25
#define ORANGE_LED 26
#define GREEN_LED 27
#define BUZZER_PIN 32

// Wi-Fi credentials
const char* ssid = "SSID";
const char* password = "Password";

// Backend Server URL
const char* serverURL = "http://<server_url>:3001/data";

// Variables
long distance;
float temperature;
bool floodWarning = false;
bool alertSent = false; // Prevent multiple alerts for the same event

// Logging interval (15 seconds)
unsigned long lastDataSend = 0;
unsigned long dataSendInterval = 15000;

void setup() {
  // Initialize GPIO pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(ORANGE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Start Serial Monitor
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED && attemptCount < 5) {
    delay(1000);
    Serial.println("WiFi Connecting...");
    attemptCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi Connection Failed!");
  }
}

void loop() {
  // Measure distance and temperature
  distance = measureDistance();
  temperature = measureTemperature();

  // Update LEDs and buzzer
  updateIndicators();

  // Log data to MongoDB at intervals
  if (millis() - lastDataSend > dataSendInterval) {
    logDataToMongoDB();
    lastDataSend = millis();
  }

  // Print data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(1000); // Delay for 1 second
}

long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

float measureTemperature() {
  int rawValue = analogRead(TEMP_PIN);
  return ((rawValue/8192.0) * 100); // Assuming LM35 with 3.3V reference
}

void updateIndicators() {
  if (distance > 100) {
    floodWarning = false;
    alertSent = false; // Reset alert flag when conditions normalize
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(ORANGE_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else if (distance > 50) {
    floodWarning = false;
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(ORANGE_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    floodWarning = true;
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(ORANGE_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    if (!alertSent) {
      sendFloodAlert();
      alertSent = true; // Prevent multiple alerts
    }
  }
}

void logDataToMongoDB() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    String jsonPayload = "{";
    jsonPayload += "\"temperature\": " + String(temperature) + ",";
    jsonPayload += "\"distance\": " + String(distance) + ",";
    jsonPayload += "\"floodWarning\": " + String(floodWarning ? "true" : "false");
    jsonPayload += "}";

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.println("Data logged successfully: " + String(httpResponseCode));
    } else {
      Serial.println("Error sending data: " + String(http.errorToString(httpResponseCode).c_str()));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void sendFloodAlert() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String twilioURL = "https://api.twilio.com/2010-04-01/Accounts/" + String(twilioSID) + "/Messages.json";
    http.begin(twilioURL);
    http.setAuthorization(twilioSID, twilioAuthToken);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String payload = "To=" + String(alertRecipient) + "&From=" + String(twilioPhoneNumber) + 
                     "&Body=Flood Warning! Distance: " + String(distance) + " cm, Temp: " + String(temperature) + "°C.";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Alert sent successfully: " + String(httpResponseCode));
    } else {
      Serial.println("Failed to send alert: " + String(http.errorToString(httpResponseCode).c_str()));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected for alert!");
  }
}
