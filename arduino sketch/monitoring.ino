#include <WiFi.h>
#include <HTTPClient.h>

// Twilio credentials for sending SMS alerts
const char* twilioSID = "SID";                // Twilio Account SID
const char* twilioAuthToken = "AuthToken";    // Twilio Auth Token
const char* twilioPhoneNumber = "Number";    // Twilio phone number (sender)
const char* alertRecipient = "Number";       // Phone number to receive alerts

// Pin Definitions
#define TRIG_PIN 18          // Trigger pin for ultrasonic sensor
#define ECHO_PIN 19          // Echo pin for ultrasonic sensor
#define TEMP_PIN 34          // Analog pin for temperature sensor
#define RED_LED 25           // Red LED pin
#define ORANGE_LED 26        // Orange LED pin
#define GREEN_LED 27         // Green LED pin
#define BUZZER_PIN 32        // Buzzer pin

// Wi-Fi credentials
const char* ssid = "SSID";                  // Wi-Fi SSID (network name)
const char* password = "Password";          // Wi-Fi password

// Backend Server URL for logging data
const char* serverURL = "http://<server_url>:3001/data";  // Replace <server_url> with your backend's IP or hostname

// Variables for sensor data and states
long distance;              // Distance measured by ultrasonic sensor
float temperature;          // Temperature measured by temperature sensor
bool floodWarning = false;  // Indicates if flood conditions are met
bool alertSent = false;     // Prevents multiple alerts for the same flood event

// Logging interval for backend (15 seconds)
unsigned long lastDataSend = 0;
unsigned long dataSendInterval = 15000;

void setup() {
  // Initialize GPIO pins for sensors, LEDs, and buzzer
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(ORANGE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Start Serial Monitor for debugging
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED && attemptCount < 5) {  // Retry up to 5 times
    delay(1000);
    Serial.println("WiFi Connecting...");
    attemptCount++;
  }

  // Check connection status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi Connection Failed!");
  }
}

void loop() {
  // Measure distance and temperature
  distance = measureDistance();
  temperature = measureTemperature();

  // Update LEDs and buzzer based on current conditions
  updateIndicators();

  // Log data to the backend server at specified intervals
  if (millis() - lastDataSend > dataSendInterval) {
    logDataToMongoDB();
    lastDataSend = millis();
  }

  // Print data to Serial Monitor for debugging
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(1000); // Delay for 1 second to prevent excessive looping
}

long measureDistance() {
  // Trigger the ultrasonic sensor to send a pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the duration of the pulse
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in cm (speed of sound = 0.034 cm/µs)
  return duration * 0.034 / 2;
}

float measureTemperature() {
  // Read analog value from temperature sensor
  int rawValue = analogRead(TEMP_PIN);

  // Convert raw value to temperature in Celsius
  return (rawValue * (3.3 / 4095.0) * 100);  // Assuming LM35 with 3.3V reference
}

void updateIndicators() {
  // Update LEDs and buzzer based on distance thresholds
  if (distance > 100) {
    floodWarning = false;    // No flood risk
    alertSent = false;       // Reset alert flag when conditions normalize
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(ORANGE_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else if (distance > 50) {
    floodWarning = false;    // Moderate flood risk
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(ORANGE_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    floodWarning = true;     // Severe flood risk
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(ORANGE_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    // Send flood alert if not already sent
    if (!alertSent) {
      sendFloodAlert();
      alertSent = true;
    }
  }
}

void logDataToMongoDB() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Connect to the backend server
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    String jsonPayload = "{";
    jsonPayload += "\"temperature\": " + String(temperature) + ",";
    jsonPayload += "\"distance\": " + String(distance) + ",";
    jsonPayload += "\"floodWarning\": " + String(floodWarning ? "true" : "false");
    jsonPayload += "}";

    // Send POST request to the server
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

    // Twilio API URL
    String twilioURL = "https://api.twilio.com/2010-04-01/Accounts/" + String(twilioSID) + "/Messages.json";
    http.begin(twilioURL);
    http.setAuthorization(twilioSID, twilioAuthToken);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Create payload for SMS alert
    String payload = "To=" + String(alertRecipient) + "&From=" + String(twilioPhoneNumber) + 
                     "&Body=Flood Warning! Distance: " + String(distance) + " cm, Temp: " + String(temperature) + "°C.";

    // Send POST request to Twilio
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
