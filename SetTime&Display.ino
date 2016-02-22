/*
Use Arduino Uno to connect DS3231 RTC and 1602 I2C LCD display
to display time and date on the LCD display
Install the following libraries <xxx.h> if they are not installed.
*/
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h> //https://github.com/JChristensen/DS3232RTC

LiquidCrystal_I2C lcd(0x3F,16,2); //set LCD I2C address and tell it's a 16 by 2 LCD display 
//To get I2C address, follow http://playground.arduino.cc/Main/I2cScanner

void setup() {
  lcd.init();               //initiate LCD display
  lcd.backlight();          //turn on LCD backlight
  lcd.setCursor(4,0);       //set cursor to 4th character and top row of the display
  lcd.print("Welcome!");    //print "Welcome!" to the display
  lcd.setCursor(2,1);        //set another cursor to 2th character and bottom row of the display
  lcd.print("Arduino Clock"); //print "Arduino Clock" to the display
  delay(2000);                //hold for 2 seconds
  lcd.clear();                //clear the display
  delay(250);              
  
  //Uncomment 1. & 2. when use RTC at the first time; comment 1. & 2. after time is set up.
  //setTime(20,27,20,21,2,2016); //1. manually set the current time to system in (hour,min,sec,day,month,year)
  //RTC.set(now());             //2. Set time to RTC from the system 
  setSyncProvider(RTC.get);   //3. the function to get the time from the RTC
}

void loop() {
  //start printing time and date on the display, follow http://playground.arduino.cc/Code/Time to call time/date
  lcd.setCursor(0,0);
  lcd.print("TIME: ");
  lcd.print(hour());
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
