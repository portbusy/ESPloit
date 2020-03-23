/*
 **********    ESPloit firmware 0.0 *************
     provides a webserver for wifi and mqtt broker configuration
     no subscription or publishing topics provided
     AP mode on IP: 192.168.4.1
     AP name: ESPLOIT AP as defined in "init.h"
        #define deviceName "ESPLOIT AP"
     in-line comments

     for more information reference:
     https://github.com/marcoprovolo/ESPloit
*/


#include <FS.h>                 //ESP8266 core
#include <ESP8266WiFi.h>        //ESP8266 core
#include <ESP8266WebServer.h>   //ESP8266 core
ESP8266WebServer server(80);    //ESP8266 core

#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson
#if ARDUINOJSON_VERSION_MAJOR > 5
#error "Requires ArduinoJson 5.13.5 or lower"
#endif

#include <PubSubClient.h>       //https://github.com/knolleary/pubsubclient
WiFiClient espClient;              //initialise a wifi client
PubSubClient client(espClient);    //creates a partially initialised client instance for MQTT

/*** OTA import ***/
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "init.h"
#include "a_root_webpage.h"
#include "b_wifi_webpage.h"
#include "c_mqtt_webpage.h"
#include "d_about_webpage.h"
#include "css.h"
#include "functions.h"

void setup() {
  Serial.begin(115200);
  Serial.println();
  // addyour setup configuration here

  SPIFFS.begin();
  // for debugging purposes
  //formatFS();

  //check a WiFi config file in the FS
  if (loadConfig(wifiNames, wifiValues, wifi_path, NR_WIFI_PARAM)) {
    StartWiFi();
    
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    }); //OTA on start
    
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });/// OTA on end
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    }); // OTA on progress
    
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });// OTA on error
    ArduinoOTA.begin();
  }//if
  else startAP();
  //check an MQTT config file in the FS
  if (STAStart && loadConfig(mqttNames, mqttValues, mqtt_path, NR_MQTT_PARAM)) startMQTT();
  else Serial.println("no MQTT config File or AP mode");

  startServer();
}//setup

void loop() {
  server.handleClient();
  if (mqttInit) nonBlockingMQTTConnection();
  ArduinoOTA.handle();
  /* your code here below*/


}//loop

boolean reconnect() {
  Serial.println("Attempting MQTT connection...");
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str(), mqttValues[3], mqttValues[4])) {
    Serial.printf("\nCONGRATS!!! U'r CONNECTED TO MQTT BROKER!\nstart making your things talk!!!");
    /*** subscribe here below ***/
  }
  return client.connected();
}//reconnect

void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  char msg_buff[100];
  for (int i = 0; i < p_length; i++) {
    msg_buff[i] = p_payload[i];
  }
  msg_buff[p_length] = '\0';
  /***
      here you recieve the payload on the topics you've been subscribed to.
      choose what to do here
   ***/


}//callback
