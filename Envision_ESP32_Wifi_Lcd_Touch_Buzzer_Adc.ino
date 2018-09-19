#include <WebSocketServer.h>
#include <WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeSans9pt7b.h>

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
                "Microwave",
                "Grill",
                "Manual FC",
                "Grill MW Combi",
                "FC MW Combi",
                "Settings",
                "6ht Sense",
                "Pizza & Pasta"};
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
#define STATE_MW_POWER     10
#define STATE_MW_GRILL     11

#define THRESHOLD_SENSOR_0 45
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

  int End_UpdateDisplay = 0;
  int RunCookTime_UpdateDisplay = 0;
  int CookTime_UpdateDisplay = 0;
  int MwPower_UpdateDisplay = 0;
  int Mw_PowerLevel = 8;
  int UpDateDisplay = 0;
int MenuSelection_Counter = 0;
int MenuSelected = 0;
char PowerString[10];
int PowerTable[11] = {0,10,20,30,40,50,60,70,80,90,100};
int CookTime_S1 = 0;
int CookTime_S2 = 0;
int CookTime_M1 = 0;
int CookTime_M2 = 0;
int CookTime_H1 = 0;
int CookTime_H2 = 0;
int CookTimeViewInit = 0;
int RunCookTime_S1 = 0;
int RunCookTime_S2 = 0;
int RunCookTime_M1 = 0;
int RunCookTime_M2 = 0;
int RunCookTime_H1 = 0;
int RunCookTime_H2 = 0;
int RunCookTime_Init = 0;

int freq = 2000;
int channel = 0;
int resolution = 8;
 int DutyCycle[6] = {1480,2950,2217,1976,2489,3951};
 int ToneOnTime = 90;
 int ToneOffTime[6] = {90,160,90,160,160,900};
 int ToneCounter = 0;
 int ToneCounter1 = 0;
 int ToneCounter2 = 0;




//================================================================ Game          ========================================================================================================
//Variables
int threshold = 40; //threshold for touch
boolean start = false; //will not start without say-so
unsigned long offsetT = 0; //time delay for touch
unsigned long offsetM = 0; //time delay for main loop

float gs;
int headX = 1;        //coordinates for head
int headY = 1;
int beenHeadX[470];   //coordinates to clear later
int beenHeadY[470];
int changeX = 0;      //the direction of the snake
int changeY = 1;
boolean lastMoveH = false; //to keep from going back on oneself
int score = 1;
int foodX;            //coordinates of food
int foodY;
boolean eaten = true; //if true a new food will be made
int loopCount = 0; //number of times the loop has run
int clearPoint = 0;  //when the loopCount is reset
boolean clearScore = false;
float gameSpeed = 6;  //Higher numbers are faster
int RunGameMode = 0;

#define LCD_BACKGROUND_MENU_COLOR 0x7B1C //0x69BA//0x945E //ILI9341_CYAN
#define LCD_MENU_OUTER_REC_COLOR ILI9341_WHITE
#define LCD_MENU_MIDLE_REC_COLOR ILI9341_BLACK
#define LCD_MENU_SELECTED_REC_COLOR 0x917F //0xC29F //ILI9341_BLUE
//================================================================ Initilization ========================================================================================================
void setup() {

  Serial.begin(115200);

  delay(500);

  ledcSetup(channel, freq, resolution);
  ledcAttachPin(12, channel);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

  pinMode(GPIO_SWITCH,INPUT);
  digitalWrite(GPIO_SWITCH,LOW);
  delay(100);

//! LCD Section
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextWrap(false);

  delay(100);
  EventValue = EVENT_NONE;
  CurrentState = STATE_POWERON;
  PowerOnCounter = 0;

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());
  server.begin();
//FontTest();
//  gameInit();
}

//================================================================ Main Loop ========================================================================================================
void loop() {
//tftColorTest();
  EventHandler();
  ViewHandler();
  ClockBackGroundHandler();
  connectWeb();
  BuzzerHandler();

  if(RunGameMode == 1)
  {
      gameLoopFunc();
  }
}
void FontTest(void)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(50, 50);
  tft.setTextSize(2);
  tft.println("Whirlpool");
  delay(500);
//   tft.setFont(&FreeSans9pt7b);
  tftTextTest();
}
void BuzzerHandler(void)
{
   ledcWrite(channel, 254);
 if(ToneCounter1 == 0)
 {
  for (int freq = 0; freq < 6; freq++)
  {
      ToneCounter++;

     if(ToneCounter < 90)
     {
        digitalWrite(13,HIGH);
     }
     else if(ToneCounter < (160))
     {
        digitalWrite(13,LOW);
     }
     else
     {
       ToneCounter1++;
       if(ToneCounter1>30)
       {
        ToneCounter1 = 0;
        ToneCounter2 = 0;
       }
     }
     ToneCounter1 = 1;
     ledcWriteTone(channel, DutyCycle[freq]);
     delay(195);
  }
 }
 digitalWrite(13,LOW);
}
//================================================================ LCD HANDLER ========================================================================================================
static int menu_counter = 0;
//================================================================ LCD Local Funtions ========================================================================================================

