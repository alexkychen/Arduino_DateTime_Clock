#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <DS3232RTC.h> //https://github.com/JChristensen/DS3232RTC for real time clock
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_MLX90614.h> //Include IR contactless thermometer library
#include <EEPROM.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int taskIndex = 0;
int setDatetimeTX; //integer 0-5; 0:return 1:year 2:month 3:day 4:hour 5:minute
int setLEDTX; //integer 0-5; 0:return 1:turnOnHour 2:turnOnMin 3:turnOffHour 4:turnOffMin 5:dimDuration(in minute)


int menuIndex = 0; //integer 0-8
String mainMenuItems[] = {"1.Set date time ",
                          "2.Set B_LED time",
                          "3.Set C_LED time",
                          "4.Set W_LED time",
                          "5.Set H2O temp. ",
                          "6.Set H2O level ",
                          "7.Disp. Datetime",
                          "8.Disp. Temp/Lev",
                          "9.Disp. All     "};

//declare variables for datetime
int YEAR;
int MONTH;
int DAY;
int HOUR;
int MINUTE;

int backlightValue = 255;//for LCD backlight control

//time variables for LED, including time to turn on, turn on hours, dimming duration
int BLEDturnOnHour;
int BLEDturnOnMinute;
int BLEDturnOffHour;
int BLEDturnOffMinute;
int BLEDdimDuration;
int BLEDdelay;

int CLEDturnOnHour;
int CLEDturnOnMinute;
int CLEDturnOffHour;
int CLEDturnOffMinute;
int CLEDdimDuration;
int CLEDdelay;

int WLEDturnOnHour;
int WLEDturnOnMinute;
int WLEDturnOffHour;
int WLEDturnOffMinute;
int WLEDdimDuration;
int WLEDdelay;

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.0 comment the other threshold and use the one below:
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   
 return btnNONE;  // when all others fail, return this...
}

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Smart Aqua Light");
  lcd.setCursor(4,1);
  lcd.print("WELCOME!");
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  BLEDturnOnHour=EEPROM.read(0); BLEDturnOnMinute=EEPROM.read(1);BLEDturnOffHour=EEPROM.read(2);BLEDturnOffMinute=EEPROM.read(3);BLEDdimDuration=EEPROM.read(4);
  CLEDturnOnHour=EEPROM.read(5); CLEDturnOnMinute=EEPROM.read(6);CLEDturnOffHour=EEPROM.read(7);CLEDturnOffMinute=EEPROM.read(8);CLEDdimDuration=EEPROM.read(9);
  WLEDturnOnHour=EEPROM.read(10); WLEDturnOnMinute=EEPROM.read(11);WLEDturnOffHour=EEPROM.read(12);WLEDturnOffMinute=EEPROM.read(13);WLEDdimDuration=EEPROM.read(14);  
  Alarm.alarmRepeat(BLEDturnOnHour,BLEDturnOnMinute,0,BLEDcycleOn); 
  Alarm.alarmRepeat(CLEDturnOnHour,CLEDturnOnMinute,0,CLEDcycleOn); 
  Alarm.alarmRepeat(WLEDturnOnHour,WLEDturnOnMinute,0,WLEDcycleOn); 
  Alarm.alarmRepeat(BLEDturnOffHour,BLEDturnOffMinute,0,BLEDcycleOff); 
  Alarm.alarmRepeat(CLEDturnOffHour,CLEDturnOffMinute,0,CLEDcycleOff);
  Alarm.alarmRepeat(WLEDturnOffHour,WLEDturnOffMinute,0,WLEDcycleOff);
  
  //define datetime
  YEAR = year();
  MONTH = month();
  DAY = day();
  HOUR = hour();
  MINUTE = minute();
  delay(3000);
  lcd.clear();                                                                                                                                                                        
  checkBLED();delay(200);
  checkCLED();delay(200);
  checkWLED();delay(200);
}

