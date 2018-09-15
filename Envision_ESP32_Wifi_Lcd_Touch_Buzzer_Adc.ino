#include <WebSocketServer.h>
#include <WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

//! Enable Features
//#define LCD_HANDLER
//#define JOYSTICK_HANDLER
//#define TOUCH_HANDLER
//#define BUZZER_HANDLER
//#define NTC_HANDLER
//#define WEBSCOKET_MODE

// Globle Variable
//#ifdef WEBSCOKET_MODE
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

/* For those of you interested in creating a Menu system, we have defined
   two arrays here. One holds the Menu Title and menu headings, and the other
   holds the associated function to be called. This is a great way to simplify
   the configuration of a menu especially when multiple menu's are rquired
*/
char* menu[] = {"  Cooking Function Menu",
                "1. BAKE           ",
                "2. GRILL          ",
                "3. CONVECTIONAL   ",
                "4. 6th SENSE      ",
                "5. PIZZA & PASTA  ",
                "6. COMBI MW+GRILL ",
                "7. FORCED AIR     ",
                "8. PIZZA & PASTA  "};
typedef void (* MenuFuncPtr) (); // this is a typedef to the menu functions
MenuFuncPtr menu_func[] = {0,
                tftTextTest,
                tftTextTest,
                tftTextTest,
                tftTextTest,
                tftTextTest,
                tftTextTest,
                tftTextTest,
                tftTextTest};
// It's usefult to know the number of menu items without hardcoding it
// We can calculate it thus.. (subtract 1 for the menu heading)
#define numMenu (sizeof(menu)/sizeof(char *))-1 //array size

#define menu_top 20   // Postition of first menu item from top of screen
char menu_select;     // Currently elected menu item
char keydown=0;       // Jog key pressed?

//void tftTextTest(void);

// Hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(_cs, _dc, _rst);


int value_Joystick_X = 0;
int value_Joystick_Y = 0;
int value_Joystick_SW = 0;;
int value_TouchButton = 0;;

#define EVENT_NONE      0
#define EVENT_RIGHT     1
#define EVENT_LEFT      2
#define EVENT_UP        3
#define EVENT_DOWN      4
#define EVENT_OK        5
#define EVENT_ON_OFF    6
#define EVENT_INFO      7
#define EVENT_START     8
#define EVENT_CANCLE    9

#define STATE_POWERON      0
#define STATE_IDLE         1
#define STATE_MENU         2
#define STATE_TEMPERATUR   3
#define STATE_DURATION     4
#define STATE_RUN          5
#define STATE_END          6
#define STATE_SETTING      7
#define STATE_SETTING_WIFI 8
#define STATE_STANDBY      9

#define THRESHOLD_SENSOR_0 35
#define THRESHOLD_SENSOR_1 45
#define THRESHOLD_SENSOR_2 45
#define THRESHOLD_SENSOR_3 45

#define THRESHOLD_X_RIGHT  2400
#define THRESHOLD_X_LEFT   1500
#define THRESHOLD_Y_UP      2400
#define THRESHOLD_Y_DOWN   1500

  int EventValue;
  int CurrentState;
  int PowerOnCounter;
  int Clock_H1 = 0;
  int Clock_H2 = 0;
  int Clock_M1 = 0;
  int Clock_M2 = 0;
  int ClockCounter = 0;
//================================================================ Initilization ========================================================================================================
void setup() {

  Serial.begin(115200);

  delay(4000);

  pinMode(GPIO_SWITCH,INPUT);
  digitalWrite(GPIO_SWITCH,LOW);
  delay(100);

//! LCD Section
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setTextWrap(false);

  delay(100);
  EventValue = EVENT_NONE;
  CurrentState = STATE_POWERON;
  PowerOnCounter = 0;


#ifdef LCD_HANDLER
  xTaskCreate(
                    taskLCDHandler,          /* Task function. */
                    "TaskLCD",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
#endif

#ifdef WEBSCOKET_MODE

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());
  server.begin();

xTaskCreate(
                    taskWebHandler,          /* Task function. */
                    "TaskWeb",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    4,                /* Priority of the task. */
                    NULL);            /* Task handle. */
#endif
}

//================================================================ Main Loop ========================================================================================================
// int message_receive_touch1 = 0;
void loop() {

  EventHandler();
  ViewHandler();
  ClockBackGroundHandler();
}

