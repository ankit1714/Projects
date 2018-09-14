#include <WebSocketServer.h>
#include <WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"



//! Enable Features
#define LCD_HANDLER
#define JOYSTICK_HANDLER
#define TOUCH_HANDLER
//#define BUZZER_HANDLER
//#define NTC_HANDLER
#define WEBSCOKET_MODE


// Globle Variable
//#ifdef WEBSCOKET_MODE
WiFiServer server(8080);
WebSocketServer webSocketServer;
const char *ssid = "Envison";
const char *password = "team4";

String Message_FromWeb;
int X_Axis_Value = 0;
int Y_Axis_Value = 0;
int SW_Value = 0;
int Touch_ButtonValue = 0;
//#endif

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
char* menu[] = {"  Cooking Functuin Menu",
                "BAKE           ",
                "GRILL          ",
                "CONVECTIONAL   ",
                "6th SENSE      ",
                "MICROWAVE      ",
                "COMBI MW+GRILL ",
                "FORCED AIR     ",
                "PIZZA & PASTA  "};
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



//RTOS Task
QueueHandle_t queue_Joystick_X;
QueueHandle_t queue_Joystick_Y;
QueueHandle_t queue_Joystick_SW;
QueueHandle_t queue_TouchButton;

QueueHandle_t queue_Joystick_X_web;
QueueHandle_t queue_Joystick_Y_web;
QueueHandle_t queue_Joystick_SW_web;
QueueHandle_t queue_TouchButton_web;

QueueHandle_t queue_Web_Incomming_Message;

