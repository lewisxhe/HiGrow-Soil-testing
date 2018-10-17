#include <WiFi.h>
#include "DHT.h"
#include <ESPmDNS.h>
#include <WebServer.h>
#include "index.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321

// You can comment this line and turn off the serial output.
#define APP_LOG_ENABLE

#ifdef APP_LOG_ENABLE
#define APP_LOG(...)           \
    Serial.printf(__VA_ARGS__); \
    Serial.println();

#define APP_LOG_BEGIN() do{\
    Serial.begin(115200);\
    while (!Serial) {}\
  }while(0)
#else
#define APP_LOG(...)
#define APP_LOG_BEGIN()
#endif

static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];
// DHT Sensor
const int DHTPin = 22;
WebServer server(80);
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);


bool autoConfig()
{
  WiFi.begin();
  for (int i = 0; i < 20; i++)
  {
    if ( WiFi.status() == WL_CONNECTED)
    {
      APP_LOG("AutoConfig Success");
      APP_LOG("SSID:%s", WiFi.SSID().c_str());
      APP_LOG("PSW:%s", WiFi.psk().c_str());
      return true;
    }
    APP_LOG("AutoConfig Waiting......");
    delay(1000);
  }
  APP_LOG("AutoConfig Faild!" );
  return false;
}

void smartConfig()
{
  WiFi.mode(WIFI_STA);
  APP_LOG("Wait for Smartconfig");
  WiFi.beginSmartConfig();

  while(!WiFi.smartConfigDone())
  {
    APP_LOG(".");
    delay(1000); 
  }
  APP_LOG("SmartConfig Success");
  APP_LOG("SSID:%s\r\n", WiFi.SSID().c_str());
  APP_LOG("PSW:%s\r\n", WiFi.psk().c_str());
  WiFi.setAutoConnect(true); 
}

void readDHTData(){
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    APP_LOG("Failed to read from DHT sensor!");
    strcpy(celsiusTemp, "\"Failed\"");
    strcpy(fahrenheitTemp, "\"Failed\"");
    strcpy(humidityTemp, "\"Failed\"");
    return;
  }
  else {
    // Computes temperature values in Celsius + Fahrenheit and Humidity
    float hic = dht.computeHeatIndex(t, h, false);
    dtostrf(hic, 6, 2, celsiusTemp);
    float hif = dht.computeHeatIndex(f, h);
    dtostrf(hif, 6, 2, fahrenheitTemp);
    dtostrf(h, 6, 2, humidityTemp);
  }
}

// root page 
void handleRootPage(){
  APP_LOG("Get root page");
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

// response json data
void handleDHTData(){
  readDHTData();
  String json = "{";
  json += "\"Fahrenheit\":";
  json += fahrenheitTemp;
  json += ",";
  json += "\"Celsius\":";
  json += celsiusTemp;
  json += ",";
  json += "\"Humidity\":";
  json += humidityTemp;
  json += ",";
  json += "\"Soil\":";
  json += String(map(analogRead(32),0,4096,100,0));
  json += ",";
  json += "\"IP\":";
  json += "\"";
  json += WiFi.localIP().toString();
  json += "\"";
  json += "}";
  server.send(200, "text/plane",json); 
}

void setup() {
  // initialize the DHT sensor
  dht.begin();

  //Initialize serial and wait for port to open:
  APP_LOG_BEGIN();

  // We start by connecting to a WiFi network
  if (!autoConfig())
  {
    APP_LOG("Starting SmartConfig...");
    smartConfig();
  }
  APP_LOG("IP Address : %s",WiFi.localIP().toString().c_str());
  // Start MDNS Server,Point your browser to http://esp32.local, you should see a response.
  if (MDNS.begin("esp32")) {
    APP_LOG("MDNS responder started");
  }

  server.on("/",handleRootPage);
  server.on("/readData",handleDHTData);
  server.begin();                  //Start server
}

void loop() {
  server.handleClient();          //Handle client requests
}