//================================================================ LCD HANDLER ========================================================================================================
#ifdef LCD_HANDLER
void taskLCDHandler( void * parameter )
{
  int message_receive_touch = 0;
    for( ;; )
    {
      xQueueReceive(queue_TouchButton, &message_receive_touch, portMAX_DELAY);

//      message_receive_touch = touchRead(8);//GPIO4
      if(message_receive_touch == 1)
      {
         tftMenuInit(message_receive_touch);
      }
      else if(message_receive_touch == 2)
      {
        tftMenuInit(message_receive_touch);
      }
       Serial.println(message_receive_touch);
      delay(500);
    }
     vTaskDelete( NULL );
 }
 #endif
//================================================================ WebSocket HANDLER ========================================================================================================
#ifdef WEBSCOKET_MODE
void taskWebHandler( void * parameter )
{
  int message_recev = 0;
    for( ;; )
    {
        WiFiClient client = server.available();
        if (client.connected() && webSocketServer.handshake(client))
        {
            String data;
            while (client.connected())
            {
              data = webSocketServer.getData();
              if (data.length() > 0)
              {
                Serial.println(data);
//              webSocketServer.sendData(Message_FromWeb);
              }

              xQueueReceive(queue_Joystick_X_web, &message_recev, portMAX_DELAY);
              webSocketServer.sendData(String(55));
              webSocketServer.sendData(String(message_recev));
              Serial.println(message_recev);
               delay(100);
              xQueueReceive(queue_Joystick_Y_web, &message_recev, portMAX_DELAY);
              webSocketServer.sendData(String(66));
              webSocketServer.sendData(String(message_recev));
               Serial.println(message_recev);
                delay(100);
              xQueueReceive(queue_Joystick_SW_web, &message_recev, portMAX_DELAY);
              webSocketServer.sendData(String(77));
              webSocketServer.sendData(String(message_recev));
               Serial.println(message_recev);
                delay(100);
              xQueueReceive(queue_TouchButton_web, &message_recev, portMAX_DELAY);
              webSocketServer.sendData(String(88));
              webSocketServer.sendData(String(message_recev));
               Serial.println(message_recev);
                delay(100);
            }
            Serial.println("The client disconnected");
            delay(50);
        }
//        Serial.println("Hello from WebSocket Handler");
        delay(1000);
    }
     vTaskDelete( NULL );
}
#endif
static int menu_counter = 0;
//================================================================ LCD Local Funtions ========================================================================================================
void tftMenuInit(int event)
{
  // Clear screen and display the menu
  char i;
  if(menu_counter >=5)
  {
    menu_counter = 0;
  }
  else if(menu_counter <= 0)
  {
    menu_counter = 6;
  }
  if(event == 1)
  {
     menu_counter++;
  }
  else if(event == 2)
  {
     menu_counter--;
  }

  drawCookingMenu(0);
  tftMenuSelect(menu_counter);
}

void tftTextTest(void)
{
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
}
void tftColorTest(void)
{
      tft.fillScreen(ILI9341_BLACK);
      delay(5000);
      tft.fillScreen(ILI9341_NAVY);
      delay(5000);
      tft.fillScreen(ILI9341_DARKGREEN);
      delay(5000);
      tft.fillScreen(ILI9341_DARKCYAN);
      delay(5000);
      tft.fillScreen(ILI9341_PURPLE);
      delay(5000);
      tft.fillScreen(ILI9341_LIGHTGREY);
      delay(5000);
      tft.fillScreen(ILI9341_DARKGREY);
      delay(5000);
      tft.fillScreen(ILI9341_BLUE);
      delay(5000);
      tft.fillScreen(ILI9341_GREEN);
      delay(5000);
      tft.fillScreen(ILI9341_CYAN);
      delay(5000);
      tft.fillScreen(ILI9341_RED);
      delay(5000);
      tft.fillScreen(ILI9341_MAGENTA);
      delay(5000);
      tft.fillScreen(ILI9341_WHITE);
      delay(5000);
      tft.fillScreen(ILI9341_ORANGE);
      delay(5000);
      tft.fillScreen(ILI9341_GREENYELLOW);
      delay(5000);
      tft.fillScreen(ILI9341_PINK);
      delay(5000);
}

