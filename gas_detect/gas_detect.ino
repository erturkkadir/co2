/*
   PROJECT TO MONTIOR GASES IN A ROOM 
*/
#include "Free_Fonts.h"   // Include the header file attached to this sketch
#include <TFT_eSPI.h>     // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "SparkFun_SGP30_Arduino_Library.h" 
#include "MICS6814.h"

SGP30 mySensor;
#define I2C_SGP30  0x58

#define PIN_NO2 27
#define PIN_NH3 26
#define PIN_CO  25
MICS6814 gas(PIN_CO, PIN_NO2, PIN_NH3);



/*TFT_WHITE TFT_BLACK TFT_YELLOW TFT_RED TFT_GREEN TFT_BLUE*/

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

const char* ssid = "golaks";
const char* pass = "";
const char* mqttServer = "";
int mqttPort = 1883;

long last_time = 0;
char myData[255];


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


/* ########################### FUNCTIONS ######################*/
/* Init the TFT object */
void initTFT() {
  tft.init();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
}

/* Print text to TFT screen generic function */
void printTFT(int x, int y, int font, char * str) {
  x = 1; // tft.width()/2;
  y = tft.height() / 2; // tft.fontHeight();  
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString(str, x, y);
}

/* Setup MQTT */
void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);  
}

/* if connection is broken, try to reconnect  */
void reconnect() {  
  while (!mqttClient.connected()) {
      if (mqttClient.connect("capilano")) {
        Serial.println("connection");
        mqttClient.subscribe("capilano/basement/air");  // subscribe to topic
      } else {
        Serial.println("no connection");
      }
  }
}

/* MQTT callbacks */
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp = "";
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
 //     digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
 //     digitalWrite(ledPin, LOW);
    }
  }
}


/* Finds absolute humidity */
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  float absoluteHumidity = 213.7f * (humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature);
  uint32_t scaledHumidity = (uint32_t) (1000.0f * absoluteHumidity); 
  return scaledHumidity;
}

/* Connect to WIFI*/
void connectToWiFi() {
  printTFT(1, 1, 4, "Connecting initialize..."); // This uses the standard ADAFruit small font
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) {
    printTFT(1, 1, 4, "Wait ...."); // Overwrite the text to clear it
  }
}
/* ########################### FUNCTIONS END ################################# */ 

void setup() {
  
  initTFT();

  connectToWiFi();

  printTFT(1, 1, 4, "Connected");

  setupMQTT();

  gas.calibrate();

  Wire.begin();
  Wire.setClock(400000);
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  mySensor.initAirQuality();

  analogReadResolution(14);

  pinMode(NO2,INPUT);
  pinMode(NH3,INPUT);
  pinMode(CO,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //tft.println("www.syshuman.com");

  if (!mqttClient.connected()) {
    reconnect();
  }
  Serial.println("connected ");
  mqttClient.loop();
  long now = millis();
  if (now - last_time > 10000) {
    mySensor.setHumidity(8200);
    mySensor.measureAirQuality();
    tft.setCursor(0, 20, 4); // font 4
    tft.print("CO2 : ");
    tft.printf("%4d ", mySensor.CO2);
    tft.println();
  
    tft.setCursor(0, 60, 2); // font 4
    tft.print("TVOC : ");
    tft.printf("%4d", mySensor.TVOC);
    mySensor.measureRawSignals();
    tft.println();
    tft.print("H2   : ");
    tft.printf("%4d", mySensor.H2);
  
    tft.println();
    tft.print("Eth  : ");
    tft.printf("%4d", mySensor.ethanol);

    tft.println();
    int no2 = gas.measure(NO2)*100;
    delay(50);
    int nh3 = gas.measure(NH3) * 100;
    delay(50);
    int co = gas.measure(CO) * 100;
    delay(50);
    tft.print("NO2 : "); tft.print(NO2);
    tft.print(" NH3 : "); tft.print(NH3);
    tft.print(" CO  : "); tft.print(CO);

    sprintf(myData, "[{'myTime':'None'},{'CO2*':%d},{'TVOC*':%d},{'H2*':%d},{'ETH*':%d},{'NO2':%d},{'NH3':%d},{'CO':%d}]", mySensor.CO2, mySensor.TVOC, mySensor.H2, mySensor.ethanol, no2, nh3, co);
    Serial.println(myData);

    mqttClient.publish("capilano/basement/air", myData);
    last_time = now;
  }
 
  yield(); // We must yield() to stop a watchdog timeout.
}
