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


#include <Wire.h>
#include <avr/power.h>
#include "SoftwareSerial.h"
#include "FastLED.h"
#define NUM_LEDS 8
#define DATA_PIN 12
CRGB leds[NUM_LEDS];

#include <SPI.h>


SoftwareSerial K_30_Serial(10,12);  //Sets up a virtual serial port


byte readCO2[] = {0x10, 0x13, 0x01, 0x10, 0x1F, 0x00, 0x53};
                
//byte readCO2[] = {0x10, 0x13, 0x01, 0x10, 0x1F, 0x00, 0x53};  //Command packet to read Co2 (see app note)
//byte readCO2[] = {0x10, 0x13, 0x06, 0x10, 0x1F, 0x00, 0x58};
byte response[] = {0,0,0,0,0,0,0,0,0};  //create an array to store the response
const int buzzerPin =  3;


void setup() 
{

FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  pinMode(buzzerPin, OUTPUT);
  //pinMode(onPin, OUTPUT);     
  Serial.begin(9600);
  Serial.print("INIT");
  digitalWrite(buzzerPin, HIGH);
  Wire.begin();
  digitalWrite(buzzerPin, LOW);
  delay(2000);
  digitalWrite(buzzerPin, LOW);
  delay(500);
  digitalWrite(buzzerPin,HIGH);
  delay(500);
  digitalWrite(buzzerPin,LOW);
  K_30_Serial.begin(38400);    //Opens the virtual serial port with a baud of 38400

Kwrite(readCO2);


}


float co2perc,tcoC, pressure, atm, tcoI, signal, voltage;
long uptime=0,olduptime=0;


void loop() 
{
 int i=0,k=0,ttt;
 char myHexString[10],myHexString2[10];

 
 byte a;
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
     byte b=a;
     a=K_30_Serial.read();
     if ((b==0x10)&&(a==0x19))
      Serial.println("NACK--------------------------NACK");
             
     if ((b==0x10)&&(a==0x1A))
      {
//-----               Serial.println("DLE DAT ------");
       int dlen=K_30_Serial.read();
       Serial.print("----------DataLEN ");
       Serial.println(dlen);
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
           tcoI=prendi4()*0.55;
           z+=4;
          }

         if (z==511)
           {
            Serial.print("dtr: ");
            signal=prendi2();
            Serial.println(signal);
            z+=2;
           }
////?dopotemplow
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
             ttt=0;
            }
           if (z==19)
            {
             Serial.print("uptime: ");
             uptime=prendi4();
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
//-----                      Serial.println("SNOOP IN DLEN");
            }
           else
            z++;
           }//aaa
          //fine while dlen               
          //Serial.print(a);
         Serial.println(" fine DATA");
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
               a=K_30_Serial.read();
               if ((b==0x10)&&(a==0x10))
                {
                  b=a;
                  a=K_30_Serial.read();
                }       
               if (a==0xFF)
                {
                  ff++;
//-----                  if (ff>3)
//-----                   Serial.println("breakato!");
                }
             } 
             
           //fine while cerca EOF
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

void suona(int pin, int dlay)
{
 digitalWrite(pin,HIGH);
 delay(dlay);
 digitalWrite(pin,LOW);
}

float min=0,max=0,conta=1;
void stampanum(char c[11])
{
  int red,i;
  
  union
    {
     long a;
    byte b [4];
    float f;  
    } tco;
    
   long val = 0;
   val=strtoul(c,0,16);
  
  tco.a = val;
  tco.f=abs(tco.f);
  Serial.println(c);
  float pco2=co2perc;
  digitalWrite(buzzerPin,LOW);

//////////iniled

if (uptime>30) //time for sensor to boot
{
  int led = pco2*100  ;
  Serial.print("led: ");
  Serial.println(led);
  int ik; 
  for (i=0;i<8;i++)
   leds[i] = CRGB::Black;
  FastLED.show();
  delay(100);
  ///uptime<1
  if (pco2==0.00)
   {
     leds[0] = CRGB::Green;
   }


  if ((pco2>0)&&(pco2<0.09))
   {

    for (ik=0;ik<=led;ik++)
     {
      if (led<4)
       leds[ik] = CRGB::Green;
      if ((led>=4)&&(led<6))
       leds[ik] = CRGB::Yellow;
      if (led>=6)
       leds[ik] = CRGB::Red;  
     }
   }
   
 if ((pco2>0.03)&&(pco2<=0.05))
  suona(buzzerPin,200);
  
 if ((pco2>=0.09)&&(pco2<0.16))
  {
    //int index = led - 8;
    for (ik=0;ik<(led-8);ik++)
     {
       leds[(led-8)-ik] = CRGB::Red;
     }
     leds[0]=CRGB::Blue;   
  }
 if (pco2>=0.16)
  {
    leds[0] = CRGB::Blue;
    leds[1] = CRGB::Blue;
    leds[2] = CRGB::Red;
    leds[3] = CRGB::Red;
    leds[4] = CRGB::Red;
    leds[5] = CRGB::Red;
    leds[6] = CRGB::Red;
    leds[7] = CRGB::Red;

  }

 if (pco2>0.05)
  suona(buzzerPin,300);
 FastLED.show();
}
else
{
  int ik;
  for (ik=0;ik<9;ik++)
   {
     leds[ik] = CRGB::Blue;
     delay(100);
     FastLED.show();
   }
    for (ik=0;ik<9;ik++)
   {
     leds[ik] = CRGB::Black;
     delay(100);
     FastLED.show();
   }

}
//////////endled


if (pco2>=0.50)
 {
   red=1;
      digitalWrite(buzzerPin,HIGH);
 }


 if (tco.f>0.25)
  digitalWrite(buzzerPin,HIGH);
 else
  digitalWrite(buzzerPin,LOW);

  Serial.print ("valore CO2: ");
  if ((max<tco.f)&&(tco.f<3))
   max=tco.f;
  if (tco.f>0)
   {
     min+=tco.f;
     conta++;
   }
  Serial.println (tco.f,4);
  //delay(500);
}




void Kwrite(byte pkt[])
{
 for (int k=0; k<7; k++)
  K_30_Serial.write(readCO2[k]);
}


