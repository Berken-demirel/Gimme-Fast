#include <EEPROM.h>
#define HardwareSH
#include "C:\\Users\\karab\\OneDrive\\Emre\\Designs\\Visible Light Com\\Arduino\\Comm_Protocol\\ARQ.h"
#include "myimage.h"
#include "pid.h"

SoftwareSerial swSer;
uint8_t my_buffer[15000];
uint16_t try_connect_number=0;
uint8_t user_index=0;

#define connect_timeout 100
#define send_timeout 200
#define read_timeout 5000
#define try_number 50

ARQ Master_Channel1(256,&Serial1);

ARQ Master_Channel2(32,&Serial2);

void send_image()
{
  try_connect_number=0;
  while(try_connect_number<try_number)
  {
    if(!Master_Channel2.IsConnected())
    { 
      Serial.println("Connecting");
      if(Master_Channel2.TryConnect(connect_timeout))
      {
        try_connect_number=0;
        Serial.println("Connected");
      }
      else
        Serial.println(String("Error:")+String(try_connect_number++));
    }
    else
    {
   //   delay(100);
      Serial.println("Requesting Send");
       long my_temp=millis();
      if(Master_Channel2.RequestSendPackage(7680,send_timeout,user_index))
      {     
        Serial.println("Sending...");
        Master_Channel2.SendBytes(my_buffer,7680,send_timeout,5);
        //Master_Channel2.SendBytes(myimage+7680*user_index,7680,send_timeout,try_number);
        Serial.print("Sent 7680 bytes in:");   
        Serial.print((millis()-my_temp)/1000.0);
        Serial.println(" seconds"); 
         
         if(Master_Channel2.DisConnect(send_timeout,false))
         {
            Serial.println("\nDisconnected");
            //user_index++;
            //if(user_index>4)
              //user_index=0;
            return;
       //     delay(2000);
         }
      }
      else
      {
        try_connect_number++;
        if(try_connect_number>4)
        {
          Master_Channel2.DisConnect(500,true);
          Serial.println("\nConnection Closed");  
        //  user_index++;
          //if(user_index>4)
            // user_index=0;
          return;
        }
        else      
          Serial.println("\nSend timeout");  
      }
     // delay(1000);
      Serial.println();
    }
  }
}

void get_image()
{
  try_connect_number=0;
  while(try_connect_number<try_number)
  {
    if(!Master_Channel1.IsConnected())
    { 
      Serial.println("Connecting");
      if(Master_Channel1.TryConnect(connect_timeout))
      {
        try_connect_number=0;
        Serial.println("Connected");
      }
      else
        Serial.println(String("Error:")+String(try_connect_number++));
    }
    else
    {
      //delay(1000);
      //Serial.println("Requesting Send");
      Serial.println("Waiting package");
      bool stat=Master_Channel1.RequestPackage(read_timeout,user_index);//user_index
      //Serial.println(stat);
      if(stat)
      {
        long my_temp=millis();
        uint16_t len=Master_Channel1.BeginReceive(my_buffer,read_timeout,try_number);
        if(len>0)
        {
          Serial.print("received:");
          Serial.print(len);
          Serial.print(" bytes in:");            
          Serial.print((millis()-my_temp)/1000.0);
          Serial.println(" seconds");        
         
         if(Master_Channel1.DisConnect(send_timeout,false))
         {
            Serial.println("\nDisconnected");
            return;
            //delay(2000);
         }
        }else
        {
          Serial.print("Error len:");
          Serial.print(len); 
          while(1);
          //delay(1000);
        }     
      }
      else
      {
        try_connect_number++;
        if(try_connect_number>4)
        {
          Master_Channel1.DisConnect(500,true);
          Serial.println("\nConnection Closed");  
          return;
        }
        else      
        {
          Serial.println("\nRead timeout");  
          Master_Channel1.DisConnect(50,true);
          //while(1);
        }
      }
      //delay(1000);
      Serial.println();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(19200,SERIAL_8N1, -1, -1, true, 100UL);
  Serial1.begin(19200,SERIAL_8N1, 32, 21, true, 100UL);
 
  pinMatrixOutDetach(16, false, false);
  pinMode(16, INPUT);
  pinMatrixInAttach(16, UART_RXD_IDX(2), false);
  pinMode(5,OUTPUT);

  pinMatrixOutDetach(32, false, false);
  pinMode(32, INPUT);
  pinMatrixInAttach(32, UART_RXD_IDX(1), false);
  
  for(long i=0;i<7680;i++)
    my_buffer[i]=0xFF;

  pid_init();
  delay(1000);
  esp_task_wdt_deinit();

  EEPROM.begin(3);
  
  if(EEPROM.read(0)!=0x60||digitalRead(SW1)){   
     EEPROM.write(0, 0x60);
     EEPROM.write(1, 0);
     EEPROM.write(2, 0);
     EEPROM.commit();
  }
  else
  {
    user_index=EEPROM.read(1);
  }
}

void loop() {
//  Motor_Test(digitalRead(SW1));
  while(digitalRead(SW2))
    Pos_Sens_Test();
  while(digitalRead(SW4))
  {    
    for(int i=0;i<10;i++)
      Serial2.write(0xAA);
    for(int i=0;i<10;i++)
      Serial2.write(0xEE);
    //Serial2.println("Emre KARABAKLA, Naber Selam Deneme Test Emre KARABAKLA, Naber Selam Deneme Test");
    //for(int i=0;i<512;i++)
   // Serial2.write(0xAA);
    delay(500);    
    
  }
  while(digitalRead(SW3))
  {
    while(Serial2.available())  
    {
        Serial.print(Serial2.read(),HEX);
    }
  }
  //user_index=4;
  long temp=millis();
  
  for(int i=user_index;i<5;i++)
  {
 //   user_index=4;
    Serial.print("Cycle:");
    Serial.println(user_index);
    gotoTerm(1); 
    gotoTerm(0);
    vTaskDelay(100 / 15);
    //ESP.restart();
    get_image();
    vTaskDelay(100 / 15);
    
    gotoTerm(2); 
    gotoTerm(0);
    vTaskDelay(100 / 15);
    send_image();
    vTaskDelay(100 / 15);

    user_index++;
    EEPROM.write(0, 0x60);
    EEPROM.write(1, user_index);
    EEPROM.commit();
  }
  temp=millis()-temp;
  EEPROM.write(0, 0x60);
  EEPROM.write(1, 0);
  EEPROM.commit();
  while(1)
  {
    Serial.print("Total Time:");
    Serial.println(temp/1000.0);
    esp_task_wdt_reset();
    delay(100);
  }
  /*  while(Serial1.available())
    {
      Serial.print((char)Serial1.read());
    }*/
  /*while(Serial.available())
  {
    if(Serial.readStringUntil('\n')=="e")
      Serial2.write(my_buffer,100);
  }*/

 /* delay(2000);
  //get_image();
  //delay(100);
  //for(int i=0;i<100;i++)
    //Serial.println(mySensors.getLeftDistance());
  for(int i=0;i<5;i++)
  { 
    Serial.println(mySensors.getRelativePos());
    Serial.println("Going to Term 1");
    gotoTerm(1); 
    gotoTerm(0);   
    delay(1000);
    get_image();
    delay(1000);
    Serial.println("Going to Term 2");
    gotoTerm(2); 
    gotoTerm(0);
    delay(1000);
    send_image();
  }
      // delay(2000);
*/
}