void drawCookingMenu(int counter)
{
  tft.setTextSize(2);

  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);
  tft.print(menu[0]);
  tft.drawLine(0, 15, tft.width()-1, 15, ILI9341_GREEN);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

// 1st Box
  tft.fillRoundRect(10, 25, 310, 31, 8, ILI9341_DARKGREY);
  tft.drawRoundRect(10, 25, 310, 31, 8, ILI9341_WHITE);
  tft.setCursor(20, 31);
  tft.print(menu[1]);

//2nd Box
  tft.fillRoundRect(10, 61, 310, 31, 8, ILI9341_DARKGREY);
  tft.drawRoundRect(10, 61, 310, 31, 8, ILI9341_WHITE);
  tft.setCursor(20, 67);
  tft.print(menu[2]);

// 3rd Box
  tft.fillRoundRect(10, 97, 310, 31, 8, ILI9341_DARKGREY);
  tft.drawRoundRect(10, 97, 310, 31, 8, ILI9341_WHITE);
  tft.setCursor(20, 103);
  tft.print(menu[3]);

// 4rd Box
  tft.fillRoundRect(10, 133, 310, 31, 8, ILI9341_DARKGREY);
  tft.drawRoundRect(10, 133, 310, 31, 8, ILI9341_WHITE);
  tft.setCursor(20, 139);
  tft.print(menu[4]);

  // 5rd Box
  tft.fillRoundRect(10, 169, 310, 31, 8, ILI9341_DARKGREY);
  tft.drawRoundRect(10, 169, 310, 31, 8, ILI9341_WHITE);
  tft.setCursor(20, 175);
  tft.print(menu[5]);
}

