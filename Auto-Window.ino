#include <LiquidCrystal.h>  // includes the LiquidCrystal Library
#include <dht.h>
#include <AbleButtons.h>
#include <Servo.h>
#include <cppQueue.h>

#define dataPin 40
#define lowerbutton 32
#define raisebutton 33
#define servopin 2
#define triggerpin 3
#define echopin 50
LiquidCrystal lcd(22, 23, 24, 25, 26, 27);  // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
dht DHT;
unsigned long lastUpdate;
int setTemp = 70;
Servo spinnyboi;
int windowPos = 20; //closed position # in cm
float timing = 0.0;
float distance = 0.0;
cppQueue disQueue(sizeof(float), 16, FIFO);
double runningSum = 0.0;

using Button = AblePullupClickerButton;
Button lower(lowerbutton);
Button raise(raisebutton);


void setup() {
  lcd.begin(16, 2);  // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display
  lower.begin();
  raise.begin();
  pinMode(lowerbutton, INPUT);
  pinMode(raisebutton, INPUT);
  spinnyboi.attach(servopin);
  spinnyboi.write(90);
  pinMode(echopin, INPUT);
  pinMode(triggerpin, OUTPUT);
  digitalWrite(triggerpin, LOW);

  Serial.begin(9600);
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
    windowPos = 10;
  }
  if(temp > setTemp+1){
    windowPos = 20;
  }

  digitalWrite(triggerpin, LOW);
  delay(2);
  
  digitalWrite(triggerpin, HIGH);
  delay(10);
  digitalWrite(triggerpin, LOW);
  
  timing = pulseIn(echopin, HIGH);
  distance = (timing * 0.034) / 2;
  bool pushed = disQueue.push(&distance);

  float avgDis;
  if (!pushed) {
    double sum = 0.0;
    float v = 0.0;

    uint16_t cnt = disQueue.getCount(); // number of records stored :contentReference[oaicite:9]{index=9}
    for (uint16_t i = 0; i < cnt; i++) {
      disQueue.peekIdx(&v, i);          // peek at index i :contentReference[oaicite:10]{index=10}
      sum += v;
    }

    avgDis = (cnt > 0) ? (float)(sum / cnt) : 0.0f;

    // Now remove oldest to make room, then push again:
    float dropped = 0.0f;
    disQueue.pop(&dropped);             // pop copies oldest into dropped :contentReference[oaicite:11]{index=11}
    disQueue.push(&distance);
  }
  
  
  if(avgDis -1 < windowPos && windowPos < avgDis +1){
    spinnyboi.write(90);
  } else if (windowPos < avgDis + 1) {
  	spinnyboi.write(180);
  } else if(windowPos > avgDis - 1){
  	spinnyboi.write(0);
  }


  /*if(lastUpdate + 1000 <= current){
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print("cm | ");
    Serial.print("WindowPos:");
    Serial.println(windowPos);
    lastUpdate = current;
  }
  */
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