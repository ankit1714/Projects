#include <WebSocketServer.h>
#include <WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include <Arduino_FreeRTOS.h>
#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).


//! Globle Variable
WiFiServer server(8080);
WebSocketServer webSocketServer;
 
const char *ssid = "Envison";
const char *password = "team4";

//Analog Input
#define ANALOG_PIN_0 36
int analog_value_0 = 0;
#define ANALOG_PIN_3 39
int analog_value_3 = 0;
#define GPIO_SWITCH 34
int gpio_switch = 0;


//LCD Variables
#define _cs   17  // goes to TFT CS
#define _dc   16  // goes to TFT DC
#define _mosi 23  // goes to TFT MOSI
#define _sclk 18  // goes to TFT SCK/CLK
#define _rst  5   // goes to TFT RESET
#define _miso     // Not connected
//       3.3V     // Goes to TFT LED  
//       5v       // Goes to TFT Vcc
//       Gnd      // Goes to TFT Gnd        

// Hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(_cs, _dc, _rst);


void setup() {
 
  Serial.begin(115200);
 
  delay(4000);
 
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());
 
  server.begin();
  pinMode(GPIO_SWITCH,INPUT);
  delay(100);

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_ORANGE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.println("Envision...");
  delay(1000);

  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
  xTaskCreate(
                    taskTwo,          /* Task function. */
                    "TaskTwo",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
}
  
  
}
 
void loop() {
 
  WiFiClient client = server.available();

  if (client.connected() && webSocketServer.handshake(client)) {
 
    String data;      
 
    while (client.connected()) {
 
      data = webSocketServer.getData();
 
      if (data.length() > 0) {
         Serial.println(data);
         webSocketServer.sendData(data);

      }
      analog_value_0 = analogRead(ANALOG_PIN_0);
      Serial.println(analog_value_0);
      webSocketServer.sendData(String(55));
      webSocketServer.sendData(String(analog_value_0));
      delay(50);
      analog_value_3 = analogRead(ANALOG_PIN_3);
      Serial.println(analog_value_3);
      webSocketServer.sendData(String(66));
      webSocketServer.sendData(String(analog_value_3));
      delay(50);
      gpio_switch = digitalRead(GPIO_SWITCH);
      Serial.println(gpio_switch);
      webSocketServer.sendData(String(77));
      webSocketServer.sendData(String(gpio_switch));
      delay(50);
   }
   Serial.println("The client disconnected");
   delay(100);
  }

    tft.setRotation(0);
    testText();
    delay(500);
 
  delay(100);
}


unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

void taskOne( void * parameter )
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 1");
        delay(1000);
    }
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}
 
void taskTwo( void * parameter)
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 2");
        delay(1000);
    }
    Serial.println("Ending task 2");
    vTaskDelete( NULL );
 
}
