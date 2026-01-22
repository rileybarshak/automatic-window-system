#include <LiquidCrystal.h>  // includes the LiquidCrystal Library
#include <dht.h>
#include <AbleButtons.h>
#include <Servo.h>
#include "NewPing.h"

#define dataPin 8
#define lowerbutton 10
#define raisebutton 9
#define servopin 6
#define triggerpin 13
#define echopin 7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
dht DHT;
unsigned long lastUpdate;
int setTemp = 70;
Servo spinnyboi;
int pos = 0;
bool windowOpen;

using Button = AblePullupClickerButton;
Button lower(lowerbutton);
Button raise(raisebutton);

NewPing sonar(triggerpin, echopin, 20);

void setup() {
  lcd.begin(16, 2);  // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display
  lower.begin();
  raise.begin();
  pinMode(lowerbutton, INPUT);
  pinMode(raisebutton, INPUT);
  spinnyboi.attach(servopin);
  spinnyboi.write(90);
  windowOpen = false;
}

void loop() {
  lower.handle();
  raise.handle();
  unsigned long current = millis();

  if(lower.resetClicked()){
    setTemp--;
    display(setTemp);
    lastUpdate = current;
  }
  if(raise.resetClicked()){
    setTemp++;
    display(setTemp);
    lastUpdate = current;
  }

  int readData = DHT.read11(dataPin);
  float t = DHT.temperature;
  int temp = round((t * 9.0) / 5.0 + 32.0);
  if(temp < setTemp-1){
    moveWindow(20.0);
  }
  if(temp > setTemp+1){
    moveWindow(10.0);
  }

  if(lastUpdate + 5000 <= current){
    display(setTemp);
    lastUpdate = current;
  }
}

void display(int s) {
  int readData = DHT.read11(dataPin);
  float t = DHT.temperature;
  float h = DHT.humidity;
  lcd.setCursor(0, 1);   // Sets the location at which subsequent text written to the LCD will be displayed
  lcd.print("Temp:");  // Prints string "Temp." on the LCD

  lcd.print(round((t * 9.0) / 5.0 + 32.0));  // print the temperature in Fahrenheit
  lcd.print((char)223);  //shows degrees character
  lcd.print("F");

  lcd.setCursor(0, 0);
  lcd.print("Set:");
  lcd.print(s);
  lcd.print((char)223);  //shows degrees character
  lcd.print("F");
  
  lcd.setCursor(11, 0);
  lcd.print("Humi:");
  lcd.setCursor(12, 1);
  lcd.print(round(h));
  lcd.print("%");
}

void moveWindow(float x){
  //Move window to 'x' centimeters
  while((sonar.ping_cm()-0.5) > x || x > (sonar.ping_cm()+0.5)){
    if(x < sonar.ping_cm()){
      spinnyboi.write(180);
    }
    if(x < sonar.ping_cm()){
      spinnyboi.write(0);
    }
  }
}