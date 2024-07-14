#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

// Define DEBUG_SERIAL to enable or disable serial debug
//#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// WiFi credentials
const char* ssid = "Redmi Note 9";
const char* password = "ngungungu";

WiFiClient client;
HTTPClient http;

// NTP Server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600*7, 60000); // 3600 là GMT offset, 60000 là thời gian update mỗi 60 giây

// Variables
const String apiKeyValue = "tPmAT5Ab3j7F9";
const String sensorName = "sensor";
const String sensorLocation = "location";
const char* serverTime = "http://192.168.190.143/esp-time-duration.php"; // Replace with your server URL
const char* serverUrl = "http://192.168.190.143/esp-post-data.php?api_key=";
const char* serverPumpStatusUrl = "http://192.168.190.143/esp-get-pump-status.php"; // Replace with your server URL

float Humidity = 60;
float Temperature = 32;
int soilMoistureValue = 300;
bool pumpState = false;
long distanceCm = 10;
bool statusRelay = false;
bool controlRelay = false;

void setup() {
  // Start serial communication
    Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DEBUG_PRINTLN("Connecting to WiFi...");
  }
  DEBUG_PRINTLN("Connected to WiFi");

  // Start NTP client
  timeClient.begin();
}

void loop() {
  // Update NTP client to get the latest time
  timeClient.update();
  // Read data from Serial JSON
  if (Serial.available() > 50) {
    String jsonData = Serial.readStringUntil('}') + "}";
    Serial.println("Received JSON: " + jsonData);

    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, jsonData);


    if (error) {
      Serial.println(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    Humidity = doc["Humidity"];
    Temperature = doc["Temperature"];
    soilMoistureValue = doc["soilMoistureValue"];
    pumpState = doc["pumpState"];

    DEBUG_PRINT("Humidity: ");
    DEBUG_PRINTLN(Humidity);
    DEBUG_PRINT("Temperature: ");
    DEBUG_PRINTLN(Temperature);
    DEBUG_PRINT("Soil Moisture Value: ");
    DEBUG_PRINTLN(soilMoistureValue);
    DEBUG_PRINT("Pump State: ");
    DEBUG_PRINTLN(pumpState);
    

  }

  
    // Send data to the server
    getHttp();
  // Send pumpState string to Arduino Uno via Serial
  Serial.print(String(statusRelay||controlRelay));


  // Get the current time
  String formattedTime = timeClient.getFormattedTime();
  DEBUG_PRINTLN("Current time: " + formattedTime);



  // Get scheduled time and duration
  getScheduledTimeAndControlPump();

  // Get remote pump status and control pump
  getRemotePumpStatus();
  
  // Delay between each loop iteration
  delay(50);
}

void getHttp() {
  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    String url = serverUrl + apiKeyValue +
                 "&sensor=" + sensorName + "&location=" + sensorLocation +
                 "&value1=" + String(Humidity) + "&value2=" + String(Temperature) +
                 "&value3=" + String(pumpState) + "&value4=" + String(soilMoistureValue) +
                 "&value5=" + String(pumpState);
    DEBUG_PRINTLN(url);
    http.begin(client, url);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      DEBUG_PRINTLN("Data pushed successfully");
    } else {
      DEBUG_PRINTLN("Push error: " + String(httpCode));
    }

    http.end();
    delay(50);
  } else {
    DEBUG_PRINTLN("WiFi Disconnected");
  }
}

void getScheduledTimeAndControlPump() {
  if (WiFi.status() == WL_CONNECTED) {

    http.begin(client, serverTime); // Specify the URL
    int httpCode = http.GET(); // Make the request

    if (httpCode > 0) { // Check for the returning code
      String payload = http.getString();
      DEBUG_PRINTLN("Response: " + payload);

      // Split the data
      int separatorIndex = payload.indexOf('/');
      if (separatorIndex != -1) {
        String time = payload.substring(0, separatorIndex);
        String durationStr = payload.substring(separatorIndex + 1);

        DEBUG_PRINTLN("Time: " + time);
        DEBUG_PRINTLN("Duration: " + durationStr);

        // Further split the time into hours and minutes
        int colonIndex1 = time.indexOf(':');
        int colonIndex2 = time.indexOf(':', colonIndex1 + 1);

        if (colonIndex1 != -1 && colonIndex2 != -1) {
          String hoursStr = time.substring(0, colonIndex1);
          String minutesStr = time.substring(colonIndex1 + 1, colonIndex2);
          int duration = durationStr.toInt();

          int hours = hoursStr.toInt();
          int minutes = minutesStr.toInt();

          DEBUG_PRINT("Hours: ");
          DEBUG_PRINTLN(hours);
          DEBUG_PRINT("Minutes: ");
          DEBUG_PRINTLN(minutes);
          DEBUG_PRINT("Duration: ");
          DEBUG_PRINTLN(duration);

          // Get current time from NTP client
          int currentHours = timeClient.getHours();
          int currentMinutes = timeClient.getMinutes();

          DEBUG_PRINT("Current Hours: ");
          DEBUG_PRINTLN(currentHours);
          DEBUG_PRINT("Current Minutes: ");
          DEBUG_PRINTLN(currentMinutes);

          // Calculate the end time
          int endMinutes = minutes + duration;
          int endHours = hours + (endMinutes / 60);
          endMinutes = endMinutes % 60;

          DEBUG_PRINT("End Hours: ");
          DEBUG_PRINTLN(endHours);
          DEBUG_PRINT("End Minutes: ");
          DEBUG_PRINTLN(endMinutes);

          bool isWithinTimeFrame = true;
          // Check if current time is within start time and end time
          if (currentHours < hours
              || currentHours > endHours
              || (currentHours == hours && currentMinutes < minutes)
              || (currentHours == endHours && currentMinutes > endMinutes)) {
            isWithinTimeFrame = false;
          }

          if (isWithinTimeFrame) {
            DEBUG_PRINTLN("Current time is within the scheduled time frame.");
            controlRelay = true;
          } else {
            DEBUG_PRINTLN("Current time is not within the scheduled time frame.");
            controlRelay = false;
          }

        } else {
          DEBUG_PRINTLN("Invalid time format");
        }
      } else {
        DEBUG_PRINTLN("Invalid format");
      }
    } else {
      DEBUG_PRINTLN("Error on HTTP request");
    }

    http.end(); // Free the resources
  } else {
    DEBUG_PRINTLN("WiFi not connected");
  }
}
void getRemotePumpStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(client, serverPumpStatusUrl); // Specify the URL
    int httpCode = http.GET(); // Make the request

    if (httpCode > 0) { // Check for the returning code
      String payload = http.getString();
      DEBUG_PRINTLN("Pump Status Response: " + payload);

      if (payload == "1") {
        DEBUG_PRINTLN("Turning pump ON remotely");
        statusRelay = true;
      } else if (payload == "0") {
        DEBUG_PRINTLN("Turning pump OFF remotely");
        if(controlRelay == false)statusRelay = false;
      } else {
        DEBUG_PRINTLN("Invalid pump status received");
      }
    } else {
      DEBUG_PRINTLN("Error on HTTP request");
    }

    http.end(); // Free the resources
  } else {
    DEBUG_PRINTLN("WiFi not connected");
  }
}