void tftTextTest(void)
{
//  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(10, 150);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(2);
  tft.println("Whirlpool");
//   tft.setCursor(10, 140);
//  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
//  tft.println("I implore thee");
//    tft.setCursor(10, 180);
//  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
//  tft.println(123456789, HEX);
//  tft.println();
//  tft.setTextColor(ILI9341_GREEN);
//  tft.setTextSize(1);
//  tft.println("Groop");
//  tft.setTextSize(2);
//  tft.println("I implore thee,");
//  tft.setTextSize(1);
//  tft.println("my foonting turlingdromes.");
//  tft.println("And hooptiously drangle me");
//  tft.println("with crinkly bindlewurdles,");
//  tft.println("Or I will rend thee");
//  tft.println("in the gobberwarts");
//  tft.println("with my blurglecruncheon,");
//  tft.println("see if I don't!");
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
      tft.fillScreen(ILI9341_YELLOW);
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
String MessageFromWeb;
void connectWeb(void)
{
        WiFiClient client = server.available();
        if (client.connected() && webSocketServer.handshake(client))
        {
            String data;
            while (client.connected())
            {
              data = webSocketServer.getData();
              MessageFromWeb = data;
              if (data.length() > 0)
              {
                Serial.println(MessageFromWeb);
//              webSocketServer.sendData(Message_FromWeb);
              }
               ConnectivityHandler();
             //   EventHandler();
                ViewHandler();
                ClockBackGroundHandler();

            }
              Serial.println("The client disconnected");
            delay(50);
        }
}
int RunningCounter = 0;
void ConnectivityHandler(void)
{
  switch(CurrentState)
  {
    case STATE_POWERON:
      StatePowerOnView();
      break;
    case STATE_IDLE:
      if(MessageFromWeb == "PowerOn")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_ON_OFF;
      }
      StateIdleView();
      StateIdleEventHandler();
      break;
    case STATE_MENU:
      if(MessageFromWeb == "MenuUp")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_UP;
      }
      else  if(MessageFromWeb == "MenuDown")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_DOWN;
      }
      else if(MessageFromWeb == "StateMWPower")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_OK;
      }
      else if(MessageFromWeb == "PowerOff")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_ON_OFF;
      }
      MenuSelection_Counter = 100;
      StateMenuView();
      StateMenuEventHandler();
      EventValue = EVENT_NONE;
      break;
    case STATE_TEMPERATUR:
      break;
    case STATE_DURATION:
      if(MessageFromWeb == "DurationUp")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_RIGHT;
      }
      else  if(MessageFromWeb == "DurationDown")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_LEFT;
      }
      else if(MessageFromWeb == "RunningState")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_OK;
      }
      MenuSelection_Counter = 100;
    StateCookTimeView();
    StateDurationEventHandler();
      EventValue = EVENT_NONE;
      break;
    case STATE_RUN:
       RunningCounter++;
      if(RunningCounter > 100)
      {
        RunningCounter = 0;
        webSocketServer.sendData(String(22));
        webSocketServer.sendData("Running...");
         webSocketServer.sendData(String(11));
        webSocketServer.sendData(String(PowerTable[Mw_PowerLevel]));
      }
      if(MessageFromWeb == "OFF")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_ON_OFF;
      }
      StateRunningView();
      StateRunEventHandler();
      EventValue = EVENT_NONE;
      break;
    case STATE_END:
      RunningCounter++;
      if(RunningCounter > 100)
      {
        RunningCounter = 0;
         webSocketServer.sendData(String(22));
        webSocketServer.sendData("Cooking Completed");
      }
      if(MessageFromWeb == "OFF")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_ON_OFF;
      }
      StateEndView();
      StateEndEventHandler();
       EventValue = EVENT_NONE;
      break;
    case STATE_SETTING:
      break;
    case STATE_SETTING_WIFI:
      break;
    case STATE_STANDBY:
      break;
    case STATE_MW_POWER:
      if(MessageFromWeb == "PowerUp")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_RIGHT;
      }
      else  if(MessageFromWeb == "PowerDown")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_LEFT;
      }
      else if(MessageFromWeb == "Duration")
      {
        MessageFromWeb = "None";
        EventValue = EVENT_OK;
      }
      MenuSelection_Counter = 100;
      StateMwPowerView();
      StateMwPowerEventHandler();
      EventValue = EVENT_NONE;
      break;
    case STATE_MW_GRILL:
      break;
    default:
      break;
  }
}

int TouchSenstivity_Debounce = 0;
int IsEventReleased = 0;
#define TOUCHSESTIVITY_S0 2
#define TOUCHSESTIVITY_S1 6
#define TOUCHSESTIVITY_S2 2
#define TOUCHSESTIVITY_S3 2
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
        if(IsEventReleased == 0)
        {
           IsEventReleased = 1;
           EventValue = EVENT_OK;
        }
        else
        {
          EventValue = EVENT_NONE;
        }
      }
      else if(sensor_0 < THRESHOLD_SENSOR_0 )
      {
        TouchSenstivity_Debounce++;
        if(TouchSenstivity_Debounce == TOUCHSESTIVITY_S0)
        {
          TouchSenstivity_Debounce = 0;
          if(IsEventReleased == 0)
          {
            IsEventReleased = 1;
            EventValue = EVENT_ON_OFF;
          }
          else
          {
            EventValue = EVENT_NONE;
          }
        }
      }
      else if(sensor_1 < THRESHOLD_SENSOR_1 )
      {
          TouchSenstivity_Debounce++;
          if(TouchSenstivity_Debounce == TOUCHSESTIVITY_S1)
          {
            TouchSenstivity_Debounce = 0;
            if(IsEventReleased == 0)
            {
              IsEventReleased = 1;
              EventValue = EVENT_INFO;
            }
            else
            {
              EventValue = EVENT_NONE;
            }
          }
      }
      else if(sensor_2 < THRESHOLD_SENSOR_2 )
      {
          TouchSenstivity_Debounce++;
          if(TouchSenstivity_Debounce == TOUCHSESTIVITY_S2)
          {
            TouchSenstivity_Debounce = 0;
            if(IsEventReleased == 0)
            {
              IsEventReleased = 1;
              EventValue = EVENT_CANCLE;
            }
            else
            {
              EventValue = EVENT_NONE;
            }
          }
      }
      else if(sensor_3 < THRESHOLD_SENSOR_3 )
      {
          TouchSenstivity_Debounce++;
          if(TouchSenstivity_Debounce == TOUCHSESTIVITY_S3)
          {
            TouchSenstivity_Debounce = 0;
            if(IsEventReleased == 0)
            {
              IsEventReleased = 1;
              EventValue = EVENT_START;
            }
            else
            {
              EventValue = EVENT_NONE;
            }
          }
      }
      else
      {
        TouchSenstivity_Debounce = 0;
        IsEventReleased = 0;
        EventValue = EVENT_NONE;
      }
