/*
   ----------------------------------------------------------------------------
   TECHNOLARP - https://technolarp.github.io/
   VU-METRE 01 - https://github.com/technolarp/vumetre_01
   version 1.0 - 09/2021
   ----------------------------------------------------------------------------
*/

/*
   ----------------------------------------------------------------------------
   Pour ce montage, vous avez besoin de 
   1 potentiometre 10Hohms
   3 ou + led neopixel
   ----------------------------------------------------------------------------
*/

/*
   ----------------------------------------------------------------------------
   PINOUT
   D0     NEOPIXEL
   A0     POTENTIOMETRE
   ----------------------------------------------------------------------------
*/

#include <Arduino.h>

// TASK SCHEDULER
#define _TASK_OO_CALLBACKS
#include <TaskScheduler.h>
Scheduler globalScheduler;

// LED RGB
#include <technolarp_fastled.h>
M_fastled* aFastled;

uint8_t indexLed = 0;
uint8_t indexLedPrevious = 0;

// CONFIG
#include "config.h"
M_config aConfig;

// POTENTIOMETER
#define POTENTIOMETER_PIN A0

// DIVERS
bool uneFois = true;

// HEARTBEAT
unsigned long int previousMillisHB;
unsigned long int currentMillisHB;
unsigned long int intervalHB;


/*
   ----------------------------------------------------------------------------
   SETUP
   ----------------------------------------------------------------------------
*/
void setup()
{
  Serial.begin(115200);

  // VERSION
  delay(500);
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("----------------------------------------------------------------------------"));
  Serial.println(F("TECHNOLARP - https://technolarp.github.io/"));
  Serial.println(F("VU-METRE 01 - https://github.com/technolarp/vumetre_01"));
  Serial.println(F("version 1.0 - 09/2021"));
  Serial.println(F("----------------------------------------------------------------------------"));
  
  // CONFIG OBJET
  Serial.println(F(""));
  Serial.println(F(""));
  aConfig.mountFS();
  aConfig.listDir("/");
  aConfig.listDir("/config");
  aConfig.listDir("/www");
  
  aConfig.printJsonFile("/config/objectconfig.txt");
  aConfig.readObjectConfig("/config/objectconfig.txt");

  //aConfig.printJsonFile("/config/networkconfig.txt");
  //aConfig.readNetworkConfig("/config/networkconfig.txt");

  // POTENTIOMETER
  pinMode(POTENTIOMETER_PIN, INPUT);
  
  // LED RGB
  aFastled = new M_fastled(&globalScheduler);
  aFastled->setNbLed(aConfig.objectConfig.activeLeds);
  aFastled->setBrightness(aConfig.objectConfig.brightness);

  // animation led de depart  
  aFastled->allLedOff();
  for (int i = 0; i < aConfig.objectConfig.activeLeds * 2; i++)
  {
    aFastled->ledOn(i % aConfig.objectConfig.activeLeds, CRGB::Blue);
    delay(50);
    aFastled->ledOn(i % aConfig.objectConfig.activeLeds, CRGB::Black);
  }
  aFastled->allLedOff();

  // HEARTBEAT
  currentMillisHB = millis();
  previousMillisHB = currentMillisHB;
  intervalHB = 5000;

  // SERIAL
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(F("START !!!"));
}
/*
   ----------------------------------------------------------------------------
   FIN DU SETUP
   ----------------------------------------------------------------------------
*/




/*
   ----------------------------------------------------------------------------
   LOOP
   ----------------------------------------------------------------------------
*/
void loop()
{  
  // avoid watchdog reset
  yield();
  
  // manage task scheduler
  globalScheduler.execute();

  // read potentiometer
  uint16_t readValue = 0;
  uint16_t readCount = 5;

  for (uint16_t i=0;i<readCount;i++)
  {
    readValue += analogRead(POTENTIOMETER_PIN);
  }
  readValue /= readCount;

  indexLed = map(readValue, 0, 1024, 0, aConfig.objectConfig.activeLeds);

  if (indexLedPrevious!=indexLed)
  {
    indexLedPrevious=indexLed;

    for (int i=0;i<aConfig.objectConfig.activeLeds;i++)
    {
      if (i<indexLed)
      {
        if (i<aConfig.objectConfig.seuil1)
        {
          aFastled->setLed(i, aConfig.objectConfig.couleur1);
        }
        else if (i<aConfig.objectConfig.seuil2)
        {
          aFastled->setLed(i, aConfig.objectConfig.couleur2);
        }
        else
        {
          aFastled->setLed(i, aConfig.objectConfig.couleur3);
        }        
      }
      else
      {
        aFastled->setLed(i, CRGB::Black);
      }
    }
    aFastled->ledShow();
  }  

  // HEARTBEAT
  currentMillisHB = millis();
  if(currentMillisHB - previousMillisHB > intervalHB)
  {
    previousMillisHB = currentMillisHB;
    
  }
}
/*
   ----------------------------------------------------------------------------
   FIN DU LOOP
   ----------------------------------------------------------------------------
*/





/*
   ----------------------------------------------------------------------------
   FONCTIONS ADDITIONNELLES
   ----------------------------------------------------------------------------
*/
