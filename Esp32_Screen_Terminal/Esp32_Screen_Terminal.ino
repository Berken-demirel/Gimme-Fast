#define HardwareSH
#include "C:\\Users\\karab\\OneDrive\\Emre\\Designs\\Visible Light Com\\Arduino\\Comm_Protocol\\ARQ.h"
#include "myimage.h"

const int TFT_DC = 2;
const int TFT_CS = 39;

//Adafruit_ST7735 tft = Adafruit_ST7735(-1,  2, 0/*no reset*/);
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#define SW1 5
#define SW2 39
#define SW3 36

#define LEDR 4
#define LEDG 0

ARQ SlaveChannel(32,&Serial2);
uint8_t *UserData;
uint8_t *data_pointer;
bool _serial=true;


void displayRGB565(unsigned char * frame, int xres, int yres)
{
  tft.setAddrWindow(0, 0, yres-1 , xres-1 );
  int i = 0;
  for(int x = 0; x < xres; x++)
    for(int y = 0; y < yres; y++)
    {
      i = (y * xres + x) << 1;
      tft.pushColor((frame[i] | (frame[i+1] << 8)));
    }  
}

void mySlaveConnectEvent()
{
  Serial.print("Master connection:");
  Serial.println(SlaveChannel.IsConnected());
  
  if(!SlaveChannel.IsConnected())
  {
    displayRGB565(UserData,160,120);
  }
}

void mySlaveStatusEvent()
{
  Serial.println("Status Event");
}

void myRequestStatusEvent()
{
  Serial.print("User index:");
  Serial.println(SlaveChannel.UserIndex());
 // data_pointer=UserData+7680*SlaveChannel.UserIndex();//beginning of the package

  printf("User_Data:%p Data_Pointer:%p\n",UserData,data_pointer);
}

void setup() {
 
  SlaveChannel.attachConnectInterrupt(mySlaveConnectEvent);
  SlaveChannel.attachStatusInterrupt(mySlaveStatusEvent);
  SlaveChannel.attachRequestInterrupt(myRequestStatusEvent);
  
  Serial.begin(115200);
  Serial2.begin(19200,SERIAL_8N1, -1, -1, true, 100UL);
  pinMatrixOutDetach(16, false, false);
  pinMode(16, INPUT);
  pinMatrixInAttach(16, UART_RXD_IDX(2), false);

  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  
  tft.init();
 // tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(40, 5);
  tft.println(F("Arduino"));
  tft.setCursor(35, 25);
  tft.println(F("Cellular"));
  tft.setCursor(35, 45);
  tft.println(F("Automata"));
  
  delay(1000);
  pinMode(SW1,INPUT);
  Serial.println("Slave Begun");
  delay(1000);
  UserData=(uint8_t*)malloc(38400);
  Serial.print("Data ADDR:");
  Serial.println(UserData==NULL);
  printf("%p\n",UserData);
  delay(1000);
  for(uint16_t i=0;i<38400;i++)
    UserData[i]=0xFF;
  tft.fillScreen(TFT_BLACK);
  //displayRGB565(myimage,160,120);
}

unsigned long error=0,i=0;
char result=0,incoming=0;
void loop() {
   //Serial2.write(0xAA);
    //Serial2.println("Emre KARABAKLA, Naber Selam Deneme Test Emre KARABAKLA, Naber Selam Deneme Test");
  //  delay(100);
  SlaveChannel.SlaveLoop(UserData,7680,5000,50);
  while(!digitalRead(SW1))
  {
    while(Serial2.available())  
    {
        Serial.print(Serial2.read(),HEX);
    } 
  }
  while(!digitalRead(SW2))
  {
    Serial2.write(0xAA);
    Serial2.write(0xEE);
    delay(500);
  }
  digitalWrite(LEDR,digitalRead(SW1));
  digitalWrite(LEDG,digitalRead(SW2));
/*  Serial.print(digitalRead(SW1));
  Serial.print(",");
  Serial.print(digitalRead(SW2));
  Serial.print(",");
  Serial.println(digitalRead(SW3));
  */
}
