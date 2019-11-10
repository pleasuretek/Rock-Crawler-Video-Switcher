#include <Adafruit_SHT31.h>
#include <TVout.h>
#include <video_gen.h>
#include "font8x8.h"
#include "font4x6.h"
#include "font6x8.h"
//#include "font16x16.h"
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "RTClib.h"

#define turnShitTheFuckOn 10
#define camrear 0
#define pwrrear 1
#define camfl 2
#define pwrfl 3
#define camfr 4
#define pwrfr 5
#define camrl 6
#define pwrrl 7
#define camrr 8
#define pwrrr 9
#define hlhigh 8
#define hllow 6

#define but1 2
#define but2 3
#define revTrig 4
#define headlightTrig 5

Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_MCP23017 mcp;
TVout TV;
RTC_DS1307 rtc;

//enum eState {REAR, FL, FR, RL, RR};
const int CAMCOUNT = 4;
int camState;
int lastCamState;


int but1State;
int but1LastState = LOW;

int but2State;
int but2LastState = LOW;

int reverseLast = LOW;

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay = 30;

unsigned long updateFreq = 666;
unsigned long lastUpdate;

char daysOfTheWeek[7][4] = {"Sun ", "Mon ", "Tues", "Wed ", "Thur", "Fri ", "Sat "};

float tempF = 66.6;
long randNumber;

void setup() {
  // put your setup code here, to run once:
  //emulate button press to turn on screen
  pinMode(turnShitTheFuckOn,OUTPUT);
  lastUpdate = millis();
  delay(20);
  digitalWrite(turnShitTheFuckOn,HIGH);
  delay(1800);
  
  
  mcp.begin();
  delay(500);
  mcp.pinMode(camfl, OUTPUT);
  mcp.pinMode(camfr, OUTPUT);
  mcp.pinMode(camrl, OUTPUT);
  mcp.pinMode(camrr, OUTPUT);
  mcp.pinMode(camrear, OUTPUT);
  mcp.pinMode(pwrfl, OUTPUT);
  mcp.pinMode(pwrfr, OUTPUT);
  mcp.pinMode(pwrrl, OUTPUT);
  mcp.pinMode(pwrrr, OUTPUT);
  mcp.pinMode(pwrrear, OUTPUT);
  pinMode(hllow, OUTPUT);
  pinMode(hlhigh, OUTPUT);
  pinMode(but1, INPUT);
  pinMode(but2, INPUT);
  pinMode(revTrig, INPUT);
  pinMode(headlightTrig, INPUT);

  digitalWrite(hllow, HIGH);
  digitalWrite(hlhigh, LOW);
  mcp.digitalWrite(camrr, HIGH);
  mcp.digitalWrite(pwrrr, HIGH);
  digitalWrite(turnShitTheFuckOn,LOW);
    
  delay(400);
  camState = 4;
  lastCamState = 4;
  stateMachine(camState);
  TV.begin(NTSC, 128, 48);
  TV.force_vscale(4.0);
  //delay(3000);
  
  if (! rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    TV.println("Couldn't find RTC");
    delay(2000);
  }
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    TV.println("Couldn't find SHT31");
    delay(2000);
  }

  //set time to compile time
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
 // rtc.adjust(DateTime(2019, 10, 30, 19, 45, 0));
  
  //TV.begin(NTSC,120,90);
  //TV.begin(NTSC,64,48);
  
  TV.select_font(font6x8);
  TV.set_cursor(2, 2);
  TV.println("Fuck Yeah\n");
  TV.draw_rect(0, 0, TV.hres() - 1, TV.vres() - 1, WHITE);
  

  //delay(200);
  TV.println("");
  TV.set_cursor(2,22);
  TV.print("Loading cyberDemon");
  delay(600);
  TV.print(".");
  delay(600);
  TV.print(".");
  delay(600);
  TV.print(".");
  delay(800);
    
  
}

void loop() {
  //debounce button press bs in software instead of hardware capacitors and bs bs bs fuck that bs

  int reading1 = digitalRead(but1);
  if (reading1 != but1LastState) {
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (reading1 != but1State) {
      but1State = reading1;
      if (but1State == HIGH) {
        //set button 1 pushed bs here
        if (camState == CAMCOUNT) {
          camState = 0;
        } else {
          camState++;
        }
      }
    }
  }
  int reading2 = digitalRead(but2);
  if (reading2 != but2LastState) {
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading2 != but2State) {
      but2State = reading2;
      if (but2State == HIGH) {
        //set button 2 pushed bs here
        if (camState == 0) {
          camState = CAMCOUNT;
        } else {
          camState--;
        }
      }

    }
  }
  but1LastState = reading1;
  but2LastState = reading2;

  //check if reverse
  int reverse = digitalRead(revTrig);
  if (reverse != reverseLast) {
    if (reverse) {
      stateMachine(0);
      reverseLast = reverse;
    } else {
      stateMachine(camState);
      reverseLast = reverse;
    }
  } else if (camState != lastCamState) {
    stateMachine(camState);
    lastCamState = camState;
  }
  //check if headlights
  int hl = digitalRead(headlightTrig);
  if (hl) {
    digitalWrite(hllow, LOW);
    digitalWrite(hlhigh, HIGH);
  } else {
    digitalWrite(hllow, HIGH);
    digitalWrite(hlhigh, LOW);
  }

  printTime();

}

