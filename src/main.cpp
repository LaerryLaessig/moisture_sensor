#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include <wifi.h>
#include <influx.h>

#define SensorPin A0

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
Point sensor("moisture_status");

void setup() { 
  Serial.begin(115200);
  // Wait for serial to initialize.
  while(!Serial) { }
  Serial.setTimeout(2000);
  client.setInsecure();
  // Connected to WiFi

  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
  }
  Serial.println("");
  Serial.println("connected!");

  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
    // Clear fields for reusing the point. Tags will remain the same as set above.
    sensor.clearFields();
    sensor.addField("rssi", WiFi.RSSI());
    sensor.addField("moisture", analogRead(SensorPin));
    boolean upload_succes = true;
    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());
  
    // Check WiFi connection and reconnect if needed
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Wifi connection lost");
      upload_succes = false;
    }
  
    // Write point
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
      upload_succes = false;
    }

    if (upload_succes) {
      Serial.print("Sleep for 1h");
      ESP.deepSleep(3600e6);
      }
    
    }