void loop(){
  Alarm.delay(0); //call alarm to cycle LED
  switch (taskIndex){
    case 0:{ mainMenu(); break;} 
    case 1:{ //Set date time
      switch (setDatetimeTX){
        case 0: { setDatetime(); break;}
        case 1: { setYear(); break;}
        case 2: { setMonth(); break;}
        case 3: { setDay(); break;}
        case 4: { setHour(); break;}
        case 5: { setMin(); break;}
      } break;}
    case 2:{ 
      switch (setLEDTX){
        case 0: {setBLED(); break;}
        case 1: {setBLEDturnOnHour(); break;}
        case 2: {setBLEDturnOnMin(); break;}
        case 3: {setBLEDturnOffHour(); break;}
        case 4: {setBLEDturnOffMin(); break;}
        case 5: {setBLEDdimDur(); break;}
      } break;}
    case 3:{ 
      switch (setLEDTX){
        case 0: {setCLED(); break;}
        case 1: {setCLEDturnOnHour(); break;}
        case 2: {setCLEDturnOnMin(); break;}
        case 3: {setCLEDturnOffHour(); break;}
        case 4: {setCLEDturnOffMin(); break;}
        case 5: {setCLEDdimDur(); break;}
      } break;}    
    case 4:{ 
      switch (setLEDTX){
        case 0: {setWLED(); break;}
        case 1: {setWLEDturnOnHour(); break;}
        case 2: {setWLEDturnOnMin(); break;}
        case 3: {setWLEDturnOffHour(); break;}
        case 4: {setWLEDturnOffMin(); break;}
        case 5: {setWLEDdimDur(); break;}
      } break;}    
    case 7:{ dispDatetime(); break;}
  }
}

//display main menu and use UP/DOWN buttons to select menu item
void mainMenu() {
  int taskIndex = 0;
  lcd.setCursor(0,0);
  lcd.print("Main menu     /9");
  displayMenuItem();
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnDOWN:{
      menuIndex = menuIndex + 1;
      if (menuIndex >= 0 && menuIndex <= 8){displayMenuItem();}
      else{menuIndex = 8; break;}
      break;}
    case btnUP:{
      menuIndex = menuIndex - 1 ;
      if (menuIndex >= 0 && menuIndex <= 8){displayMenuItem();}
      else{menuIndex = 0; break;} 
      break;}
    case btnSELECT:{
      switch (menuIndex){
        case 0:{setDatetime(); break;}
        case 1:{setBLED(); break;}
        case 2:{setCLED(); break;}
        case 3:{setWLED(); break;}
        case 4:{setH2OTemp(); break;}
        case 5:{setH2OLevel(); break;}
        case 6:{dispDatetime(); break;}
        case 7:{dispTempLevel(); break;}
        case 8:{dispAll(); break;}
      }
    break;}
  }
}

