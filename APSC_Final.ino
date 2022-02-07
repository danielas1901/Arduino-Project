#include <SoftwareSerial.h>
#define LED 4//indicator, Grove - LED is connected with D4 of Arduino
#include <Wire.h> //needed library
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//////////////////////////////////////////////////////////////////////////////////////////////////// TEMPERATURE
int body_tempF;
int body_tempC;
int criticalT = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////// BLUETOOTH
SoftwareSerial BTserial(5, 6); // TX | RX

int sensorPin = A0;
int sensorValue = 0;
int statepin = 4;
//////////////////////////////////////////////////////////////////////////////////////////////////// LCD SCREEN
int rs=7;
int en = 8;
int d4 = 9;
int d5 = 10;
int d6 = 11;
int d7 = 12;
int temp_pin = A5;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);
//////////////////////////////////////////////////////////////////////////////////////////////////// HEART RATE
unsigned char counter;
unsigned long temp[6];
unsigned long sub;
bool data_effect=true;
unsigned int BPM;//the measurement result of heart rate
const int max_heartpluse_duty = 2000;//after 2 seconds of nothing it senses an error
int measuringBPM = 0;
int criticalH = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////// OTHER
int butpin = 3;
int count = 0;

int working = 1;
char CritT;
char CritH;

void setup() 
{
Serial.begin(9600);
////////////////////////for LCD
lcd.begin(16,2);
////////////////////////for heart rate 
for(unsigned char i=0;i < 5;i ++)
  {
    temp[i]=0;
  }
temp[5]=millis();
////////////////////////for temp sensor
mlx.begin();
////////////////////////for bluetooth
BTserial.begin(9600);
pinMode(statepin, INPUT);
//////////////////////// OTHER
pinMode(butpin, INPUT);
attachInterrupt(1, change, FALLING);

while (digitalRead(statepin)==LOW)
{
  lcd.setCursor(0,0);
  lcd.print("Starting up....");
  lcd.setCursor(0,1);
  lcd.print("Connect Phone");
  delay(1000);
  lcd.clear();
}

lcd.setCursor(0,0);
lcd.print("Successful");
lcd.setCursor(0,1);
lcd.print("Connection.");
delay(2000);
lcd.clear();
working = 1;

}

void loop() {

if (working==0)
{
  while (digitalRead(statepin)==LOW)
  {
    lcd.setCursor(0,0);
    lcd.print("Please Reconnect");
    lcd.setCursor(0,1);
    lcd.print("To The Device");
    delay(1000);
    lcd.clear();
  }
  working = 1;
}

if (working==1)
{
  if (count%2==0)
    {
      if (digitalRead(statepin)==LOW)
      {
        working = 0;
      }
      body_tempC = mlx.readObjectTempC();
      body_tempF = mlx.readObjectTempF();
      count = 0;
      BPM = 0;
      lcd.setCursor(0,0);
      lcd.print("Reading Temp.");
      lcd.setCursor(0,1);
      lcd.print("Button -> BPM");
      BTserial.print("T");
      BTserial.print(",");
      BTserial.print(body_tempC);
      BTserial.print(",");
      BTserial.print(body_tempF);
      BTserial.print(",");
      if (body_tempF < 95 or body_tempF > 107)
        criticalT = 1;
      else
        criticalT = 0;
      if (count%2==0 and criticalT==1)
      {
         BTserial.print("Yes");
         BTserial.print(",");
      }
      else 
      {
        BTserial.print("No");
        BTserial.print(",");
      }
      BTserial.print(BPM);
      BTserial.print(",");
      BTserial.print("N/A");
      BTserial.print(";");
    }
  if (count%2==1)
    {
      if (digitalRead(statepin)==LOW)
      {
        working = 0;
      }
      lcd.setCursor(0,0);
      lcd.print("Reading BPM");
      lcd.setCursor(0,1);
      lcd.print("Button -> Temp.");
      attachInterrupt(0, interrupt, RISING);//set interrupt 0,digital port 2, run interrupt function when it detects that the sensor is being used
    }
delay(1000);
lcd.clear();
}

//function to read button and return 
}

void change()
{
  count++;
}

void interrupt()
{
  measuringBPM = 1;
  temp[counter]=millis();
  Serial.println(counter,DEC);
  Serial.println(temp[counter]);
  switch(counter)
    {
      case 0:
        sub=temp[counter]-temp[5];
        //Serial.println(sub);
        break;
      default:
        sub=temp[counter]-temp[counter-1];
        //Serial.println(sub);
        break;
    }
  if(sub>max_heartpluse_duty)//set 2 seconds as max heart pluse duty
    {
      data_effect=0;//sign bit
      counter=0;
      measuringBPM = 0;
      for(unsigned char i=0;i < 5;i ++)
        {
          temp[i]=0;
        }
      temp[5]=millis();
    }
  if (counter==5&&data_effect)
    {
      counter=0;
      if(data_effect)
        {
          BPM=300000/(temp[5]-temp[0]);//60*5*1000/5_total_time
          BTserial.print("H");
          BTserial.print(",");
          BTserial.print(0);
          BTserial.print(",");
          BTserial.print(0);
          BTserial.print(",");
          BTserial.print("N/A");
          BTserial.print(",");
          BTserial.print(BPM);
          BTserial.print(",");
          if (BPM > 100 or BPM < 50)
            criticalH = 1;
          else
            criticalH = 0;
          if (criticalH==1)
          {
            BTserial.print("Yes");
            BTserial.print(";");
          }
          else 
          {
            BTserial.print("No");
            BTserial.print(";");
          }
        }
      data_effect=1;//sign bit
    }
    else if(counter!=5&&data_effect)
      counter++;
    else 
      {
        counter=0;
        data_effect=1;
      }
 
}
