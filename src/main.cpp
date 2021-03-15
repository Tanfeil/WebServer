#include <Arduino.h>
#include <WiFi.h>
#include <WiFiGeneric.h>
#include <WiFiType.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <FS.h>
#include "../.pio/libdeps/esp32dev/ESP Async WebServer/src/ESPAsyncWebServer.h"
#include "../.pio/libdeps/esp32dev/ESP Async WebServer/src/AsyncJson.h"
#include "../.pio/libdeps/esp32dev/ArduinoJson/ArduinoJson.h"
#include "pass.h"

int status;

String dayHours;
String dayMinutes;
String nightHours;
String nightMinutes;
String nightState;
String dayState;

Preferences preferences;

AsyncWebServer server(80);
AsyncCallbackJsonWebHandler* handler;

void updateSettings(){
    nightHours = preferences.getString("nightHours", "-1");
    nightMinutes = preferences.getString("nightMinutes", "-1");
    dayHours = preferences.getString("dayHours", "-1");
    dayMinutes = preferences.getString("dayMinutes", "-1");
    dayState = preferences.getString("dayState", "-1");
    nightState = preferences.getString("nightState", "-1");
}

void setUpdates(){
    preferences.putString("dayHours", dayHours);
    preferences.putString("dayMinutes", dayMinutes);
    preferences.putString("nightHours", nightHours);
    preferences.putString("nightMinutes", nightMinutes);
    preferences.putString("nightState", nightState);
    preferences.putString("dayState", dayState);
}

DynamicJsonDocument getSettings(){
    DynamicJsonDocument ret(1024);
    ret["hoursDay"] = dayHours;
    ret["minutesDay"] = dayMinutes;
    ret["hoursNight"] = nightHours;
    ret["minutesNight"] = nightMinutes;
    ret["stateNight"] = nightState;
    ret["stateDay"] = dayState;
    return ret;
}

void printSetting(){
    Serial.println("Time: " + dayHours + ":" + dayMinutes + "\tState:" + dayState + "\nTime: " + nightHours + ":" + nightMinutes + "\tState:" +  nightState);
}

void setSettings(DynamicJsonDocument data){
    dayHours = data["hoursDay"].as<String>();
    dayMinutes = data["minutesDay"].as<String>();
    nightHours = data["hoursNight"].as<String>();
    nightMinutes = data["minutesNight"].as<String>();
    nightState = data["stateNight"].as<String>();
    dayState = data["stateDay"].as<String>();
    printSetting();
    setUpdates();
}

void printWifiData() {
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    Serial.println(ip);

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);

}

void wifiConnect(){
    while (!Serial) { ; // wait for serial port to connect. Needed for native USB port only
    }

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(5000);
        if(status == WL_CONNECT_FAILED) Serial.println("Connection failed");
        if(status == WL_NO_SSID_AVAIL) Serial.println("No SSID available");
        if(status == WL_NO_SHIELD) Serial.println("WiFi shield not present");
    }

    // you're connected now, so print out the data:
    Serial.print("You're connected to the network");
    printWifiData();
}

void server_on(){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/ws.html", "text/html");
    });
    server.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/bootstrap.min.css", "text/css");
    });
    server.on("/src/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/styles.css", "text/css");
    });
    server.on("/background.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/background.png", "image/png");
    });
    server.on("/src/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/jquery.min.js", "text/javascript");
    });
    server.on("/src/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/bootstrap.min.js", "text/javascript");
    });
    server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("get")){
            Serial.println("Got HTTP_GET");
            request->send(200,"application/json", getSettings().as<String>());
        }else request->send(400);
    });
    server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request){
        //nothing and dont remove it
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        Serial.println("Got HTTP_POST");
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, data);
        setSettings(doc);
        request->send(200);
    });
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404);
    });
}

void setup()
{
    Serial.begin(115200);

    delay(10);
    preferences.begin("var", false);

    updateSettings();

    printSetting();

    SPIFFS.begin();

    wifiConnect();

    server_on();
    server.begin();
}


void loop(){}