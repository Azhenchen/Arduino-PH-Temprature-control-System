
#define SensorPin A2            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 1000
#define ArrayLenth  40    //times of collection
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;
int relay = 0; //继电器导通触发信号-高电平有效；    
int redpin = 11; //select the pin for the red LED
int bluepin =10; // select the pin for the blue LED
int greenpin =9;// select the pin for the green LED
int val;
int PHtimes=0;
int TMPtimes=0;
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
OneWire  ds(5);  // 连接arduino10引脚
void setup(void)
{
  lcd.init();
  pinMode(redpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(LED,OUTPUT);  
  pinMode(relay,OUTPUT); //定义端口属性为输出； 
}
void loop()
{ 
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  //PH
  
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltage+Offset;
  
  //Temp
  if ( !ds.search(addr)) {
    ds.reset_search();
    return;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes 3.9A
    data[i] = ds.read();
    }
  // convert the data to actual temperature
  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
   //LED
  if(celsius < 45 && celsius > 25)
  {
    analogWrite(11, 0);
    analogWrite(10, 255);
    analogWrite(9, 0);
     digitalWrite(relay,HIGH); //继电器导通；
    // delay(1000);
  }
  else if(celsius <= 25)
  {
    analogWrite(11, 255);
    analogWrite(10, 0);
    analogWrite(9, 0);
     digitalWrite(relay,HIGH); //继电器导通；
     delay(1000);
  }
  else
  {
    analogWrite(11, 0);
    analogWrite(10, 0);
    analogWrite(9, 255);
    digitalWrite(relay,LOW); //继电器开关断开；
    delay(1000);
  }
  lcd.backlight();
  if(PHtimes==TMPtimes){
    lcd.print("PH value: ");
    lcd.print(pHValue,2);
    lcd.print("    "); 
    delay(1000);
    lcd.clear(); 
    PHtimes++;
    }
   else{
    lcd.print("Temp: ");
    lcd.print(celsius,2);
    lcd.print("       ");
    delay(1000);
    lcd.clear();
    TMPtimes++;}
}

double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }
    }
    avg = (double)amount/(number-2);
  }
  return avg;
}
