#include <FastLED.h>  // https://github.com/FastLED/FastLED  // VERSION 3.6.0

#define NB_LEDS_MAX 50
#define NB_COULEURS_MAX 5
#define LED_DATA_PIN D0

class M_fastled
{
  private:
	uint8_t nbLeds;
	
	bool animationActive;
	bool ledStatus;
	uint16_t iterations;
	
	uint16_t interval;
	uint32_t previousMillis;
	
	CRGB couleurs[NB_COULEURS_MAX];
	
	enum {ANIM_NONE, ANIM_BLINK, ANIM_SERPENT};
	uint8_t animActuelle;
	
	bool animForever;
	
	uint8_t indexLed;
	
	bool increaseBrightness;
	uint8_t indexBrightness;
	uint16_t intervalScintillement;
    uint16_t scintillementOnOff;
	uint32_t previousMillisBrightness;
      
  public:  
	M_fastled();
	
	// tableau de leds neopixel
	CRGB leds[NB_LEDS_MAX];

	void ledOn(uint8_t ledToSet, CRGB colorToSet, bool change);
	void allLedOn(CRGB colorToSet, bool change);
	void ledOff(uint8_t ledToSet, bool change);
	void allLedOff(bool change);
	void allLedOff();
	
	
	void setNbLed(uint8_t nbLedsInit);
	uint8_t getNbLed();
	void setBrightness(uint8_t newBrightness);
	
	void ledShow();
	
	void updateAnimation();
	void switchAnim(uint8_t anim);
	void switchAnimEnd(uint8_t anim);
	
	void animationDepart(uint16_t intervalToSet, uint16_t iterationToSet, CRGB colorToSet1);
	
	void animationBlink01Start(uint16_t intervalToSet, uint16_t iterationToSet, CRGB colorToSet1, CRGB colorToSet2);
	void animationBlink02Start(uint16_t intervalToSet, uint16_t dureeToSet, CRGB colorToSet1, CRGB colorToSet2, bool forever);
	void animationBlink02Start(uint16_t intervalToSet, uint16_t dureeToSet, CRGB colorToSet1, CRGB colorToSet2);
	void animationBlink();
	void animationBlinkEnd();
	
	void animationSerpent01Start(uint16_t intervalToSet, uint16_t iterationToSet, CRGB colorToSet1, CRGB colorToSet2);
	void animationSerpent02Start(uint16_t intervalToSet, uint16_t dureeToSet, CRGB colorToSet1, CRGB colorToSet2);
	void animationSerpent();
	void animationSerpentEnd();	
	
	
	bool isAnimActive();
	void setAnimation(uint8_t toSet);
	
	void controlBrightness(uint8_t maxBrightness);
	void setControlBrightness(bool toSet);
	bool getControlBrightness();
	void setIntervalControlBrightness(uint16_t toSet);
	uint16_t getIntervalControlBrightness();
	
};