//show menu item text
void displayMenuItem(){
  lcd.setCursor(13,0); //print a menu item numbder at the end of firs line on LCD
  lcd.print(menuIndex+1);
  lcd.setCursor(0,1); //print menu item text at second line on LCD
  lcd.print(mainMenuItems[menuIndex]);
  delay(150);
}
//Set up current date for return
void setDatetime(){
  taskIndex = 1;
  setDatetimeTX = 0;
  lcd.setCursor(0,0);
  lcd.print("Set Time<RETURN>");
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("  ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setDatetimeTX = setDatetimeTX + 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 5; break;} break;}
    case btnLEFT:{setDatetimeTX = setDatetimeTX - 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 0; break;} break;}
    case btnSELECT:{ taskIndex = 0; delay(350); break;}
  }
}
//set up datetime - year
void setYear(){
  taskIndex = 1;
  setDatetimeTX = 1;
  lcd.setCursor(0,0);
  lcd.print("Set Time <YEAR> ");
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print("  ");lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setDatetimeTX = setDatetimeTX + 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 5; break;} break;}
    case btnLEFT:{setDatetimeTX = setDatetimeTX - 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 0; break;} break;}
    case btnUP:{ YEAR = YEAR + 1; setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now()); delay(200); break;}
    case btnDOWN: { YEAR = YEAR - 1; setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now()); delay(200);break;}
  }
}
//set up datetime - month
void setMonth(){
  taskIndex = 1;
  setDatetimeTX = 2;
  lcd.setCursor(0,0);
  lcd.print("Set Time <MONTH>");
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");lcd.print("  ");lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setDatetimeTX = setDatetimeTX + 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 5; break;} break;}
    case btnLEFT:{setDatetimeTX = setDatetimeTX - 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 0; break;} break;}
    case btnUP:{ MONTH = MONTH + 1;
      if(MONTH>=1&&MONTH<=12){setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else {MONTH = 12; break;} break;}
    case btnDOWN:{MONTH = MONTH - 1;
      if (MONTH>=1&&MONTH<=12){setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else {MONTH = 1; break;} break;}
  }
}
//set up datetime - dayth
void setDay(){
  taskIndex = 1;
  setDatetimeTX = 3;
  lcd.setCursor(0,0);
  lcd.print("Set Time  <DAY> ");
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");lcd.print("  ");lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setDatetimeTX = setDatetimeTX + 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 5; break;} break;}
    case btnLEFT:{setDatetimeTX = setDatetimeTX - 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 0; break;} break;}
    case btnUP:{ DAY = DAY + 1;
      if (DAY>=1&&DAY<=31){ setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else {DAY = 31; break;} break;}
    case btnDOWN:{DAY = DAY - 1;
      if (DAY>=1&&DAY<=31){ setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else {DAY = 1; break;} break;}
  }
}
//Setup datetime - hour
void setHour(){
  taskIndex = 1;
  setDatetimeTX = 4;
  lcd.setCursor(0,0);
  lcd.print("Set Time <HOUR> ");
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");lcd.print("  ");lcd.print(":");print2digits(minute());
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setDatetimeTX = setDatetimeTX + 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 5; break;} break;}
    case btnLEFT:{setDatetimeTX = setDatetimeTX - 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 0; break;} break;}
    case btnUP:{HOUR = HOUR + 1;
      if(HOUR>=0&&HOUR<=23){setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else { HOUR = 23; break;} break;}
    case btnDOWN:{HOUR = HOUR - 1;
      if(HOUR>=0&&HOUR<=23){setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else {HOUR = 0; break;} break;} 
  } 
}
//Setup datetime -minute
void setMin(){
  taskIndex = 1;
  setDatetimeTX = 5;
  lcd.setCursor(0,0);
  lcd.print("Set Time<MINUTE>");
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");print2digits(minute());
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print(year()%100);lcd.print("/");print2digits(month());lcd.print("/");print2digits(day());lcd.print(" ");print2digits(hour());lcd.print(":");lcd.print("  ");
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setDatetimeTX = setDatetimeTX + 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 5; break;} break;}
    case btnLEFT:{setDatetimeTX = setDatetimeTX - 1; 
      if(setDatetimeTX >= 0 && setDatetimeTX<=5){ loop(); delay(200); break;}
      else {setDatetimeTX  = 0; break;} break;}
    case btnUP:{MINUTE = MINUTE + 1;
      if(MINUTE>=0&&MINUTE<=59){setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else { MINUTE = 59; break;} break;}
    case btnDOWN:{MINUTE = MINUTE - 1;
      if(MINUTE>=0&&MINUTE<=59){setTime(HOUR,MINUTE,01,DAY,MONTH,YEAR);RTC.set(now());; delay(200); break;}
      else {MINUTE = 0; break;} break;} 
  } 
}

//check LEDs current status 
void checkBLED(){
  HOUR = hour();
  MINUTE = minute();
  if (BLEDturnOnHour < BLEDturnOffHour){
    if (HOUR > BLEDturnOnHour && HOUR < BLEDturnOffHour){analogWrite(11,255);}
    else if (HOUR == BLEDturnOnHour && MINUTE > BLEDturnOnMinute){analogWrite(11,255);}
    else if (HOUR == BLEDturnOffHour && MINUTE < BLEDturnOffMinute){analogWrite(11,255);} }
  else if (BLEDturnOnHour > BLEDturnOffHour){
    if (HOUR > BLEDturnOnHour || HOUR < BLEDturnOffHour){analogWrite(11,255);}
    else if (HOUR == BLEDturnOnHour && MINUTE > BLEDturnOnMinute){analogWrite(11,255);}
    else if (HOUR == BLEDturnOffHour && MINUTE < BLEDturnOffMinute){analogWrite(11,255);} }
  else if (BLEDturnOnHour == BLEDturnOffHour && HOUR == BLEDturnOnHour){
    if (MINUTE > BLEDturnOnMinute){analogWrite(11,255);} }
}

void checkCLED(){
  HOUR = hour();
  MINUTE = minute();
  if (CLEDturnOnHour < CLEDturnOffHour){
    if (HOUR > CLEDturnOnHour && HOUR < CLEDturnOffHour){analogWrite(12,255);}
    else if (HOUR == CLEDturnOnHour && MINUTE > CLEDturnOnMinute){analogWrite(12,255);}
    else if (HOUR == CLEDturnOffHour && MINUTE < CLEDturnOffMinute){analogWrite(12,255);} }
  else if (CLEDturnOnHour > CLEDturnOffHour){
    if (HOUR > CLEDturnOnHour || HOUR < CLEDturnOffHour){analogWrite(12,255);}
    else if (HOUR == CLEDturnOnHour && MINUTE > CLEDturnOnMinute){analogWrite(12,255);}
    else if (HOUR == CLEDturnOffHour && MINUTE < CLEDturnOffMinute){analogWrite(12,255);} }
  else if (CLEDturnOnHour == CLEDturnOffHour && HOUR == CLEDturnOnHour){
    if (MINUTE > CLEDturnOnMinute){analogWrite(12,255);} }
}