void tftMenuSelect(char menuitem)
{
    switch(menuitem)
    {
      case 0:
//      tft.fillRoundRect(10, 25, 310, 31, 8, ILI9341_BLUE);
      break;

      case 1:
      tft.fillRoundRect(10, 25, 310, 31, 8, ILI9341_BLUE);
      tft.drawRoundRect(10, 25, 310, 31, 8, ILI9341_WHITE);
      tft.setCursor(20, 31);
      break;

      case 2:
      tft.fillRoundRect(10, 61, 310, 31, 8, ILI9341_BLUE);
      tft.drawRoundRect(10, 61, 310, 31, 8, ILI9341_WHITE);
      tft.setCursor(20, 67);
      break;

      case 3:
      tft.fillRoundRect(10, 97, 310, 31, 8, ILI9341_BLUE);
      tft.drawRoundRect(10, 97, 310, 31, 8, ILI9341_WHITE);
      tft.setCursor(20, 103);
      break;

      case 4:
      tft.fillRoundRect(10, 133, 310, 31, 8, ILI9341_BLUE);
      tft.drawRoundRect(10, 133, 310, 31, 8, ILI9341_WHITE);
      tft.setCursor(20, 139);
      break;

      case 5:
      tft.fillRoundRect(10, 169, 310, 31, 8, ILI9341_BLUE);
      tft.drawRoundRect(10, 169, 310, 31, 8, ILI9341_WHITE);
      tft.setCursor(20, 175);
      break;
    }
     tft.print(menu[menuitem]);
}
//================================================================ LCD-Web Local Funtions ========================================================================================================
void connectWeb(void)
{

}
int TouchSenstivity_Debounce = 0;
char IsEventReleased = 0;
#define TOUCHSESTIVITY 5
void EventHandler(void)
{
    int sensor_0 = 0;
    int sensor_1 = 0;
    int sensor_2 = 0;
    int sensor_3 = 0;

    // X-Axis Opration
    value_Joystick_X = analogRead(ANALOG_PIN_0);
        // Y-Axis Opration
    value_Joystick_Y = analogRead(ANALOG_PIN_3);
     // Switch Opration
    value_Joystick_SW = digitalRead(GPIO_SWITCH);

    sensor_0 = touchRead(T0);//GPIO4
    sensor_1 = touchRead(T3);//GPIO15
    sensor_2 = touchRead(T9);//GPIO9
    sensor_3 = touchRead(T8);//GPIO8


      if(value_Joystick_X > THRESHOLD_X_RIGHT)
      {
        EventValue = EVENT_RIGHT; // Event Right
      }
      else if(value_Joystick_X < THRESHOLD_X_LEFT)
      {
        EventValue = EVENT_LEFT; // Event Left
      }
      else if(value_Joystick_Y > THRESHOLD_Y_UP)
      {
        EventValue = EVENT_UP; // Event UP
      }
      else if(value_Joystick_Y < THRESHOLD_Y_DOWN)
      {
        EventValue = EVENT_DOWN; // Event Down
      }
      else if( value_Joystick_SW == 0 )
      {
        EventValue = EVENT_OK;
      }
      else if(sensor_0 < THRESHOLD_SENSOR_0 )
      {
        TouchSenstivity_Debounce++;
        if(TouchSenstivity_Debounce == TOUCHSESTIVITY)
        {
          TouchSenstivity_Debounce = 0;
          if(IsEventReleased == 0)
          {
            IsEventReleased = 1;
            EventValue = EVENT_ON_OFF;
          }
        }     
      }
      else if(sensor_1 < THRESHOLD_SENSOR_1 )
      {
        EventValue = EVENT_INFO;
      }
      else if(sensor_2 < THRESHOLD_SENSOR_2 )
      {
        EventValue = EVENT_CANCLE;
      }
      else if(sensor_3 < THRESHOLD_SENSOR_3 )
      {
        EventValue = EVENT_START;
      }
      else
      {
        IsEventReleased = 0;
        EventValue = EVENT_NONE;
      }
    Serial.println(EventValue);
}
void ViewHandler(void)
{
  switch(CurrentState)
  {
    case STATE_POWERON:
      StatePowerOnView();
      break;
    case STATE_IDLE:
      StateIdleView();
      StateIdleEventHandler();
      break;
    case STATE_MENU:
      menu_select=1;                    // Select 1st menu item
      if(menu_select==1)
      {
         tftMenuInit(0);                    // Draw menu
        tftMenuSelect(menu_select);       // Highlight selected menu item
        menu_select = 2;
      }      
      StateMenuEventHandler();
      break;
    case STATE_TEMPERATUR:
      break;
    case STATE_DURATION:
      break;
    case STATE_RUN:
      break;
    case STATE_END:
      break;
    case STATE_SETTING:
      break;
    case STATE_SETTING_WIFI:
      break;
    case STATE_STANDBY:
      break;
    default:
      break;
  }
}
void StatePowerOnView(void)
{
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.setCursor(0, 100);
  tft.println("Whirlpool LOGO...");
  PowerOnCounter++;
  if(PowerOnCounter > 2 )
  {
    PowerOnCounter = 0;
    CurrentState = STATE_IDLE;
    tft.fillScreen(ILI9341_BLACK);
  }
}
void ClockBackGroundHandler(void)
{
    ClockCounter++;
  if(ClockCounter > 10 )
  {
    ClockCounter = 0;
    Clock_M2++;
    if(Clock_M2 >= 9 )
    {
      Clock_M2 = 0;
      Clock_M1++;
      if(Clock_M1 >5)
      {
        Clock_M1 = 0;
        Clock_H2++;
        if(Clock_H2 > 9)
        {
          Clock_H2 = 0;
          Clock_H1++;
          if(Clock_H1>2)
          {
            Clock_H2=0;
            Clock_H1 = 0;
            Clock_M1 = 0;
            Clock_M2 = 0;
          }
        }
      }
    }    
  }
}
void StateIdleView(void)
{
  delay(100);
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
  tft.setTextSize(5);
  tft.setCursor(100, 115);
  tft.println(String(Clock_H1));
  tft.setCursor(133, 115);
  tft.println(String(Clock_H2));
  tft.setCursor(160, 115);
  tft.println(":");
  tft.setCursor(187, 115);
  tft.println(String(Clock_M1));
  tft.setCursor(220, 115);
  tft.println(String(Clock_M2));
}
void StateIdleEventHandler(void)
{
  if(EventValue == EVENT_ON_OFF)
  {
    CurrentState = STATE_MENU;
    tft.fillScreen(ILI9341_BLACK);
  }
}
void StateMenuEventHandler(void)
{
  if(EventValue == EVENT_ON_OFF)
  {
    CurrentState = STATE_IDLE;
    tft.fillScreen(ILI9341_BLACK);
  }
}
