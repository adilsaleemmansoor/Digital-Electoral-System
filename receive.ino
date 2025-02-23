#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "^_Encrypted Air_^";
const char* password = "S3curP@ssW1f1";

// Server URL for sending data
const String serverUrl = "http://192.168.1.5/upload.php"; // Android device IP and port

// Buffer size for incoming serial data
const int bufferSize = 256;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (Serial.available()) {
    char jsonBuffer[bufferSize];
    int bytesRead = Serial.readBytesUntil('\n', jsonBuffer, bufferSize - 1);
    jsonBuffer[bytesRead] = '\0';

    if (strlen(jsonBuffer) == 0 || jsonBuffer[0] != '{') {
      Serial.println("Skipping non-JSON or empty input");
      return;
    }

    StaticJsonDocument<bufferSize> doc;
    DeserializationError error = deserializeJson(doc, jsonBuffer);

    if (!error) {
      int totalVotes = doc["totalVotes"];
      int PTI = doc["PTI"];
      int PPP = doc["PPP"];
      int ANP = doc["ANP"];
      String winner_name = doc["winner"].as<String>();

      sendToAndroidApp(totalVotes, PTI, PPP, ANP, winner_name);
    } else {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
    }
  }

  delay(10000); // Check every 10 seconds
}

void sendToAndroidApp(int totalVotes, int PTI, int PPP, int ANP, String winner_name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> jsonDoc;
    jsonDoc["totalVotes"] = totalVotes;
    jsonDoc["PTI"] = PTI;
    jsonDoc["PPP"] = PPP;
    jsonDoc["ANP"] = ANP;
    jsonDoc["winner"] = winner_name;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    Serial.print("Sending data to server: ");
    Serial.println(jsonString);

    int httpResponseCode = http.POST(jsonString);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Server response: ");
      Serial.println(response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      Serial.print("Error message: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Error: Not connected to Wi-Fi");
  }
}
