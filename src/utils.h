#include <Arduino.h>
#include <MQTT.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include "json.hpp"
using json = nlohmann::json;

bool connect()
{
  // Serial.println("Entered connect()");
  //set all leds to black
  uint8_t i;
  for (i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB::Black;

  i = 0;

  FastLED.show();  
  leds[0] = CRGB(0x00, 0x00, 0x02);
  FastLED.show();

  // Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    // Serial.print(".");
    // WiFi.reconnect();
    delay(1000);
    i++;
    if (i > 11){
      // Serial.println("WiFi connection failed");
      leds[0] = CRGB(0x02, 0x00, 0x00);
      FastLED.show();
      return false;
    }
  }
  leds[0] = CRGB(0x00, 0x02, 0x00);
  FastLED.show();
  // Serial.println("Connected to WiFi");

  i = 0;

  leds[1] = CRGB(0x00, 0x00, 0x02);
  FastLED.show();

  // Serial.print("Connecting to MQTT");
  while (!client.connect(DEVICE_NAME, "try", "try")) // lamp | shelf
  {
    // Serial.print(".");
    delay(1000);
    i++;
    if (i > 11){
      // Serial.println("MQTT connection failed");
      leds[1] = CRGB(0x02, 0x00, 0x00);
      FastLED.show();
      return false;
    }
  }
  leds[1] = CRGB(0x00, 0x02, 0x00);
  FastLED.show();
  client.subscribe((String)UNIQUE_TOPIC + "/set");
  client.subscribe((String)GENERAL_TOPIC + "/set");
  // Serial.println("Connected to MQTT");
  delay(1000);

  FastLED.clear();
  FastLED.show();  
  // Serial.println("Cleared Leds");
  
  return true;
}

uint8_t hexConcat(uint8_t a, uint8_t b)
{
  return (a << 4) | b;
}

uint8_t hexStringToUint8(char a)
{
  switch (a)
  {
  case '0':
    return 0x0;
  case '1':
    return 0x1;
  case '2':
    return 0x2;
  case '3':
    return 0x3;
  case '4':
    return 0x4;
  case '5':
    return 0x5;
  case '6':
    return 0x6;
  case '7':
    return 0x7;
  case '8':
    return 0x8;
  case '9':
    return 0x9;
  case 'A':
  case 'a':
    return 0xA;
  case 'B':
  case 'b':
    return 0xB;
  case 'C':
  case 'c':
    return 0xC;
  case 'D':
  case 'd':
    return 0xD;
  case 'E':
  case 'e':
    return 0xE;
  case 'F':
  case 'f':
    return 0xF;
  default:
    return 0x0;
  }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, BRIGHTNESS, currentBlending);
    colorIndex += 3;
  }
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);

  json in = json::parse(payload);
  on = !in["state"].is_null() ? in["state"] == "ON" ? true : false : on;
  prevColor = color;
  color = !in["color"].is_null() ? String(in["color"].dump().c_str()) : color;
  rgb = !in["rgbMode"].is_null() ? in["rgbMode"] == "ON" ? true : false : rgb;
  if (prevColor != color) rgb = false;
  json out;
  out["state"] = on ? "ON" : "OFF";
  out["color"] = color;
  out["rgbMode"] = rgb ? "ON" : "OFF";
  client.publish((String)UNIQUE_TOPIC + "/state", out.dump().c_str());
  #ifdef REPORTS_STATE_TO_GENERAL_TOPIC
    client.publish((String)GENERAL_TOPIC + "/state", out.dump().c_str());
  #endif
}

void lightLoop()
{
  startIndex != 255 ? startIndex++ : startIndex = 0;

  uint8_t r, g, b;
  r = hexConcat(hexStringToUint8(color.charAt(2)), hexStringToUint8(color.charAt(3)));
  g = hexConcat(hexStringToUint8(color.charAt(4)), hexStringToUint8(color.charAt(5)));
  b = hexConcat(hexStringToUint8(color.charAt(6)), hexStringToUint8(color.charAt(7)));

  if (on) 
    if (rgb == true) 
      FillLEDsFromPaletteColors(startIndex);
    else
      for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(r, g, b);
  else 
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;


  FastLED.show();
}