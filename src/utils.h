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
  return a - (((int)a > 47 && (int)a < 58)  ? 48 :         //character range for numeric characters; subtract 48 to get to int 0-9
              ((int)a > 64 && (int)a < 91)  ? 55 :         //character range for capital letters; subtract 55 to get to int 10-15 (0xA-0xF)
              ((int)a > 96 && (int)a < 123) ? 87 : 0);     //character range for lowercase letters; subtract 87 to get to int 10-15 (0xA-0xF)
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
  // Serial.println("incoming: " + topic + " - " + payload);

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