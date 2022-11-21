#if defined(ESP8226)
#include <ESP8226WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "ThingsBoard.h"
#include "DHTesp.h"
#include <LiquidCrystal_I2C.h>

#define CURRENT_FIRMWARE_TITLE    "SCU IOT Temperature And Humidity Sensor Monitoring"
#define CURRENT_FIRMWARE_VERSION   "1.0.0"

#define WIFI_SSID           "Wokwi-GUEST"
#define WIFI_PASSWORD       ""

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN               "KY8OQyy9o6e6eU84gw1e"
#define THINGSBOARD_SERVER  "thingsboard.cloud"


// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

const int DHT_PIN = 15;

DHTesp dhtSensor;

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

LiquidCrystal_I2C lcd(0x27, 16, 2);
byte SimbolDerajat[] = {
  B00110,
  B01001,
  B01001,
  B00110,
  B00000,
  B00000,
  B00000,
  B00000,
};

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_SSID,  WIFI_PASSWORD);
  while (WiFi.status()  != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED)  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status()  != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}

void setup() {

      lcd.init();
      lcd.createChar(0, SimbolDerajat);
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print("Temperature &");
      lcd.setCursor(0, 1);
      lcd.print("Humidity");
      delay(4000);
     lcd.clear();

  // initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  Serial.println();
  InitWiFi();
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

  void loop() {
    delay(1000);

     if (WiFi.status() != WL_CONNECTED) {
      reconnect();
    }

      if (!tb.connected()) {
       // Connect to the ThingsBoard
       Serial.print("Connecting to: ");
       Serial.print(THINGSBOARD_SERVER);
       Serial.print(" with token ");
       Serial.println(TOKEN);
       if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
         Serial.println("Failed to connect");
         return;
        }
      }

    Serial.println("Sending data...");

    // Uploads new telemetry to ThingsBoard using MQTT.
    // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
    // for more details

    TempAndHumidity   data = dhtSensor.getTempAndHumidity();
    tb.sendTelemetryInt("temperature", data.temperature);
    tb.sendTelemetryFloat("humidity", data.humidity);
    Serial.print("Temperature : ");
    Serial.print(data.temperature);
    Serial.print(" Humidity : ");
    Serial.println(data.humidity);

  lcd.setCursor(0, 0);
  lcd.print("Temp :" + String(data.temperature));
  lcd.write(0);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Hum  :" + String(data.humidity) + "%");
    tb.loop();

    tb.loop();
  }