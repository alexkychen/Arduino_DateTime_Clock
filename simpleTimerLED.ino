/*
Use Arduino to set up a control timer for an LED;
Arbitrarily set light on and off time; 
The LED fades using PWM;
*/
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h> //https://github.com/JChristensen/DS3232RTC
LiquidCrystal_I2C lcd(0x3F,16,2);

int dawnHr = 1;
int dawnMin = 14;
int duskHr = 1;
int duskMin = 15;

void setup() {
  //pinMode(11, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(4,0);
  lcd.print("Welcome!");
  lcd.setCursor(1,1);
  lcd.print("Arduino Clock");
  delay(2000);
  lcd.clear();
  delay(250);
  
  //setTime(20,27,20,21,2,2016);
  //RTC.set(now());
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  Alarm.alarmRepeat(dawnHr,dawnMin,0,LightUp);
  Alarm.alarmRepeat(duskHr,duskMin,0,LightOff);
}

void loop() {
  showTime();
  Alarm.delay(0);
}

void showTime(){
  lcd.setCursor(0,0);
  lcd.print("TIME: ");
  print2digits(hour());
  lcd.print(':');
  print2digits(minute());
  lcd.print(':');
  print2digits(second());
  lcd.setCursor(0,1);
  lcd.print("DATE:");
  lcd.print(year());
  lcd.print("/");
  lcd.print(monthShortStr(month()));
  lcd.print("/");
  print2digits(day());
}

//define a function to handle changes between 1-digit and 2-digit numbers
void print2digits(int number) {
  // Output leading zero
  if (number < 10) {
    lcd.write('0');
  }
  lcd.print(number);
}

//define a function to 
void LightUp() {
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("Dawning...");
  for (int f=0; f<=255; f=f+1){
    analogWrite(11,f);
    lcd.setCursor(11,0);
    lcd.print("    ");
    int power = f * 100 / 255;
    lcd.setCursor(11,0);
    lcd.print(power);
    lcd.print("%");
    delay(100);
  }
  lcd.clear();
}

void LightOff(){
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("Dusking...");
  for (int f=255; f>=0; f=f-1){
    analogWrite(11,f);
    lcd.setCursor(11,0);
    lcd.print("    ");
    int power = f * 100 / 255;
    lcd.setCursor(11,0);
    lcd.print(power);
    lcd.print("%");
    delay(100);
  }
  lcd.clear();
}