void checkWLED(){
  HOUR = hour();
  MINUTE = minute();
  delay(10);
  if (WLEDturnOnHour < WLEDturnOffHour){
    if (HOUR > WLEDturnOnHour && HOUR < WLEDturnOffHour){analogWrite(13,255);}
    else if (HOUR == WLEDturnOnHour && MINUTE > WLEDturnOnMinute){analogWrite(13,255);}
    else if (HOUR == WLEDturnOffHour && MINUTE < WLEDturnOffMinute){analogWrite(13,255);} }
  else if (WLEDturnOnHour > WLEDturnOffHour){
    if (HOUR > WLEDturnOnHour || HOUR < WLEDturnOffHour){analogWrite(13,255);}
    else if (HOUR == WLEDturnOnHour && MINUTE > WLEDturnOnMinute){analogWrite(13,255);}
    else if (HOUR == WLEDturnOffHour && MINUTE < WLEDturnOffMinute){analogWrite(13,255);} }
  else if (WLEDturnOnHour == WLEDturnOffHour && HOUR == WLEDturnOnHour){
    if (MINUTE > WLEDturnOnMinute){analogWrite(13,255);} }
}
//set up Blue LED timer (for return)
void setBLED(){
  taskIndex = 2;
  setLEDTX = 0;
  lcd.setCursor(0,0);
  lcd.print("B_LED  <RETURN> ");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("  ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnSELECT:{ taskIndex = 0; delay(350); break;}
  }
}
//set up Blue LED timer (for turn on hour)
void setBLEDturnOnHour(){
  taskIndex = 2;
  setLEDTX = 1;
  lcd.setCursor(0,0);
  lcd.print("B_LED<TurnOn at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print("  ");lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { BLEDturnOnHour = BLEDturnOnHour + 1;
       if(BLEDturnOnHour>=0&&BLEDturnOnHour<=23){ EEPROM.update(0,BLEDturnOnHour); break;}
       else {BLEDturnOnHour = 23; break; } break;}
    case btnDOWN: { BLEDturnOnHour = BLEDturnOnHour - 1;
       if(BLEDturnOnHour>=0&&BLEDturnOnHour<=23){ EEPROM.update(0,BLEDturnOnHour); break;}
       else {BLEDturnOnHour = 0; break; } break;}
  }
}
//set up Blue LED timer (for turn on minute)
void setBLEDturnOnMin(){
  taskIndex = 2;
  setLEDTX = 2;
  lcd.setCursor(0,0);
  lcd.print("B_LED<TurnOn at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");lcd.print("  ");lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { BLEDturnOnMinute = BLEDturnOnMinute + 5;
       if(BLEDturnOnMinute>=0&&BLEDturnOnMinute<=55){ EEPROM.update(1,BLEDturnOnMinute); break;}
       else {BLEDturnOnMinute = 55; break; } break;}
    case btnDOWN: { BLEDturnOnMinute = BLEDturnOnMinute - 5;
       if(BLEDturnOnMinute>=0&&BLEDturnOnMinute<=55){ EEPROM.update(1,BLEDturnOnMinute); break;}
       else {BLEDturnOnMinute = 0; break; } break;}
  }
}
//set up Blue LED timer (for turn off hour)
void setBLEDturnOffHour(){
  taskIndex = 2;
  setLEDTX = 3;
  lcd.setCursor(0,0);
  lcd.print("B_LED<TurnOf at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");lcd.print("  ");lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { BLEDturnOffHour = BLEDturnOffHour + 1;
       if(BLEDturnOffHour>=0&&BLEDturnOffHour<=23){ EEPROM.update(2,BLEDturnOffHour); break;}
       else {BLEDturnOffHour = 23; break; } break;}
    case btnDOWN: { BLEDturnOffHour = BLEDturnOffHour - 1;
       if(BLEDturnOffHour>=0&&BLEDturnOffHour<=23){ EEPROM.update(2,BLEDturnOffHour); break;}
       else {BLEDturnOffHour = 0; break; } break;}
  }
}
//set up Blue LED time (for turn off minute)
void setBLEDturnOffMin(){
  taskIndex = 2;
  setLEDTX = 4;
  lcd.setCursor(0,0);
  lcd.print("B_LED<TurnOf at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");lcd.print("  ");lcd.print(" ");print2digits(BLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { BLEDturnOffMinute = BLEDturnOffMinute + 5;
       if(BLEDturnOffMinute>=0&&BLEDturnOffMinute<=55){ EEPROM.update(3,BLEDturnOffMinute); break;}
       else {BLEDturnOffMinute = 55; break; } break;}
    case btnDOWN: { BLEDturnOffMinute = BLEDturnOffMinute - 5;
       if(BLEDturnOffMinute>=0&&BLEDturnOffMinute<=55){ EEPROM.update(3,BLEDturnOffMinute); break;}
       else {BLEDturnOffMinute = 0; break; } break;}
  }
}
//set up Blue LED timer (for dimming duration)
void setBLEDdimDur(){
  taskIndex = 2;
  setLEDTX = 5;
  lcd.setCursor(0,0);
  lcd.print("B_LED<dim. dur.>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print(" ");print2digits(BLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(BLEDturnOnHour);lcd.print(":");print2digit0(BLEDturnOnMinute);lcd.print(" ");print2digit0(BLEDturnOffHour);lcd.print(":");print2digit0(BLEDturnOffMinute);lcd.print("   ");
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { BLEDdimDuration = BLEDdimDuration + 5;
       if(BLEDdimDuration>=0&&BLEDdimDuration<=20){ EEPROM.update(4,BLEDdimDuration); break;}
       else {BLEDdimDuration = 20; break; } break;}
    case btnDOWN: { BLEDdimDuration = BLEDdimDuration - 5;
       if(BLEDdimDuration>=0&&BLEDdimDuration<=20){ EEPROM.update(4,BLEDdimDuration); break;}
       else {BLEDdimDuration = 0; break; } break;}
  }
}
//Turn on BLED
void BLEDcycleOn(){
  BLEDdelay = BLEDdimDuration * 60000 / 256;
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("BLED PowerUp");
  for (int f=0; f<=255; f=f+1){
    analogWrite(11, f); //BLED connect to pin 11 via TIP122 transistor
    lcd.setCursor(13,1);lcd.print("   ");
    int power = f * 100 / 255;
    lcd.setCursor(13,1);lcd.print(power);lcd.print("%");
    delay(BLEDdelay);
   }
}
//Turn off BLED
void BLEDcycleOff(){
  BLEDdelay = BLEDdimDuration * 60000 / 256;
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("BLED Dimming");
  for (int f=255; f>=0; f=f-1){
    analogWrite(11, f);
    lcd.setCursor(13,1);lcd.print("   ");
    int power = f * 100 /255;
    lcd.setCursor(13,1);lcd.print(power);lcd.print("%");
    delay(BLEDdelay);
  }
}

