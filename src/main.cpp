#include <Arduino.h>
#include <MQTT.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>

#include "config.h"

CRGB leds[NUM_LEDS];

WiFiClient net;
MQTTClient client;

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

uint8_t BRIGHTNESS = 255;
static uint8_t startIndex = 0;
bool rgb = false;
bool on = false;
String color = "#000000", prevColor = "#000000";

#include "utils.h"

void setup()
{
  // Serial.begin(74880);
  pinMode(1, OUTPUT);

  FastLED.addLeds<LED_TYPE, 1, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  WiFi.begin(SSID, WIFI_PASS);

  client.begin(BROKER_URL, net);
  client.onMessage(messageReceived);
  
  while(!connect()) {;}

  client.subscribe((String)UNIQUE_TOPIC + "/set");
  client.subscribe((String)GENERAL_TOPIC + "/set");

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}

void loop()
{
  if ((WiFi.status() != WL_CONNECTED) | (!client.connected())){
    connect();
  }

  client.loop();
  lightLoop();
  FastLED.show();
  FastLED.delay(1);
}