//    Serial.println(sensor_0);
//    Serial.println(IsEventReleased);
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
      StateMenuView();
      StateMenuEventHandler();
      break;
    case STATE_TEMPERATUR:
      break;
    case STATE_DURATION:
    StateCookTimeView();
    StateDurationEventHandler();
      break;
    case STATE_RUN:
      StateRunningView();
      StateRunEventHandler();
      break;
    case STATE_END:
      StateEndView();
      StateEndEventHandler();
      break;
    case STATE_SETTING:
      break;
    case STATE_SETTING_WIFI:
      break;
    case STATE_STANDBY:
      break;
    case STATE_MW_POWER:
      StateMwPowerView();
      StateMwPowerEventHandler();
      break;
    case STATE_MW_GRILL:
      break;
    default:
      break;
  }
}
void StatePowerOnView(void)
{
  if(PowerOnCounter == 0)
  {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(3);
    tft.setCursor(40, 135);
    tft.println("Whirlpool");
  }
//  tft.drawBitmap(0,0,WhirlpoolLogo,320,240,0);
  PowerOnCounter++;
  if(PowerOnCounter > 1000 )
  {
    PowerOnCounter = 0;
    CurrentState = STATE_IDLE;
    tft.fillScreen(ILI9341_BLACK);
  }
}
void ClockBackGroundHandler(void)
{
  ClockCounter++;
  if(ClockCounter > 500 )
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
int ClockUpdateCounter = 0;
void StateIdleView(void)
{
  ClockUpdateCounter++;
  if(ClockUpdateCounter > 500)
  {
    ClockUpdateCounter = 0;
  //  delay(100);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(100, 115);
    tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
    tft.println(String(Clock_H1));
    tft.setCursor(133, 115);
    tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
    tft.println(String(Clock_H2));
    tft.setCursor(160, 115);
    tft.println(":");
    tft.setCursor(187, 115);
    tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
    tft.println(String(Clock_M1));
    tft.setCursor(220, 115);
    tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
    tft.println(String(Clock_M2));
  }
}
void StateIdleEventHandler(void)
{
  End_UpdateDisplay = 0;
  RunCookTime_UpdateDisplay = 0;
  CookTime_UpdateDisplay = 0;
  MwPower_UpdateDisplay = 0;
  Mw_PowerLevel = 8;

  UpDateDisplay = 0;
  MenuSelection_Counter = 0;
  MenuSelected = 0;
 CookTime_S1 = 0;
 CookTime_S2 = 0;
 CookTime_M1 = 0;
 CookTime_M2 = 0;
 CookTime_H1 = 0;
 CookTime_H2 = 0;
 CookTimeViewInit = 0;
 RunCookTime_S1 = 0;
 RunCookTime_S2 = 0;
 RunCookTime_M1 = 0;
 RunCookTime_M2 = 0;
 RunCookTime_H1 = 0;
 RunCookTime_H2 = 0;
 RunCookTime_Init = 0;
 RunGameMode = 0;

  if(EventValue == EVENT_ON_OFF)
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_MENU;
    tft.fillScreen(ILI9341_BLACK);
    drawCookingMenuScreen1(0);
  }
}