//set up Cool LED timer (for return)
void setCLED(){
  taskIndex = 3;
  setLEDTX = 0;
  lcd.setCursor(0,0);
  lcd.print("C_LED  <RETURN> ");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("  ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnSELECT:{ taskIndex = 0; delay(350); break;}
  }
}
//set up Cool LED timer (for turn on hour)
void setCLEDturnOnHour(){
  taskIndex = 3;
  setLEDTX = 1;
  lcd.setCursor(0,0);
  lcd.print("C_LED<TurnOn at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print("  ");lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { CLEDturnOnHour = CLEDturnOnHour + 1;
       if(CLEDturnOnHour>=0&&CLEDturnOnHour<=23){ EEPROM.update(5,CLEDturnOnHour); break;}
       else {CLEDturnOnHour = 23; break; } break;}
    case btnDOWN: { CLEDturnOnHour = CLEDturnOnHour - 1;
       if(CLEDturnOnHour>=0&&CLEDturnOnHour<=23){ EEPROM.update(5,CLEDturnOnHour); break;}
       else {CLEDturnOnHour = 0; break; } break;}
  }
}
//set up Cool LED timer (for turn on minute)
void setCLEDturnOnMin(){
  taskIndex = 3;
  setLEDTX = 2;
  lcd.setCursor(0,0);
  lcd.print("C_LED<TurnOn at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");lcd.print("  ");lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { CLEDturnOnMinute = CLEDturnOnMinute + 5;
       if(CLEDturnOnMinute>=0&&CLEDturnOnMinute<=55){ EEPROM.update(6,CLEDturnOnMinute); break;}
       else {CLEDturnOnMinute = 55; break; } break;}
    case btnDOWN: { CLEDturnOnMinute = CLEDturnOnMinute - 5;
       if(CLEDturnOnMinute>=0&&CLEDturnOnMinute<=55){ EEPROM.update(6,CLEDturnOnMinute); break;}
       else {CLEDturnOnMinute = 0; break; } break;}
  }
}
//set up Cool LED timer (for turn off hour)
void setCLEDturnOffHour(){
  taskIndex = 3;
  setLEDTX = 3;
  lcd.setCursor(0,0);
  lcd.print("C_LED<TurnOf at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");lcd.print("  ");lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { CLEDturnOffHour = CLEDturnOffHour + 1;
       if(CLEDturnOffHour>=0&&CLEDturnOffHour<=23){ EEPROM.update(7,CLEDturnOffHour); break;}
       else {CLEDturnOffHour = 23; break; } break;}
    case btnDOWN: { CLEDturnOffHour = CLEDturnOffHour - 1;
       if(CLEDturnOffHour>=0&&CLEDturnOffHour<=23){ EEPROM.update(7,CLEDturnOffHour); break;}
       else {CLEDturnOffHour = 0; break; } break;}
  }
}
//set up Cool LED time (for turn off minute)
void setCLEDturnOffMin(){
  taskIndex = 3;
  setLEDTX = 4;
  lcd.setCursor(0,0);
  lcd.print("C_LED<TurnOf at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");lcd.print("  ");lcd.print(" ");print2digits(CLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { CLEDturnOffMinute = CLEDturnOffMinute + 5;
       if(CLEDturnOffMinute>=0&&CLEDturnOffMinute<=55){ EEPROM.update(8,CLEDturnOffMinute); break;}
       else {CLEDturnOffMinute = 55; break; } break;}
    case btnDOWN: { CLEDturnOffMinute = CLEDturnOffMinute - 5;
       if(CLEDturnOffMinute>=0&&CLEDturnOffMinute<=55){ EEPROM.update(8,CLEDturnOffMinute); break;}
       else {CLEDturnOffMinute = 0; break; } break;}
  }
}
//set up Cool LED timer (for dimming duration)
void setCLEDdimDur(){
  taskIndex = 3;
  setLEDTX = 5;
  lcd.setCursor(0,0);
  lcd.print("C_LED<dim. dur.>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print(" ");print2digits(CLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(CLEDturnOnHour);lcd.print(":");print2digit0(CLEDturnOnMinute);lcd.print(" ");print2digit0(CLEDturnOffHour);lcd.print(":");print2digit0(CLEDturnOffMinute);lcd.print("   ");
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { CLEDdimDuration = CLEDdimDuration + 5;
       if(CLEDdimDuration>=0&&CLEDdimDuration<=20){ EEPROM.update(9,CLEDdimDuration); break;}
       else {CLEDdimDuration = 20; break; } break;}
    case btnDOWN: { CLEDdimDuration = CLEDdimDuration - 5;
       if(CLEDdimDuration>=0&&CLEDdimDuration<=20){ EEPROM.update(9,CLEDdimDuration); break;}
       else {CLEDdimDuration = 0; break; } break;}
  }
}
//Turn on CLED
void CLEDcycleOn(){
  CLEDdelay = CLEDdimDuration * 60000 / 256;
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("CLED PowerUp");
  for (int f=0; f<=255; f=f+1){
    analogWrite(12, f); //BLED connect to pin 11 via TIP122 transistor
    lcd.setCursor(13,1);lcd.print("   ");
    int power = f * 100 / 255;
    lcd.setCursor(13,1);lcd.print(power);lcd.print("%");
    delay(CLEDdelay);
   }
}
//Turn off CLED
void CLEDcycleOff(){
  CLEDdelay = CLEDdimDuration * 60000 / 256;
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("CLED Dimming");
  for (int f=255; f>=0; f=f-1){
    analogWrite(12, f);
    lcd.setCursor(13,1);lcd.print("   ");
    int power = f * 100 /255;
    lcd.setCursor(13,1);lcd.print(power);lcd.print("%");
    delay(CLEDdelay);
  }
}

