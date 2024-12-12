#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Twilio credentials
const char* twilioSID = "your_account_sid"; // Replace with your Twilio SID
const char* twilioAuthToken = "your_auth_token"; // Replace with your Twilio Auth Token
const char* twilioPhoneNumber = "+1234567890"; // Replace with your Twilio phone number
const char* alertRecipient = "+0987654321"; // Replace with the recipient phone number

// Ultrasonic and LM35 pin definitions
#define TRIG_PIN 18
#define ECHO_PIN 19
#define TEMP_PIN 34
#define RED_LED 25
#define ORANGE_LED 26
#define GREEN_LED 27
#define BUZZER_PIN 32

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Wi-Fi credentials
const char* ssid = "SSID";           // Replace with your Wi-Fi SSID
const char* password = "password";   // Replace with your Wi-Fi Password

// Backend Server URL
const char* serverURL = "http://<your_server_ip>/data"; // Replace <your_server_ip> with your server IP or domain

// Variables
long distance;
float temperature;
bool floodWarning = false;
bool alertSent = false; // Prevent multiple alerts for the same event

// Logging interval (30 seconds)
unsigned long lastDataSend = 0;
unsigned long dataSendInterval = 30000; // 30 seconds

void setup() {
  // Initialize GPIO pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(ORANGE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Flood Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);

  // Connect to Wi-Fi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED && attemptCount < 5) {
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connecting...");
    attemptCount++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected!");
    Serial.println("WiFi Connected!");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Failed...");
    Serial.println("WiFi Failed!");
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

  // Display data on LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C   ");

  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm   ");

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
  return (rawValue * (3.3 / 4095.0)) * 100; // Assuming LM35 with 3.3V reference
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
    blinkFloodWarning();
    if (!alertSent) {
      sendFloodAlert();
      alertSent = true; // Prevent multiple alerts
    }
  }
}

void blinkFloodWarning() {
  static unsigned long lastBlinkTime = 0;
  static bool warningState = false;

  if (millis() - lastBlinkTime > 1000) {
    lastBlinkTime = millis();
    warningState = !warningState;

    lcd.setCursor(0, 1);
    if (warningState) {
      lcd.print("Flood Warning!!!");
    } else {
      lcd.print("                "); // Clear message
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