void StateMenuView(void)
{

  if(EventValue == EVENT_DOWN)
  {
    MenuSelection_Counter++;
    if(MenuSelection_Counter > 100)
    {
      MenuSelection_Counter = 0;
      MenuSelected++;
      if(MenuSelected == 4)
      {
        UpDateDisplay = 0;
      }
      else if(MenuSelected > 6)
      {
        MenuSelected = 0;
        UpDateDisplay = 0;
      }
      MenuInit(MenuSelected-1);
      MenuSelection(MenuSelected);
      MenuFunctionSelected(MenuSelected-1);
      MenuFunctionSelected(MenuSelected);
    }
  }
  else if(EventValue == EVENT_UP)
  {
    MenuSelection_Counter++;
    if(MenuSelection_Counter > 100)
    {
      MenuSelection_Counter = 0;
      MenuSelected--;
      if(MenuSelected == 4)
      {
        UpDateDisplay = 0;
      }
      else if(MenuSelected < 0)
      {
        MenuSelected = 6;
        UpDateDisplay = 0;
      }
      MenuInit(MenuSelected+1);
      MenuSelection(MenuSelected);
      MenuFunctionSelected(MenuSelected+1);
      MenuFunctionSelected(MenuSelected);
    }
  }
  else
  {
    MenuSelection_Counter = 0;
  }

  if(UpDateDisplay == 0)
  {
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
    drawCookingMenuScreen1(MenuSelected);
    MenuSelection(MenuSelected);
    MenuFunctionSelected(MenuSelected);
    UpDateDisplay = 1;
  }
}
void drawCookingMenuScreen1(int counter)
{
  tft.setTextSize(2);

  if(counter < 4)
  {
    // 1st Box
      tft.drawRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.fillRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.drawRect(45, 15, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.fillRect(45, 15, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.drawRect(50, 20, 200, 45, ILI9341_BLACK);
      tft.fillRect(50, 20, 200, 45, ILI9341_BLACK);
      tft.setCursor(65, 50);
      tft.print(menu[1]);

    //2nd Box
      tft.drawRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.fillRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.drawRect(45, 90, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.fillRect(45, 90, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.drawRect(50, 95, 200, 45, ILI9341_BLACK);
      tft.fillRect(50, 95, 200, 45, ILI9341_BLACK);
      tft.setCursor(100, 125);
      tft.print(menu[2]);

      //3nd Box
      tft.drawRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.fillRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.drawRect(45, 165, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.fillRect(45, 165, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.drawRect(50, 170, 200, 45, ILI9341_BLACK);
      tft.fillRect(50, 170, 200, 45, ILI9341_BLACK);
      tft.setCursor(65, 200);
      tft.print(menu[3]);
  }
  else
  {
        // 1st Box
      tft.drawRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.fillRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.drawRect(15, 15, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.fillRect(15, 15, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.drawRect(20, 20, 260, 45, ILI9341_BLACK);
      tft.fillRect(20, 20, 260, 45, ILI9341_BLACK);
      tft.setCursor(25, 50);
      tft.print(menu[4]);

    //2nd Box
      tft.drawRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.fillRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.drawRect(15, 90, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.fillRect(15, 90, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.drawRect(20, 95, 260, 45, ILI9341_BLACK);
      tft.fillRect(20, 95, 260, 45, ILI9341_BLACK);
      tft.setCursor(35, 125);
      tft.print(menu[5]);

      //3nd Box
      tft.drawRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.fillRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      tft.drawRect(15, 165, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.fillRect(15, 165, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.drawRect(20, 170, 260, 45, ILI9341_BLACK);
      tft.fillRect(20, 170, 260, 45, ILI9341_BLACK);
      tft.setCursor(80, 200);
      tft.print(menu[6]);
  }
}
void MenuInit(int menuitem)
{
      switch(menuitem)
    {
      case 0:
      break;

      case 1:
    // 1st Box
          tft.drawRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
          tft.fillRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      break;

      case 2:
          tft.drawRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
          tft.fillRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      break;

      case 3:
          tft.drawRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
          tft.fillRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      break;

      case 4:
          tft.drawRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
          tft.fillRect(10, 10, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      break;

      case 5:
          tft.drawRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
          tft.fillRect(10, 85, 280, 65, LCD_MENU_OUTER_REC_COLOR);
      break;
      case 6:
          tft.drawRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
          tft.fillRect(10, 160, 280, 65, LCD_MENU_OUTER_REC_COLOR);
        break;
    }
}
void MenuSelection(int menuitem)
{
      switch(menuitem)
    {
      case 0:
      break;

      case 1:
    // 1st Box
          tft.drawRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.fillRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
      break;

      case 2:
          tft.drawRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.fillRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
      break;

      case 3:
          tft.drawRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.fillRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
      break;

      case 4:
          tft.drawRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.fillRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
      break;

      case 5:
          tft.drawRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.fillRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
      break;
      case 6:
          tft.drawRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.fillRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
        break;
    }
}
void MenuFunctionSelected(char menuitem)
{
    switch(menuitem)
    {
      case 0:
      break;

      case 1:
    // 1st Box
//      tft.drawRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
//      tft.fillRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
      tft.drawRect(45, 15, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.fillRect(45, 15, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
      tft.drawRect(50, 20, 200, 45, ILI9341_BLACK);
      tft.fillRect(50, 20, 200, 45, ILI9341_BLACK);
      tft.setCursor(65, 50);
      tft.print(menu[1]);
      break;

      case 2:
//          tft.drawRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
//          tft.fillRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.drawRect(45, 90, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.fillRect(45, 90, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.drawRect(50, 95, 200, 45, ILI9341_BLACK);
          tft.fillRect(50, 95, 200, 45, ILI9341_BLACK);
          tft.setCursor(100, 125);
          tft.print(menu[2]);
      break;

      case 3:
//          tft.drawRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
//          tft.fillRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.drawRect(45, 165, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.fillRect(45, 165, 210, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.drawRect(50, 170, 200, 45, ILI9341_BLACK);
          tft.fillRect(50, 170, 200, 45, ILI9341_BLACK);
          tft.setCursor(65, 200);
          tft.print(menu[3]);
      break;

      case 4:
//          tft.drawRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
//          tft.fillRect(10, 10, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.drawRect(15, 15, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.fillRect(15, 15, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.drawRect(20, 20, 260, 45, ILI9341_BLACK);
          tft.fillRect(20, 20, 260, 45, ILI9341_BLACK);
          tft.setCursor(25, 50);
          tft.print(menu[4]);
      break;

      case 5:
//          tft.drawRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
//          tft.fillRect(10, 85, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.drawRect(15, 90, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.fillRect(15, 90, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.drawRect(20, 95, 260, 45, ILI9341_BLACK);
          tft.fillRect(20, 95, 260, 45, ILI9341_BLACK);
          tft.setCursor(35, 125);
          tft.print(menu[5]);
      break;
      case 6:
//          tft.drawRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
//          tft.fillRect(10, 160, 280, 65, LCD_MENU_SELECTED_REC_COLOR);
          tft.drawRect(15, 165, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.fillRect(15, 165, 270, 55, LCD_MENU_MIDLE_REC_COLOR);
          tft.drawRect(20, 170, 260, 45, ILI9341_BLACK);
          tft.fillRect(20, 170, 260, 45, ILI9341_BLACK);
          tft.setCursor(80, 200);
          tft.print(menu[6]);
        break;
    }
}
void StateMenuEventHandler(void)
{
  if(EventValue == EVENT_ON_OFF)
  {
    EventValue = EVENT_NONE;
      CurrentState = STATE_IDLE;
      UpDateDisplay = 0;
      tft.fillScreen(ILI9341_BLACK);
  }
  else if(EventValue == EVENT_OK)
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_MW_POWER;
    UpDateDisplay = 0;
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
  }
}

void StateMwPowerView(void)
{
  if(EventValue == EVENT_RIGHT)
  {
    MenuSelection_Counter++;
    if(MenuSelection_Counter > 100)
    {
      MenuSelection_Counter = 0;
      MwPower_UpdateDisplay = 0;
      Mw_PowerLevel++;
      if(Mw_PowerLevel > 10)
      {
        Mw_PowerLevel = 1;
      }
    }
  }
  else if(EventValue == EVENT_LEFT)
  {
    MenuSelection_Counter++;
    if(MenuSelection_Counter > 100)
    {
      MenuSelection_Counter = 0;
      MwPower_UpdateDisplay = 0;
      Mw_PowerLevel--;
      if(Mw_PowerLevel < 1)
      {
        Mw_PowerLevel = 10;
      }
    }
  }
  else
  {
    MenuSelection_Counter = 0;
  }

  if(MwPower_UpdateDisplay == 0)
  {
    MwPower_UpdateDisplay = 1;
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
    MwPowerDrawScreen(Mw_PowerLevel);
  }
}
void MwPowerDrawScreen(int power_level)
{
  tft.setTextSize(2);
  tft.drawRect(10, 10, 300, 65, ILI9341_CYAN);
  tft.fillRect(10, 10, 300, 65, ILI9341_CYAN);
  tft.drawRect(20, 20, 280, 45, ILI9341_BLACK);
  tft.fillRect(20, 20, 280, 45, ILI9341_BLACK);
  tft.setCursor(30, 50);
  tft.print(menu[MenuSelected]);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK,LCD_BACKGROUND_MENU_COLOR);
  tft.setCursor(35, 130);
  if(power_level == 1)
  {
     sprintf(PowerString, "%d%%", PowerTable[10]);
  }
  else
  {
     sprintf(PowerString, "%d%%", PowerTable[power_level-1]);
  }
  tft.print(PowerString);

  tft.setTextSize(2);
  tft.setCursor(135, 130);
  sprintf(PowerString, "%d%%", PowerTable[power_level]);
  tft.print(PowerString);

  tft.setTextSize(1);
  tft.setCursor(245, 130);
  if(power_level == 10)
  {
     sprintf(PowerString, "%d%%", PowerTable[1]);
  }
  else
  {
    sprintf(PowerString, "%d%%", PowerTable[power_level+1]);
  }

  tft.print(PowerString);

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
  tft.drawRect(10, 190, 300, 45, ILI9341_CYAN);
  tft.fillRect(10, 190, 300, 45, ILI9341_CYAN);
  tft.drawRect(20, 200, 280, 25, ILI9341_BLACK);
  tft.fillRect(20, 200, 280, 25, ILI9341_BLACK);
  tft.setCursor(100, 215);
  tft.print("Power Level");
}
void StateMwPowerEventHandler(void)
{
  if(EventValue == EVENT_ON_OFF)
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_IDLE;
    MwPower_UpdateDisplay = 0;
    Mw_PowerLevel = 80;
    tft.fillScreen(ILI9341_BLACK);
  }
  else if(EventValue == EVENT_OK)
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_DURATION;
    UpDateDisplay = 0;
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
    MwPower_UpdateDisplay = 0;
  }
}

void StateCookTimeView(void)
{
  if(EventValue == EVENT_RIGHT)
  {
    MenuSelection_Counter++;
    if(MenuSelection_Counter > 100)
    {
      Serial.println(EventValue);
      MenuSelection_Counter = 0;
      CookTime_UpdateDisplay = 0;
      CookTime_S1 = CookTime_S1+5;
      if(CookTime_S1 >= 10 )
      {
        CookTime_S1 = 0;
        CookTime_S2++;
        if(CookTime_S2 > 5)
        {
          CookTime_S2 = 0;
          CookTime_M1++;
          if(CookTime_M1 > 9)
          {
            CookTime_M1 = 0;
            CookTime_M2++;
            if(CookTime_M2 > 9)
            {
              CookTime_M2 = 0;
              CookTime_M1 = 0;
              CookTime_S1 = 0;
              CookTime_S2 = 0;
            }
          }
        }
      }
    }
  }
  else if(EventValue == EVENT_LEFT)
  {
    MenuSelection_Counter++;
    if(MenuSelection_Counter > 100)
    {
      Serial.println(EventValue);
      MenuSelection_Counter = 0;
      CookTime_UpdateDisplay = 0;

      if((CookTime_S2>0)||(CookTime_M1>0)||(CookTime_M2>0))
      {
        CookTime_S1 = CookTime_S1-5;
        if(CookTime_S1 <= 0 )
        {
          CookTime_S1 = 0;
          CookTime_S2--;
          if(CookTime_S2 <= 0)
          {
            CookTime_S2 = 5;
            CookTime_M1--;
            if(CookTime_M1 <= 0)
            {
              CookTime_M1 = 9;
              CookTime_M2--;
              if(CookTime_M2 < 0)
              {
                CookTime_M2 = 0;
                CookTime_M1 = 0;
                CookTime_S1 = 0;
                CookTime_S2 = 0;
              }
            }
          }
        }
      }
    }
  }
  else
  {
    MenuSelection_Counter = 0;
  }

  if(CookTime_UpdateDisplay == 0)
  {
    CookTime_UpdateDisplay = 1;
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
    DurationDrwaScreen();
  }
}
void DurationDrwaScreen(void)
{
  tft.setTextSize(2);
  tft.drawRect(10, 10, 300, 65, ILI9341_CYAN);
  tft.fillRect(10, 10, 300, 65, ILI9341_CYAN);
  tft.drawRect(20, 20, 280, 45, ILI9341_BLACK);
  tft.fillRect(20, 20, 280, 45, ILI9341_BLACK);
  tft.setCursor(30, 50);
  tft.print(menu[MenuSelected]);

  tft.setTextColor(ILI9341_BLACK,LCD_BACKGROUND_MENU_COLOR);
  tft.setCursor(80, 125);
  tft.println(String(CookTime_H2));
  tft.setCursor(100, 125);
  tft.println(String(CookTime_H1));
  tft.setCursor(120, 125);
  tft.println(":");
  tft.setCursor(140, 125);
  tft.println(String(CookTime_M2));
  tft.setCursor(160, 125);
  tft.println(String(CookTime_M1));
  tft.setCursor(180, 125);
  tft.println(":");
  tft.setCursor(200, 125);
  tft.println(String(CookTime_S2));
  tft.setCursor(220, 125);
  tft.println(String(CookTime_S1));

  tft.setTextSize(1);
  tft.setCursor(110, 150);
  tft.println("HM:MM:SS");

  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
  tft.drawRect(10, 190, 300, 45, ILI9341_CYAN);
  tft.fillRect(10, 190, 300, 45, ILI9341_CYAN);
  tft.drawRect(20, 200, 280, 25, ILI9341_BLACK);
  tft.fillRect(20, 200, 280, 25, ILI9341_BLACK);
  tft.setCursor(100, 215);
  tft.print("Cook Time");
}
void StateDurationEventHandler(void)
{
  if(EventValue == EVENT_ON_OFF)
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_IDLE;
    CookTime_UpdateDisplay = 0;
    CookTime_S1 = 0;
    CookTime_S2 = 0;
    CookTime_M1 = 0;
    CookTime_M2 = 0;
    CookTime_H1 = 0;
    CookTime_H2 = 0;
    tft.fillScreen(ILI9341_BLACK);
    CookTimeViewInit = 0;
    CookTime_UpdateDisplay = 0;
  }
  else if((EventValue == EVENT_UP) || (EventValue == EVENT_DOWN))
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_MW_POWER;
    CookTimeViewInit = 0;
    CookTime_UpdateDisplay = 0;
    MwPower_UpdateDisplay = 0;
  }
  else if((EventValue == EVENT_START)||(EventValue == EVENT_OK))
  {
  EventValue = EVENT_NONE;
  CurrentState = STATE_RUN;
  CookTimeViewInit = 0;
  CookTime_UpdateDisplay = 0;
  }
}

void StateRunningInit(void)
{
    tft.setTextSize(2);
    tft.drawRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.fillRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.drawRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.fillRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.setCursor(30, 50);
    tft.print(menu[MenuSelected]);

    RunCookTime_S1 = CookTime_S1;
    RunCookTime_S2 = CookTime_S2;
    RunCookTime_M1 = CookTime_M1;
    RunCookTime_M2 = CookTime_M2;
    RunCookTime_H1 = CookTime_H1;
    RunCookTime_H2 = CookTime_H2;

    tft.setTextColor(ILI9341_BLACK,LCD_BACKGROUND_MENU_COLOR);
    tft.setCursor(80, 125);
    tft.println(String(RunCookTime_H2));
    tft.setCursor(100, 125);
    tft.println(String(RunCookTime_H1));
    tft.setCursor(120, 115);
    tft.println(":");
    tft.setCursor(140, 125);
    tft.println(String(RunCookTime_M2));
    tft.setCursor(160, 115);
    tft.println(String(RunCookTime_M1));
    tft.setCursor(180, 125);
    tft.println(":");
    tft.setCursor(200, 125);
    tft.println(String(RunCookTime_S2));
    tft.setCursor(220, 125);
    tft.println(String(RunCookTime_S1));

    tft.setTextSize(1);
    tft.setCursor(110, 150);
    tft.println("HM:MM:SS");

    tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
    tft.drawRect(10, 190, 300, 45, ILI9341_CYAN);
    tft.fillRect(10, 190, 300, 45, ILI9341_CYAN);
    tft.drawRect(20, 200, 280, 25, ILI9341_BLACK);
    tft.fillRect(20, 200, 280, 25, ILI9341_BLACK);
    tft.setCursor(100, 215);
    tft.print("MWO Power : ");
    tft.setCursor(130, 215);
    tft.print(String(PowerTable[Mw_PowerLevel]));
    tft.setCursor(140, 215);
    tft.print(String("%"));
}
void StateRunningView(void)
{
  if(RunCookTime_Init == 0)
  {
    RunCookTime_Init = 1;
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
    StateRunningInit();
  }

  MenuSelection_Counter++;
  if(MenuSelection_Counter > 600)
  {
    if((CookTime_S1 == 0)&&(CookTime_S2==0))
    {
      CurrentState = STATE_END;
    }

    RunCookTime_UpdateDisplay = 0;
    MenuSelection_Counter = 0;
    if(CookTime_S1 > 0)
    {
      CookTime_S1--;
      RunCookTime_S1 = CookTime_S1;
    }
    else
    {
      if(CookTime_S2 > 0)
      {
        CookTime_S2--;
        RunCookTime_S2 = CookTime_S2;
        CookTime_S1 = 9;
      }
    }
  }

  if(RunCookTime_UpdateDisplay == 0)
  {
    RunCookTime_UpdateDisplay = 1;
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
    RunningDrwaScreen();
  }
}

void RunningDrwaScreen(void)
{
  if(RunGameMode == 0)
  {
    tft.setTextSize(2);
    tft.drawRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.fillRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.drawRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.fillRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.setCursor(30, 50);
    tft.print(menu[MenuSelected]);

    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK,LCD_BACKGROUND_MENU_COLOR);
    tft.setCursor(80, 125);
    tft.println(String(RunCookTime_H2));
    tft.setCursor(100, 125);
    tft.println(String(RunCookTime_H1));
    tft.setCursor(120, 125);
    tft.println(":");
    tft.setCursor(140, 125);
    tft.println(String(RunCookTime_M2));
    tft.setCursor(160, 125);
    tft.println(String(RunCookTime_M1));
    tft.setCursor(180, 125);
    tft.println(":");
    tft.setCursor(200, 125);
    tft.println(String(RunCookTime_S2));
    tft.setCursor(220, 125);
    tft.println(String(RunCookTime_S1));

    tft.setTextSize(1);
    tft.setCursor(110, 150);
    tft.println("HM:MM:SS");

    tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
    tft.drawRect(10, 190, 300, 45, ILI9341_CYAN);
    tft.fillRect(10, 190, 300, 45, ILI9341_CYAN);
    tft.drawRect(20, 200, 280, 25, ILI9341_BLACK);
    tft.fillRect(20, 200, 280, 25, ILI9341_BLACK);
    tft.setCursor(30, 215);
    tft.print("MWO Power : ");
    tft.setCursor(190, 215);
    tft.print(String(PowerTable[Mw_PowerLevel]));
    tft.setCursor(220, 215);
    tft.print(String("%"));
  }
}
void StateRunEventHandler(void)
{
  if(EventValue == EVENT_ON_OFF)
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_IDLE;
    CookTime_UpdateDisplay = 0;
    CookTime_S1 = 0;
    CookTime_S2 = 0;
    CookTime_M1 = 0;
    CookTime_M2 = 0;
    CookTime_H1 = 0;
    CookTime_H2 = 0;
    tft.fillScreen(ILI9341_BLACK);
  }
  else if((EventValue == EVENT_UP) || (EventValue == EVENT_DOWN))
  {
//    EventValue = EVENT_NONE;
//    CurrentState = STATE_MW_POWER;
//    CookTime_UpdateDisplay = 0;
//    MwPower_UpdateDisplay = 0;
  }
  else if(EventValue == EVENT_INFO )
  {
    RunGameMode = (RunGameMode==0)?1:0;
    if(RunGameMode == 1)
    {
      gameInit();
    }
    else
    {
      tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
      RunGameMode = 0;
    headX = 1;              //reset snake
    headY = 1;
    changeX = 0;
    changeY = 1;
    lastMoveH = false;

  memset(beenHeadX, 0, 470); //clear the beenHead arrays
  memset(beenHeadY, 0, 470); //probably not necessary

      loopCount = 0;
      clearScore = true;
    start = false;     //stops game
    delay(50);
      tft.setTextSize(2);
    tft.drawRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.fillRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.drawRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.fillRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.setCursor(30, 50);
    tft.print(menu[MenuSelected]);
    }
  }
}


void StateEndView(void)
{
  if(End_UpdateDisplay == 0)
  {
    tft.fillScreen(LCD_BACKGROUND_MENU_COLOR);
    End_UpdateDisplay = 1;
    EndDrwaScreen();
  }
}
void EndDrwaScreen(void)
{
    tft.setTextSize(2);
    tft.drawRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.fillRect(10, 10, 300, 65, ILI9341_CYAN);
    tft.drawRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.fillRect(20, 20, 280, 45, ILI9341_BLACK);
    tft.setCursor(30, 50);
    tft.print(menu[MenuSelected]);

    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK,LCD_BACKGROUND_MENU_COLOR);
    tft.setCursor(2, 130);
    tft.println("Cooking Completed");

    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
    tft.drawRect(10, 190, 300, 45, ILI9341_CYAN);
    tft.fillRect(10, 190, 300, 45, ILI9341_CYAN);
    tft.drawRect(20, 200, 280, 25, ILI9341_BLACK);
    tft.fillRect(20, 200, 280, 25, ILI9341_BLACK);
    tft.setCursor(30, 215);
    tft.print("MWO Power : ");
    tft.setCursor(190, 215);
    tft.print(String(PowerTable[Mw_PowerLevel]));
    tft.setCursor(220, 215);
    tft.print(String("%"));
}
void StateEndEventHandler(void)
{
  if((EventValue == EVENT_ON_OFF)||(EventValue == EVENT_CANCLE))
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_IDLE;
    End_UpdateDisplay = 0;
    tft.fillScreen(ILI9341_BLACK);
  }
  else if((EventValue == EVENT_UP) || (EventValue == EVENT_DOWN))
  {
    EventValue = EVENT_NONE;
    CurrentState = STATE_MENU;
    End_UpdateDisplay = 0;
  }
}

//======================================================================Game============================================================================================================
void gameInit(void)
{
  gs = 1000 / gameSpeed; //calculated gameSpeed in milliseconds

  memset(beenHeadX, 0, 470); //initiate beenHead with a bunch of zeros
  memset(beenHeadY, 0, 470);

//  tft.begin();           //turn on display
//  tft.setRotation(0);

  tft.fillScreen(ILI9341_WHITE);                //sets background
  tft.fillRect(3, 21, 316, 226, ILI9341_BLUE);

//  tft.setTextColor(ILI9341_WHITE); //Start notification
//  tft.setTextSize(3);
//  tft.setCursor(80, 90);
//  tft.print(">START<");

  tft.setTextColor(ILI9341_BLACK); //Score keeper
  tft.setTextSize(1);
  tft.setCursor(5, 12);
  tft.print("Length: ");
  printScore();

  randomSeed(547); //make every game unique
  select();
}
void gameLoopFunc(void)
{
  delay(50);
  if( EventValue == EVENT_UP )
  {
    up();
  }
  else if( EventValue == EVENT_DOWN )
  {
    down();
  }
  else if( EventValue == EVENT_LEFT )
  {
    left();
  }
  else if( EventValue == EVENT_RIGHT )
  {
    right();
  }
  else if( EventValue == EVENT_OK )
  {
    select();
  }

  if (clearScore and start) { //resets score from last game, won't clear
    score = 1;                //until new game starts so you can show off
    printScore();             //your own score
    clearScore = false;
  }
  if (millis() - offsetM > gs and start) {
    beenHeadX[loopCount] = headX;  //adds current head coordinates to be
    beenHeadY[loopCount] = headY;  //covered later

    headX = headX + (changeX);  //head moved
    headY = headY + (changeY);

    if (headX - foodX == 0 and headY - foodY == 0) { //food
      score += 1;
      printScore();
      eaten = true;
    }

    loopCount += 1; //loopCount used for addressing, mostly

    if (loopCount > 467) {            //if loopCount exceeds size of
      clearPoint = loopCount - score; //beenHead arrays, reset to zero
      loopCount = 0;
    }

    drawDot(headX, headY); //head is drawn

    if (loopCount - score >= 0) { //if array has not been reset
      eraseDot(beenHeadX[loopCount - score], beenHeadY[loopCount - score]);
    }  //covers end of tail
    else {
      eraseDot(beenHeadX[clearPoint], beenHeadY[clearPoint]);
      clearPoint += 1;
    }

    if (eaten) {     //randomly create a new piece of food if last was eaten
      foodX = random(2, 26);
      foodY = random(2, 18);
      eaten = false;
    }

    drawDotRed(foodX, foodY); //draw the food

    if (headX > 26 or headX < 1 or headY < 1 or headY > 18) { //Boudaries
      endGame();
    }

    if (loopCount - score < 0) {         //check to see if head is on tail
      for (int j = 0; j < loopCount; j++) {
        if (headX == beenHeadX[j] and headY == beenHeadY[j]) {
          endGame();
        }
      }
      for (int k = clearPoint; k < 467; k++) {
        if (headX == beenHeadX[k] and headY == beenHeadY[k]) {
          endGame();
        }
      }
    }
    else {
      for (int i = loopCount - (score - 1); i < loopCount; i++) {
        if (headX == beenHeadX[i] and headY == beenHeadY[i]) {
          endGame();
        }
      }
    }

    offsetM = millis(); //reset game loop timer
  }
}

void endGame() {
  RunGameMode = 0;
  tft.fillRect(3, 21, 316, 226, ILI9341_BLUE); //deletes the old game

  eaten = true; //new food will be created

  tft.setCursor(80, 90);       //Retry message
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("RETRY?");
  tft.setTextColor(ILI9341_BLACK); //sets back to scoreboard settings
  tft.setTextSize(1);

  tft.setCursor(5, 12);
  tft.print("Length: ");

  headX = 1;              //reset snake
  headY = 1;
  changeX = 0;
  changeY = 1;
  lastMoveH = false;

  memset(beenHeadX, 0, 470); //clear the beenHead arrays
  memset(beenHeadY, 0, 470); //probably not necessary

  loopCount = 0;
  clearScore = true;
  start = false;     //stops game
}

void drawDot(int x, int y) {
  tft.fillRect(12*(x-1)+5, 12*(y-1)+23, 10, 10, ILI9341_WHITE);
}

void drawDotRed(int x, int y) {
  tft.fillRect(12*(x-1)+5, 12*(y-1)+23, 10, 10, ILI9341_RED);
}

void eraseDot(int x, int y) {
  tft.fillRect(12*(x-1)+5, 12*(y-1)+23, 10, 10, ILI9341_BLUE);
}

void printScore() {
  tft.fillRect(88, 3, 50, 16, ILI9341_WHITE);//clears old score
  tft.setCursor(88, 12);
  tft.print(score);                            //prints current score
}

void up() {
  if (millis() - offsetT > gs and lastMoveH) { //lastMoveH makes sure you can't go back on yourself
    changeX = 0;   //changes the direction of the snake
    changeY = -1;
    offsetT = millis();
    lastMoveH = false;
  }
}

void down() {
  if (millis() - offsetT > gs and lastMoveH) {
    changeX = 0;
    changeY = 1;
    offsetT = millis();
    lastMoveH = false;
  }
}

void left() {
  if (millis() - offsetT > gs and !lastMoveH) {
    changeX = -1;
    changeY = 0;
    offsetT = millis();
    lastMoveH = true;
  }
}

void right() {
  if (millis() - offsetT > gs and !lastMoveH) {
    changeX = 1;
    changeY = 0;
    offsetT = millis();
    lastMoveH = true;
  }
}

void select() {
  if (millis() - offsetT > gs and !start) {
    tft.fillRect(80, 90, 126, 24, ILI9341_BLUE); //Erase start message
    start = true;                                //allows loop to start
    offsetT = millis();
  }
}