//set up Warm LED timer (for return)
void setWLED(){
  taskIndex = 4;
  setLEDTX = 0;
  lcd.setCursor(0,0);
  lcd.print("W_LED  <RETURN> ");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("  ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnSELECT:{ taskIndex = 0; delay(350); break;}
  }
}
//set up Warm LED timer (for turn on hour)
void setWLEDturnOnHour(){
  taskIndex = 4;
  setLEDTX = 1;
  lcd.setCursor(0,0);
  lcd.print("W_LED<TurnOn at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");lcd.print("  ");lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { WLEDturnOnHour = WLEDturnOnHour + 1;
       if(WLEDturnOnHour>=0&&WLEDturnOnHour<=23){ EEPROM.update(10,WLEDturnOnHour); break;}
       else {WLEDturnOnHour = 23; break; } break;}
    case btnDOWN: { WLEDturnOnHour = WLEDturnOnHour - 1;
       if(WLEDturnOnHour>=0&&WLEDturnOnHour<=23){ EEPROM.update(10,WLEDturnOnHour); break;}
       else {WLEDturnOnHour = 0; break; } break;}
  }
}
//set up Warm LED timer (for turn on minute)
void setWLEDturnOnMin(){
  taskIndex = 4;
  setLEDTX = 2;
  lcd.setCursor(0,0);
  lcd.print("W_LED<TurnOn at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");lcd.print("  ");lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { WLEDturnOnMinute = WLEDturnOnMinute + 5;
       if(WLEDturnOnMinute>=0&&WLEDturnOnMinute<=55){ EEPROM.update(11,WLEDturnOnMinute); break;}
       else {WLEDturnOnMinute = 55; break; } break;}
    case btnDOWN: { WLEDturnOnMinute = WLEDturnOnMinute - 5;
       if(WLEDturnOnMinute>=0&&WLEDturnOnMinute<=55){ EEPROM.update(11,WLEDturnOnMinute); break;}
       else {WLEDturnOnMinute = 0; break; } break;}
  }
}
//set up Warm LED timer (for turn off hour)
void setWLEDturnOffHour(){
  taskIndex = 4;
  setLEDTX = 3;
  lcd.setCursor(0,0);
  lcd.print("W_LED<TurnOf at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");lcd.print("  ");lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { WLEDturnOffHour = WLEDturnOffHour + 1;
       if(WLEDturnOffHour>=0&&WLEDturnOffHour<=23){ EEPROM.update(12,WLEDturnOffHour); break;}
       else {WLEDturnOffHour = 23; break; } break;}
    case btnDOWN: { WLEDturnOffHour = WLEDturnOffHour - 1;
       if(WLEDturnOffHour>=0&&WLEDturnOffHour<=23){ EEPROM.update(12,WLEDturnOffHour); break;}
       else {WLEDturnOffHour = 0; break; } break;}
  }
}
//set up Warm LED time (for turn off minute)
void setWLEDturnOffMin(){
  taskIndex = 4;
  setLEDTX = 4;
  lcd.setCursor(0,0);
  lcd.print("W_LED<TurnOf at>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");lcd.print("  ");lcd.print(" ");print2digits(WLEDdimDuration);
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { WLEDturnOffMinute = WLEDturnOffMinute + 5;
       if(WLEDturnOffMinute>=0&&WLEDturnOffMinute<=55){ EEPROM.update(13,WLEDturnOffMinute); break;}
       else {WLEDturnOffMinute = 55; break; } break;}
    case btnDOWN: { WLEDturnOffMinute = WLEDturnOffMinute - 5;
       if(WLEDturnOffMinute>=0&&WLEDturnOffMinute<=55){ EEPROM.update(13,WLEDturnOffMinute); break;}
       else {WLEDturnOffMinute = 0; break; } break;}
  }
}
//set up Warm LED timer (for dimming duration)
void setWLEDdimDur(){
  taskIndex = 4;
  setLEDTX = 5;
  lcd.setCursor(0,0);
  lcd.print("W_LED<dim. dur.>");
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print(" ");print2digits(WLEDdimDuration);
  delay(125);
  lcd.setCursor(0,1);
  lcd.print("R ");print2digit0(WLEDturnOnHour);lcd.print(":");print2digit0(WLEDturnOnMinute);lcd.print(" ");print2digit0(WLEDturnOffHour);lcd.print(":");print2digit0(WLEDturnOffMinute);lcd.print("   ");
  delay(25);
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnRIGHT:{setLEDTX = setLEDTX + 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 5; break;} break;}
    case btnLEFT:{setLEDTX = setLEDTX - 1; 
      if(setLEDTX >= 0 && setLEDTX<=5){ loop(); delay(200); break;}
      else {setLEDTX  = 0; break;} break;}
    case btnUP: { WLEDdimDuration = WLEDdimDuration + 5;
       if(WLEDdimDuration>=0&&WLEDdimDuration<=20){ EEPROM.update(14,WLEDdimDuration); break;}
       else {WLEDdimDuration = 20; break; } break;}
    case btnDOWN: { WLEDdimDuration = WLEDdimDuration - 5;
       if(WLEDdimDuration>=0&&WLEDdimDuration<=20){ EEPROM.update(14,WLEDdimDuration); break;}
       else {WLEDdimDuration = 0; break; } break;}
  }
}
//Turn on WLED
void WLEDcycleOn(){
  WLEDdelay = WLEDdimDuration * 60000 / 256;
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("WLED PowerUp");
  for (int f=0; f<=255; f=f+1){
    analogWrite(13, f); //WLED connect to pin 11 via TIP122 transistor
    lcd.setCursor(13,1);lcd.print("   ");
    int power = f * 100 / 255;
    lcd.setCursor(13,1);lcd.print(power);lcd.print("%");
    delay(WLEDdelay);
   }
}
//Turn off WLED
void WLEDcycleOff(){
  WLEDdelay = WLEDdimDuration * 60000 / 256;
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("WLED Dimming");
  for (int f=255; f>=0; f=f-1){
    analogWrite(13, f);
    lcd.setCursor(13,1);lcd.print("   ");
    int power = f * 100 /255;
    lcd.setCursor(13,1);lcd.print(power);lcd.print("%");
    delay(WLEDdelay);
  }
}

