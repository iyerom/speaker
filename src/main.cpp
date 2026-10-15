#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "Audio.h"
#include "config.h"


int BCLK = 14;
int LRC  = 12;
int DIN  = 13;

void processData(AsyncResult &aResult);
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

FirebaseApp app;
WiFiClientSecure sslClient;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(sslClient);
RealtimeDatabase db;
Audio audio;

String newMsg = "";
bool hasPending = false;
static unsigned long lastWifiCheck = 0;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, WIFI_USER, WIFI_USER, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  audio.setPinout(BCLK, LRC, DIN);
  audio.setVolume(21);  

  sslClient.setInsecure();
  sslClient.setTimeout(1000);
  sslClient.setHandshakeTimeout(5);
  initializeApp(aClient, app, getAuth(user_auth), processData, "tts | authTask");
  app.getApp<RealtimeDatabase>(db);
  db.url(DATABASE_URL);
  db.get(aClient, "/message", processData, true, "messageStream");
}

void loop() {
  if (millis() - lastWifiCheck > 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost, reconnecting");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, WIFI_USER, WIFI_USER, WIFI_PASSWORD);
    }
  }
  app.loop();
  audio.loop();

  if (hasPending) {
    
    hasPending = false;
    audio.connecttospeech(newMsg.c_str(), "en");
  }
}

void processData(AsyncResult &aResult) {
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (!aResult.available())
    return;

  RealtimeDatabaseResult &rtdb = aResult.to<RealtimeDatabaseResult>();
  if (!rtdb.isStream())
    return;

  if (rtdb.dataPath().equals("/")) {
    String text = rtdb.to<String>();
    String name = "", msg = text;
    int sep = text.indexOf(":");
    if(sep>0){
      name = text.substring(0,sep);
      msg = text.substring(sep+1);
      name.trim(); msg.trim();
    }
    if (msg.length() > 0) {
      newMsg = name.length() > 0 ? (name + " says " + msg): msg;
      hasPending = true;
    }
  }
}

