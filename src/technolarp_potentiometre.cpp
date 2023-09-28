#include "technolarp_potentiometre.h"

M_potentiometre::M_potentiometre()
{
	potPin = POTENTIOMETER_PIN;
	
	readCount = 5;
	
	pinMode(potPin, INPUT);
}

M_potentiometre::~M_potentiometre()
{
  
}

void M_potentiometre::setReadCount(uint16_t toSet)
{
	readCount = toSet;
}

uint16_t M_potentiometre::getReadCount()
{
	return(readCount);
}


uint16_t M_potentiometre::readPotValue()
{
  uint16_t readValue = 0;
	
  // read potentiometer
  for (uint8_t i=0;i<readCount;i++)
  {
    readValue += analogRead(potPin);
  }
  readValue /= readCount;
  
  return(readValue);
}