void setH2OTemp(){
  taskIndex = 5;
}

void setH2OLevel(){
  taskIndex = 6;
}
//display date and time on LCD (use LEFT button to go back main menu, use UP and DOWN to adjust LCD backlight brightness)
void dispDatetime(){
  taskIndex = 7;
  lcd.setCursor(0,0);
  lcd.print(year());lcd.print(" ");lcd.print(monthShortStr(month()));lcd.print(" ");print2digits(day());lcd.print(" ");lcd.print(dayShortStr(weekday()));lcd.print(".");
  lcd.setCursor(0,1);
  lcd.print("    ");print2digit0(hour());lcd.print(":");print2digit0(minute());lcd.print(":");print2digit0(second());lcd.print("    ");
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnDOWN:{ backlightValue = backlightValue - 32; 
      if (backlightValue<=255&&backlightValue>=0){analogWrite(10, backlightValue); delay(200);}
      else {backlightValue=0; analogWrite(10, backlightValue);delay(100);}
      break;}
    case btnUP:{ backlightValue = backlightValue + 32; 
      if (backlightValue<=255&&backlightValue>=0){analogWrite(10, backlightValue); delay(200);}
      else {backlightValue=255; analogWrite(10, backlightValue);delay(100);}
      break;}
    case btnLEFT:{ taskIndex = 0; delay(300); break;}//LEFT button to go back main menu
  }
}

void dispTempLevel(){
  taskIndex = 8;
}

void dispAll(){
  taskIndex = 9;
}

//define a function to handle changes between 1-digit and 2-digit numbers (space-style)
void print2digits(int number) {
  // Output leading zero
  if (number < 10) {
    lcd.write(" ");
  }
  lcd.print(number);
}
//define a function to handle changes between 1-digit and 2-digit numbers (0-style) 
void print2digit0(int number) {
  // Output leading zero
  if (number < 10) {
    lcd.write("0");
  }
  lcd.print(number);
}

//LCD backlight control
void adjustBacklight(){
  lcd_key = read_LCD_buttons();
  switch (lcd_key) {
    case btnDOWN:{ backlightValue = backlightValue - 32; 
      if (backlightValue<=255&&backlightValue>=0){analogWrite(10, backlightValue); delay(200);}
      else {backlightValue=0; analogWrite(10, backlightValue);delay(100);}
      break;}
    case btnUP:{ backlightValue = backlightValue + 32; 
      if (backlightValue<=255&&backlightValue>=0){analogWrite(10, backlightValue); delay(200);}
      else {backlightValue=255; analogWrite(10, backlightValue);delay(100);}
      break;}
  }
}