void printTime() {
  unsigned long current = millis();
  if ( current - lastUpdate > updateFreq ) {
    TV.clear_screen();
    //Time
    TV.select_font(font8x8);
    TV.set_cursor(4, 2);
    DateTime stamp = rtc.now();
    //uint8_t hour12 = stamp.hour()%12 ==0? 12 : stamp.hour()%12;
    int hour12 = stamp.hour() % 12 == 0 ? 12 : stamp.hour() % 12;
    TV.print(hour12);
    TV.print(":");
    if (stamp.minute() < 10)  TV.print("0") ;
    TV.print(stamp.minute(), DEC);

    //date
    TV.set_cursor(50, 2);
    TV.select_font(font6x8);
    TV.print(stamp.month(), DEC);
    TV.print('/');
    TV.print(stamp.day(), DEC);
    TV.print('/');
    TV.println(stamp.year(), DEC);

    //print name of day of week isn't working right... get some sleep
    TV.set_cursor(54, 10);
    TV.select_font(font6x8);
    switch (stamp.dayOfTheWeek()) {
      case 0:
        TV.print("Sunday");
        break;
      case 1:
        TV.print("Monday");
        break;
      case 2:
        TV.print("Tuesday");
        break;
      case 3:
        TV.print("Wednesday");
        break;
      case 4:
        TV.print("Thursday");
        break;
      case 5:
        TV.print("Friday");
        break;
      case 6:
        TV.print("Saterday");
        break;
    }

    randNumber = random(666);

    //temp humid bs
    TV.set_cursor(0, 20);
    TV.select_font(font6x8);
    float t = sht31.readTemperature();
    if (! isnan(t)) {  // check if 'is not a number'
      tempF = (t * 1.8) + 32.0;
      TV.print(tempF); TV.println("*F");
    } else {

      if (randNumber > 600) {
        TV.println("420");
      } else {
        TV.println("666F*");
        //TV.print(tempF); TV.println("*F");
      }
    }

    TV.set_cursor(60, 20);
    float h = sht31.readHumidity();
    if (! isnan(h)) {  // check if 'is not a number'
      TV.print(h); TV.println("%");
    } else {
      TV.println("666%");
    }

    //funnystuphz
    TV.set_cursor(2, 30);
    TV.select_font(font6x8);
    if (randNumber > 600) {
      TV.print("Fuck it  ");
      TV.print("Hail Satan");
    } else {
      TV.print("Fuck Yeah    ");
      TV.print("Send it");
    }
    lastUpdate = current;
    updateFreq = random(2666);
  }
}

void stateMachine(int st) {
  switch (st) {
    case 0:
      //Serial.println("case 0");
      mcp.digitalWrite(camrear , HIGH);
      mcp.digitalWrite(pwrrear , HIGH);
      mcp.digitalWrite(camfl   , LOW);
      mcp.digitalWrite(camfr   , LOW);
      mcp.digitalWrite(camrl   , LOW);
      mcp.digitalWrite(camrr   , LOW);
      mcp.digitalWrite(pwrfl   , LOW);
      mcp.digitalWrite(pwrfr   , LOW);
      mcp.digitalWrite(pwrrl   , LOW);
      mcp.digitalWrite(pwrrr   , LOW);
      break;
    case 1:
      //Serial.println("case 1");
      mcp.digitalWrite(camfl   , HIGH);
      mcp.digitalWrite(camrear , LOW);
      mcp.digitalWrite(camfr   , LOW);
      mcp.digitalWrite(camrl   , LOW);
      mcp.digitalWrite(camrr   , LOW);
      mcp.digitalWrite(pwrfl   , HIGH);
      mcp.digitalWrite(pwrrear , LOW);
      mcp.digitalWrite(pwrfr   , LOW);
      mcp.digitalWrite(pwrrl   , LOW);
      mcp.digitalWrite(pwrrr   , LOW);
      break;
    case 2:
      //Serial.println("case 2");
      mcp.digitalWrite(camfr   , HIGH);
      mcp.digitalWrite(camrear , LOW);
      mcp.digitalWrite(camfl   , LOW);
      mcp.digitalWrite(camrl   , LOW);
      mcp.digitalWrite(camrr   , LOW);
      mcp.digitalWrite(pwrfr   , HIGH);
      mcp.digitalWrite(pwrrear , LOW);
      mcp.digitalWrite(pwrfl   , LOW);
      mcp.digitalWrite(pwrrl   , LOW);
      mcp.digitalWrite(pwrrr   , LOW);
      break;
    case 3:
      //Serial.println("case 3");
      mcp.digitalWrite(camrl   , HIGH);
      mcp.digitalWrite(camrear , LOW);
      mcp.digitalWrite(camfl   , LOW);
      mcp.digitalWrite(camfr   , LOW);
      mcp.digitalWrite(camrr   , LOW);
      mcp.digitalWrite(pwrrl   , HIGH);
      mcp.digitalWrite(pwrrear , LOW);
      mcp.digitalWrite(pwrfl   , LOW);
      mcp.digitalWrite(pwrfr   , LOW);
      mcp.digitalWrite(pwrrr   , LOW);
      break;
    case 4:
      //Serial.println("case 4");
      mcp.digitalWrite(camrr   , HIGH);
      mcp.digitalWrite(camrear , LOW);
      mcp.digitalWrite(camfl   , LOW);
      mcp.digitalWrite(camfr   , LOW);
      mcp.digitalWrite(camrl   , LOW);
      mcp.digitalWrite(pwrrr   , HIGH);
      mcp.digitalWrite(pwrrear , LOW);
      mcp.digitalWrite(pwrfl   , LOW);
      mcp.digitalWrite(pwrfr   , LOW);
      mcp.digitalWrite(pwrrl   , LOW);
      break;
  }
}
