#include "my_cam_settings.h"
#include "myimage.h"
#define HardwareSH
#include "C:\\Users\\karab\\OneDrive\\Emre\\Designs\\Visible Light Com\\Arduino\\Comm_Protocol\\ARQ.h"

#define SW4 26
#define SW3 25
#define SW2 39
#define SW1 36

ARQ SlaveChannel(256,&Serial2);

OV7670 *camera;
uint8_t *frame_package;//[7680]; //160*120*2 bytes/5

void mySlaveConnectEvent()
{
  Serial.print("Master connection:");
  Serial.println(SlaveChannel.IsConnected());
  
  if(!SlaveChannel.IsConnected())
  {
    
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
  //frame_package=camera->frame+7680*SlaveChannel.UserIndex();//beginning of the package
}
#define mlen 16
#define mlen2 5
uint8_t mydata[mlen];
uint8_t mydata2[mlen2];
void setup() {

  SlaveChannel.attachConnectInterrupt(mySlaveConnectEvent);
  SlaveChannel.attachStatusInterrupt(mySlaveStatusEvent);
  SlaveChannel.attachRequestInterrupt(myRequestStatusEvent);
  
  Serial.begin(115200);
  Serial2.begin(19200,SERIAL_8N1, -1, -1, true, 100UL);
  pinMatrixOutDetach(16, false, false);
  pinMode(16, INPUT);
  pinMatrixInAttach(16, UART_RXD_IDX(2), false);
  
  pinMode(SW1,INPUT);
  pinMode(SW2,INPUT);
  pinMode(SW3,INPUT);
  pinMode(SW4,INPUT);

  //camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  //delay(100);
  //camera->oneFrame();
 // frame_package=camera->frame;
  Serial.println("Slave Begun"); 

  for(int i=0;i<mlen;i++)
    mydata[i]=0xEE;
  for(int i=0;i<mlen2;i++)
    mydata2[i]=0xAA;
  //Serial.println("\n\n\n");
  //Serial.println("Frame_Begin");
 // Serial.write(myimage,38400);
}

void loop() {
  //SlaveChannel.SlaveLoop(camera->frame,7680,500,50);
  SlaveChannel.SlaveLoop(myimage,7680,500,50);
  while(digitalRead(SW4))
  {
    while(Serial2.available())  
    {
        Serial.print((char)Serial2.read());
    }
  }
  while(digitalRead(SW3))
  {
   // Serial2.write(0x00);
    Serial2.write(0xAA);
    Serial2.write(0xEE);
    //Serial2.write(0xAA);
    //Serial2.println("Emre KARABAKLA, Naber Selam Deneme Test Emre KARABAKLA, Naber Selam Deneme Test");
    //for(int i=0;i<33;i++)
   // Serial2.write(mydata2,mlen2);
    //delay(10);
    //Serial2.write(mydata,mlen);
   // while(Serial2.availableForWrite()<35);
    delay(500);
  }
}
