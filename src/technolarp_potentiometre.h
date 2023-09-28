#include <Arduino.h>
#define POTENTIOMETER_PIN A0

class M_potentiometre
{
  private:
  uint8_t potPin;    
  uint16_t readCount;
  
  public:  
  M_potentiometre();
  ~M_potentiometre();
    
  void setReadCount(uint16_t toSet);
  uint16_t getReadCount();
  
  uint16_t readPotValue();
};
