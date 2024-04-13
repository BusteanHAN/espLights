#define SSID ""
#define WIFI_PASS ""
#define BROKER_URL ""
#define UNIQUE_TOPIC ""
#define GENERAL_TOPIC ""
#define DEVICE_NAME ""
//#define REPORTS_STATE_TO_GENERAL_TOPIC //uncomment to make this device be the general topic reporter; there may only be one single device with this on, as it causes message collisions over MQTT

#define NUM_LEDS 36
#define LED_TYPE WS2812
#define COLOR_ORDER GRB