int queueSize1 = 100;

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
  tft.fillScreen(ILI9341_ORANGE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

 // tftMenuInit();                    // Draw menu
  //menu_select=1;                    // Select 1st menu item 
  //tftMenuSelect(menu_select);       // Highlight selected menu item
  //delay(1000);

  queue_Joystick_X = xQueueCreate( queueSize1, sizeof( int ) );
  queue_Joystick_Y = xQueueCreate( queueSize1, sizeof( int ) );
  queue_Joystick_SW = xQueueCreate( queueSize1, sizeof( int ) );
  queue_TouchButton = xQueueCreate( queueSize1, sizeof( int ) );

  queue_Joystick_X_web = xQueueCreate( queueSize1, sizeof( int ) );
  queue_Joystick_Y_web = xQueueCreate( queueSize1, sizeof( int ) );
  queue_Joystick_SW_web = xQueueCreate( queueSize1, sizeof( int ) );
  queue_TouchButton_web = xQueueCreate( queueSize1, sizeof( int ) );
 // queue_Web_Incomming_Message = xQueueCreate( queueSize1, sizeof( int ) );


#ifdef LCD_HANDLER
  xTaskCreate(
                    taskLCDHandler,          /* Task function. */
                    "TaskLCD",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
#endif

#ifdef JOYSTICK_HANDLER
  xTaskCreate(
                    taskJoyStickHandler,          /* Task function. */
                    "TaskJoyStick",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
#endif


#ifdef TOUCH_HANDLER
  xTaskCreate(
                    taskTouchHandler,          /* Task function. */
                    "TaksTouch",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */  
#endif

#ifdef BUZZER_HANDLER
 xTaskCreate(
                    taskBuzzerHandler,          /* Task function. */
                    "TaskBuzzer",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL);            /* Task handle. */  
#endif

#ifdef NTC_HANDLER             
xTaskCreate(
                    taskNTCHandler,          /* Task function. */
                    "TaskNTC",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    3,                /* Priority of the task. */
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
 
void loop() {
  delay(1000);
}

//================================================================ LCD HANDLER ========================================================================================================
#ifdef LCD_HANDLER
void taskLCDHandler( void * parameter )
{ 
    for( ;; )
    {

    }
     vTaskDelete( NULL );
 }
 #endif
//================================================================ JoyStick HANDLER ======================================================================================================== 
#ifdef JOYSTICK_HANDLER
void taskJoyStickHandler( void * parameter)
{
    int x_axis_value = 0;
    int y_axis_value = 0;
    int switch_value = 0;
    for( ;; )
    {
        // X-Axis Opration
        x_axis_value = 25;//analogRead(ANALOG_PIN_0);
        xQueueSend(queue_Joystick_X, &x_axis_value, portMAX_DELAY);
        xQueueSend(queue_Joystick_X_web, &x_axis_value, portMAX_DELAY);
        delay(500);
        // Y-Axis Opration 
        y_axis_value = 26;//analogRead(ANALOG_PIN_3);
        xQueueSend(queue_Joystick_Y, &y_axis_value, portMAX_DELAY);
        xQueueSend(queue_Joystick_Y_web, &y_axis_value, portMAX_DELAY);
        delay(500);
         // Switch Opration
        switch_value = 27;//digitalRead(GPIO_SWITCH);
        xQueueSend(queue_Joystick_SW, &switch_value, portMAX_DELAY);  
        xQueueSend(queue_Joystick_SW_web, &switch_value, portMAX_DELAY);     
        delay(500);    
    }
   vTaskDelete( NULL );
}
#endif
//================================================================ Touch HANDLER ========================================================================================================
#ifdef TOUCH_HANDLER
void taskTouchHandler( void * parameter )
{
      int sensor_0 = 0;
      int sensor_1 = 0;
      int sensor_2 = 0;
      int sensor_3 = 0;
      int touch_event = 0;
      
     for( ;; )
     { 
        sensor_0 = touchRead(T8);
        sensor_1 = touchRead(T8);
        sensor_2 = touchRead(T8);
        sensor_3 = touchRead(T8);
        
        if(sensor_0 < 50 )
        {
          touch_event = 1;
        }
        else if(sensor_1 < 50 )
        {
          touch_event = 2;
        }
        else if(sensor_2 < 50 )
        {
          touch_event = 3;
        }
        else if(sensor_3 < 50 )
        {
          touch_event = 4;
        }
        xQueueSend(queue_TouchButton, &touch_event, portMAX_DELAY);
        xQueueSend(queue_TouchButton_web, &touch_event, portMAX_DELAY);
        delay(1000);    
    }    
    vTaskDelete( NULL ); 
}
#endif
//================================================================ Buzzer HANDLER ========================================================================================================
#ifdef BUZZER_HANDLER
void taskBuzzerHandler( void * parameter )
{ 
    for( ;; )
    {
       Serial.println("Hello from Buzzer Handler");
        delay(1000);
    }
     vTaskDelete( NULL );
}
#endif
//================================================================ NTC HANDLER ========================================================================================================
#ifdef NTC_HANDLER
void taskNTCHandler( void * parameter )
{
    for( ;; )
    {
       Serial.println("Hello from NTC Handler");
        delay(1000);
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

//================================================================ LCD Local Funtions ========================================================================================================
void tftMenuInit()
{
  // Clear screen and display the menu
  char i;
  
  tft.setTextWrap(false);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK); 
  tft.println(menu[0]);

  tft.drawLine(0, 15, tft.width()-1, 15, ILI9341_GREEN);
  
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);  
  for(i=1;i<=numMenu;i++)
  {
     tft.setCursor(0, ((i-1)*10)+menu_top);    
     tft.println(menu[i]);
  }  
}

void tftMenuSelect(char menuitem) 
{
  // Highlight a selected menu item
  char i;
  // Remove highlight of current item
  tft.setCursor(0, ((menu_select-1)*10)+menu_top);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK); 
  tft.println(menu[menu_select]); 
  // Highlight new menu item
  tft.setCursor(0, ((menuitem-1)*10)+menu_top);
  tft.setTextColor(ILI9341_RED, ILI9341_PURPLE); 
  tft.println(menu[menuitem]);
  // change menu_select to new item  
  menu_select=menuitem;
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
//================================================================ LCD-Web Local Funtions ========================================================================================================
void connectWeb(void)
{

}

