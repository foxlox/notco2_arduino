/*************************************************** 
  This is a real time CO2 reader for Arduino.
  Written by Fortunato Lodari.  
  Apache 2 license, all text above must be included 
  in any redistribution.
  
  Copyright 2019 fortunato lodari fox@thebrain.net

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
  
 ****************************************************/

 /****************************************************
  NOTE ON DISPLAY
  This display has a particularity, when you write a char and then another in the same
  location, the previous one does not disappear, so you do not read it well. The software overwrites
  the char of the previous output using the black color to delete it, instead to do a CLEAR().
  Ex: Background black, write a green A in (0,0), and to delete it write a black A in (0,0)
      before to write a new char
 ****************************************************/


#include <Wire.h>
#include <avr/power.h>
#include "SoftwareSerial.h"


#define nome "Owner Name"
/*
#define CS   4
#define SCLK 5
#define MOSI 6
#define DC   7
#define RST  8

cs - cs
sclk2 - scl
mosi - sda
dc - rs
rst  - reset
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

Adafruit_ST7735 tft = Adafruit_ST7735(4, 7, 6, 5, 8);
SoftwareSerial K_30_Serial(10,12);  //Sets up a virtual serial port


byte readCO2[] = {0x10, 0x13, 0x01, 0x10, 0x1F, 0x00, 0x53};
//byte readCO2[] = {0x10, 0x13, 0x01, 0x10, 0x1F, 0x00, 0x53};  //Command packet to read Co2 (see app note)
//byte readCO2[] = {0x10, 0x13, 0x06, 0x10, 0x1F, 0x00, 0x58};

const int buzzerPin =  3;


void splash()
{
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.setTextWrap(false);
  digitalWrite(buzzerPin, LOW);
  tft.fillScreen(ST7735_BLACK);
  tft.drawRect(0,0,160,128,ST7735_BLUE);
  printtfttitoli("Co2",30,50,ST7735_RED,0,5);
  delay(2000);
  digitalWrite(buzzerPin, LOW);
  tft.drawLine(20,40,120,100,ST7735_GREEN);
  delay(500);
  tft.drawLine(120,40,20,100,ST7735_GREEN);
  delay(1000);
  printtfttitoli("not",50,20,ST7735_GREEN,0,3);
  delay(500);
  printtfttitoli("(c) fox lodari 2012-2019",2,10,ST7735_MAGENTA,0,1);
  printtfttitoli(nome,2,100,ST7735_YELLOW,0,1);
  digitalWrite(buzzerPin,HIGH);
  delay(500);
  printtfttitoli("Auto calibration...",2,110,ST7735_YELLOW,0,1);
  delay(1000);
  delay(500);
  digitalWrite(buzzerPin,LOW);
  tft.fillScreen(ST7735_BLACK);  
  tft.drawRect(0,0,160,18,ST7735_WHITE);
  tft.drawRect(0,18,160,28,ST7735_WHITE);
  tft.drawRect(0,46,160,40,ST7735_WHITE);
  tft.drawRect(0,86,160,22,ST7735_WHITE);
  tft.drawRect(0,108,160,20,ST7735_WHITE);
  tft.fillRect(1,1,159,17,ST7735_GREEN);
  printtfttitoli(nome,20,5,ST7735_BLACK,0,1);
}

void setup() 
{
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
  Serial.print("INIT");
  digitalWrite(buzzerPin, HIGH);
  Wire.begin();
  splash();
  K_30_Serial.begin(38400);    //Opens the virtual serial port with a baud of 38400

  Kwrite(readCO2);

//temp
  
}




float pco2;
long uptime=0,olduptime;

float co2perc,tco2C, pressure, tco2I,oldpco2,oldsignal,oldtco2,oldtco2I, oldatm, voltage, oldvoltage;

void loop() 
{
  byte a;
  char myHexString[10],myHexString2[10];
  int i=0,k=0,ttt=0;
  Kwrite(readCO2);
  a=K_30_Serial.read();
  Serial.print("0x");
  Serial.print(a,HEX);
  Serial.print(" "); 
  i=0;

  while(K_30_Serial.available())  //flush whatever we have
   {
    int sensorValue = analogRead(A3);
    Serial.print("sensorv    ");
    Serial.println(sensorValue);
    voltage = sensorValue* (3.7 / 1023);
    Serial.print("voltage    ------------------1-");
    Serial.println(voltage,2);
    i++;
    //Serial.println("leggo");
    byte b=a;
    a=K_30_Serial.read();
    if ((b==0x10)&&(a==0x19))
     Serial.println("NACK--------------------------NACK");
           
    if ((b==0x10)&&(a==0x1A))
     {
//-----               Serial.println("DLE DAT ------");
      int dlen=K_30_Serial.read();
      //dlen=K_30_Serial.read();
      Serial.print("----------DataLEN ");
      Serial.println(dlen);
      //delay(50); //---hdelay
      int z=0;
      a=K_30_Serial.read();
              
      while (z<dlen-1)
       {
         b=a;
         if ((b==0x10)&&(b==a))
         a=K_30_Serial.read();
        if (z==3)
         {
          Serial.print("co2 % ");
          co2perc=prendi4();
          z+=4;
         }
  
        if (z==7)
         {
          Serial.print("temp ");
          tco2I=prendi4()*0.55;
          z+=4;
         }
  
        if (z==511)
         {
          Serial.print("dtr: ");
          float signal=prendi2();
          Serial.println(signal);
          z+=2;
         }
        if (z==513)
         {
          Serial.print("ref: ");
          long dethigh=prendi2();
          Serial.println(dethigh);
          z+=2;
         }
                    
       if (z==15)
        {
         Serial.println("in z");          
         int st=0;
         for (int t = 0; t < 4; t++) 
          {
            a=K_30_Serial.read();
            b=a;
            if ((b==0x10)&&(b==a))
             a=K_30_Serial.read();
  //-----    Serial.print(a,HEX);
           myHexString[0+st] = (a >> 4) + 0x30;
           if (myHexString[0+st] > 0x39) myHexString[0+st] +=7;
           myHexString[1+st] = (a & 0x0f) + 0x30;
           if (myHexString[1+st] > 0x39) myHexString[1+st] +=7;
            st+=2;
  //-----    Serial.print("v: ");
  //-----    Serial.println(a,HEX);
           z++;
          }
         }   
        if ((z==19)&&(dlen==32))
         {
          Serial.print("CO2 HEX: ");
          myHexString2[0]=myHexString[6];
          myHexString2[1]=myHexString[7];
          myHexString2[2]=myHexString[4];
          myHexString2[3]=myHexString[5];
          myHexString2[4]=myHexString[2];
          myHexString2[5]=myHexString[3];
          myHexString2[6]=myHexString[0];
          myHexString2[7]=myHexString[1];
          char zerox[11]="0x";
          for (int j=0;j<8;j++)
           zerox[j+2]=myHexString2[j];
        //-----  Serial.println(zerox); 
          long val = 0;
          val=strtoul(zerox,0,16);
  //-----Serial.println(val);   
          stampanum(zerox);
          //delay(500);
          ttt=0;
         }
         if (z==19)
          {
            Serial.print("uptime: ");
            uptime=prendi4()/60;
            z+=4;
          }
                    
  //dtr low  
         if (z==2223)
          {
           Serial.print("dtrl: ");
           long detlow=prendi2();
           Serial.println(detlow);
           z+=2;
          }
          
                      
  ////dethig
         if (z==2225)
          {
           Serial.print("dtrh: ");
           long dethigh=prendi2();
           Serial.println(dethigh);
           z+=2;
          }
  
  ////reflow
         if (z==2227)
          {
           Serial.print("refl: ");
           long reflow=prendi2();
           Serial.println(reflow);
           z+=2;
          }
  ////refhig
         if (z==2229)
          {
           Serial.print("refh: ");
           long refhigh=prendi2();
           Serial.println(refhigh);
           z+=2;
          }
   
         a=K_30_Serial.read();
         
         if ((b==0x10)&&(a==0x10))
          {
           Serial.println("SNOOP IN DLEN");
          }
         else
          z++;
      }
   //fine while dlen               
   //Serial.print(a);
   Serial.println(" end DATA");
   Serial.println();
               
   Kwrite(readCO2);
   delay(500); //era 500
   int in=0;
//-----            Serial.println("in while cerca EOF");
   int ff=0;
   while (((b != 0x10)&&(a != 0x1F))||(ff>4))
    {
      in++;
      b=a;
//-----               Serial.print("0x");
//-----               Serial.print(a,HEX);
//-----               Serial.print(" ");
      a=K_30_Serial.read();
//-----               Serial.print("q");
      if ((b==0x10)&&(a==0x10))
       {
//-----                  Serial.print("snoop");
         b=a;
         a=K_30_Serial.read();
       }       
      if (a==0xFF)
       {
         ff++;
//-----                  if (ff>3)
//-----                   Serial.println("breckato!");
       }
    } 
             
  //fine while cerca EOF
//-----           if (in==0)
//-----            Serial.print(" non in ");
//-----           Serial.println(" EOF - ");
  a=K_30_Serial.read();
  int crc=K_30_Serial.read();
  }           
 }
}

float prendi4()
{
 char dtrh[11];
 int st=0;
 for(int ud=0;ud<4;ud++)
  {
    byte a=K_30_Serial.read();
    char b=a;
    if ((b==0x10)&&(b==a))
      a=K_30_Serial.read();

    dtrh[0+st] = (a >> 4) + 0x30;
    if (dtrh[0+st] > 0x39) dtrh[0+st] +=7;
    dtrh[1+st] = (a & 0x0f) + 0x30;
    if (dtrh[1+st] > 0x39) dtrh[1+st] +=7;
     st+=2;
  }
 char a2[11];
 a2[0]='0'; a2[1]='x'; a2[2]=dtrh[6]; a2[3]=dtrh[7]; a2[4]=dtrh[4]; a2[5]=dtrh[5]; a2[6]=dtrh[2]; a2[7]=dtrh[3]; a2[8]=dtrh[0]; a2[9]=dtrh[1];
 
 union
  {
   long a;
   byte b [4];
   float f;  
  } t;
    
 t.a=strtoul(a2,0,16);
 Serial.println(t.f);
 return (t.f);
}

long prendi2()
{
 char dtrh[8];
 int st=0;
 for(int ud=0;ud<2;ud++)
  {
    byte a=K_30_Serial.read();
    char b=a;
    if ((b==0x10)&&(b==a))
     a=K_30_Serial.read();

   dtrh[0+st] = (a >> 4) + 0x30;
   if (dtrh[0+st] > 0x39) dtrh[0+st] +=7;
   dtrh[1+st] = (a & 0x0f) + 0x30;
   if (dtrh[1+st] > 0x39) dtrh[1+st] +=7;
   st+=2;
  }
 char a2[10];
 a2[0]='0'; a2[1]='x'; a2[6]=dtrh[2]; a2[7]=dtrh[3]; a2[8]=dtrh[0]; a2[9]=dtrh[1]; a2[2]='0'; a2[3]='0'; a2[4]='0'; a2[5]='0';

 return strtoul(a2,0,16);
}



float min=0,max=0,conta=1;
void stampanum(char c[11])
{
  int red=1;
 
 union
  {
   long a;
   byte b [4];
   float f;  
  } tco2;
    
   long val = 0;
   val=strtoul(c,0,16);
  
  tco2.a = val;
  tco2.f=abs(tco2.f);
  Serial.println(c);
  //float pco2=co2perc;
  printtfttitoli("volt",80,110,ST7735_GREEN,0,1);
  printtfttitoli("fabs",1,22,ST7735_GREEN,0,1);
  printtfttitoli("p",2,50,ST7735_GREEN,0,1);
  printtfttitoli("CO2",2,75,ST7735_GREEN,0,1);
  int color=ST7735_WHITE;
  //Serial.println(color);

  digitalWrite(buzzerPin,LOW);

  if (pco2>=0.50)
   {
     red=1;
     color=ST7735_RED;
     digitalWrite(buzzerPin,HIGH);
      //tone(13,500,1000);
      //delay(500);
     tft.fillRect(1,1,159,17,ST7735_RED);
     printtftb(oldpco2, pco2, 50, 52, color,2,4,ST7735_WHITE);
   }
  else
   printtftb(oldpco2, pco2, 50, 52, color,2,4,ST7735_BLACK);

  if ((pco2<0.5)&&(red==1))
   {
     tft.fillRect(1,1,159,17,ST7735_GREEN);
     red=0;
   }

  printtft(oldtco2, tco2.f, 50, 22, ST7735_WHITE,4,3);

  oldtco2=tco2.f;
  oldpco2=pco2;

  if (tco2.f>0.25)
   digitalWrite(buzzerPin,HIGH);
  else
   digitalWrite(buzzerPin,LOW);

  printtfttitoli("temp",1,90,ST7735_GREEN,0,1);
  printtfttitoli("min",1,110,ST7735_GREEN,0,1);

  printtft(oldtco2I,tco2I,30,90,ST7735_WHITE,2,2);
  printtft(olduptime,uptime,30,110,ST7735_WHITE,0,2);
  printtft(oldvoltage,voltage,109,110,ST7735_WHITE,2,2);

  olduptime=uptime;
  oldtco2I=tco2I;
  oldvoltage=voltage;
  

  Serial.print ("valore CO2: ");
  if ((max<tco2.f)&&(tco2.f<3))
   max=tco2.f;
  if (tco2.f>0)
   {
     min+=tco2.f;
     conta++;
   }
  Serial.println (tco2.f,4);
  //delay(500);
 }

void printtfttitoli(char titolo[30], uint16_t x, uint16_t y, uint16_t color, int k, int dim)
{
  tft.setCursor(x,y);
  tft.setTextColor(color);
  tft.setTextSize(dim);
  tft.println(titolo);


}

void printtft(float oldval, float val, uint16_t x, uint16_t y, uint16_t color, int k, int dim)
{
  tft.setCursor(x,y);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(dim);
  tft.println(oldval,k);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(dim);
  tft.println(val,k);


}

void printtftb(float oldval, float val, uint16_t x, uint16_t y, uint16_t color, int k, int dim,int cb)
{
  tft.setCursor(x,y);
  tft.setTextColor(ST7735_BLACK,cb);
  tft.setTextSize(dim);
  tft.println(oldval,k);
  tft.setCursor(x, y);
  tft.setTextColor(color,cb);
  tft.setTextSize(dim);
  tft.println(val,k);
}



void Kwrite(byte pkt[])
{
 for (int k=0; k<7; k++)
  K_30_Serial.write(readCO2[k]);
}


