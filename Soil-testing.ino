#include <WiFi.h>
#include "DHT.h"
#include <ESPmDNS.h>
#include <WebServer.h>
#include "index.h"
#include <WiFiMulti.h>

// #define USE_FIXED_WIFI_PASSWD

#define WIFI_SSID_1 "TP-LINK_2746"
#define WIFI_SSID_2 "Xiaomi"
#define WIFI_PASSWORD_1 "AA15994823428"
#define WIFI_PASSWORD_2 "12345678"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT11 // DHT 22  (AM2302), AM2321
#define WIFI_TIMEOUT 30000

#define LED_PIN 16
#define DTH_SENSOR_PIN 22

WiFiMulti wifiMulti;

static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

// Initialize DHT sensor.
DHT dht(DTH_SENSOR_PIN, DHTTYPE);
WebServer server(80);

bool autoConfig()
{
  WiFi.begin();
  for (int i = 0; i < 20; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("PSW: ");
      Serial.println(WiFi.psk());
      return true;
    }
    Serial.println("AutoConfig Waiting......");
    delay(1000);
  }
  Serial.println("AutoConfig Faild!");
  return false;
}

void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("Wait for Smartconfig");
  WiFi.beginSmartConfig();

  while (!WiFi.smartConfigDone())
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("SmartConfig Success");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("PSW: ");
  Serial.println(WiFi.psk());
  WiFi.stopSmartConfig();
  WiFi.setAutoConnect(true);
}

void readDHTData()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT sensor!");
    strcpy(celsiusTemp, "\"Failed\"");
    strcpy(fahrenheitTemp, "\"Failed\"");
    strcpy(humidityTemp, "\"Failed\"");
    return;
  }
  else
  {
    // Computes temperature values in Celsius + Fahrenheit and Humidity
    float hic = dht.computeHeatIndex(t, h, false);
    dtostrf(hic, 6, 2, celsiusTemp);
    float hif = dht.computeHeatIndex(f, h);
    dtostrf(hif, 6, 2, fahrenheitTemp);
    dtostrf(h, 6, 2, humidityTemp);
  }
}

// root page
void handleRootPage()
{
  String s = MAIN_page;             //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

// response json data
void handleDHTData()
{
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
  json += String(map(analogRead(32), 0, 4096, 100, 0));
  json += ",";
  json += "\"IP\":";
  json += "\"";
  json += WiFi.localIP().toString();
  json += "\"";
  json += "}";
  server.send(200, "text/plane", json);
}

void setup()
{
  // initialize the DHT sensor
  dht.begin();

  //Initialize serial and wait for port to open:
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  int i = 3;
  while (i--)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
  digitalWrite(LED_PIN, HIGH);

#ifdef USE_FIXED_WIFI_PASSWD

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASSWORD_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASSWORD_2);

  Serial.println("Connecting Wifi...");

  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connecting to ");
  Serial.println(WiFi.SSID());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
#else
  // We start by connecting to a WiFi network
  if (!autoConfig())
  {
    Serial.println("Starting SmartConfig...");
    smartConfig();
  }
  Serial.println("IP Address :");
  Serial.println(WiFi.localIP());
  //Start MDNS Server, Point your browser to http : //esp32.local, you should see a response.
#endif

  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRootPage);
  server.on("/readData", handleDHTData);
  server.begin();

  MDNS.addService("http", "tcp", 80);
}

void loop()
{
#ifdef USE_FIXED_WIFI_PASSWD
  wifiMulti.run();
#else
  static unsigned long last_wifi_check_time = 0;
  unsigned long now = millis();

  if (now - last_wifi_check_time > WIFI_TIMEOUT)
  {
    Serial.print("Checking WiFi... ");
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi connection lost. Reconnecting...");
      WiFi.reconnect();
    }
    else
    {
      Serial.println("OK");
    }
    last_wifi_check_time = now;
  }
#endif

  server.handleClient(); //Handle client requests
}
