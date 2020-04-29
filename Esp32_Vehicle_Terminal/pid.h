/*#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;*/

#include "SharpSens.h"

#define Pos2_pin A14
#define Pos1_pin A10
#define MVA_len 500

SharpSens mySensors(Pos1_pin,Pos2_pin,MVA_len);

#define Mot_in1 18
#define Mot_in2 19
#define Mot_en  5

#define SW4 33
#define SW3 25 
#define SW2 26
#define SW1 27

const double kp = 100; //130
const double ki = 0;
const double kd = 15; //45
double last_error=0,i_error=0;
long long prev=0;
double dt=0;

void pid_init()
{
  pinMode(Mot_in1,OUTPUT);
  pinMode(Mot_in2,OUTPUT);

  pinMode(SW1,INPUT);
  pinMode(SW2,INPUT);
  pinMode(SW3,INPUT);
  pinMode(SW4,INPUT);
  
  ledcSetup(0, 5000, 10);
  ledcAttachPin(5, 0);
  ledcWrite(0,0);

  //SerialBT.begin("ESP32_Vehicle"); //Bluetooth device name
}

double pid(double target_value, double current_value) {
  double pidTerm;
  double error;
  dt=(micros()-prev)/1000000.0;
  error = target_value - current_value;
  i_error += error;
  if(i_error>1023)
    i_error=1023;
  pidTerm = (error)* kp + (error - last_error) * kd/dt + i_error* ki*dt;
  last_error = error;
  return pidTerm;
}

void gotoPos(double pos)
{   
    double _pid=pid(pos,mySensors.getRelativePos());
    if(_pid>0)
    {
      digitalWrite(Mot_in1,0);
      digitalWrite(Mot_in2,1);  
    }
    else
    {
      digitalWrite(Mot_in1,1);
      digitalWrite(Mot_in2,0);
    }
    //Serial.print(_pid);
  //  Serial.print("   Pos: ");
   // Serial.println(mySensors.getRelativePos());
    _pid=abs(_pid);
    if(_pid>1024)
      _pid=1024;
   // Serial.println(_pid);
    ledcWrite(0,_pid);
    delayMicroseconds(1);
}
bool checkError(double ref,double input,double error)
{
  return abs(input-ref)<error;
}
bool gotoTerm(int term)
{
  if(term==1)  //camera
  {
    for(int i=0;i<50;i++)
    {
      while(!checkError(47,mySensors.getRelativePos(),2))
      {
        esp_task_wdt_reset();
        gotoPos(47);
       // Serial.print("goint to term 2: ");
       // Serial.println(mySensors.getRelativePos());
      }
      delayMicroseconds(50);
    }
  }
  else if(term==2)  // screen
  {
    for(int i=0;i<50;i++)
    {
      while(!checkError(-47,mySensors.getRelativePos(),2))
      {
        esp_task_wdt_reset();
        gotoPos(-47);
        //Serial.print("goint to term 2: ");
       //Serial.println(mySensors.getRelativePos());
      }
      delayMicroseconds(50);
    }
    
  }else
  {
    ledcWrite(0,0);
    digitalWrite(Mot_in1,0);
    digitalWrite(Mot_in2,0);
  }
  return false;
}

void Pos_Sens_Test()
{
  //SerialBT.print("Pos: ");
 // SerialBT.println(mySensors.getRelativePos());
  Serial.print("Pos: ");
  Serial.println(mySensors.getRelativePos());
}

void Motor_Test(bool stat)
{
  if(stat)
  { 
    delay(2000);
    long prev=millis();
    gotoTerm(2);    
    Serial.print("Elapsed Time:");
    Serial.println((millis()-prev)/1000.0);
    Serial.print("Set=48 Distance: ");
    Serial.println(mySensors.getRelativePos());
    gotoTerm(0);
    delay(3000);
    prev=millis();
    gotoTerm(1);
    Serial.print("Elapsed Time:");
    Serial.println((millis()-prev)/1000.0);
    Serial.print("Set=-48 Distance: ");
    Serial.println(mySensors.getRelativePos());
    gotoTerm(0);
    delay(3000);
  }
  else
  {
    gotoTerm(0);
  }
}
