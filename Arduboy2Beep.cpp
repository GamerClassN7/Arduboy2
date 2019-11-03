/**
 * @file Arduboy2Beep.cpp
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#include <Arduino.h>
#include "Arduboy2Beep.h"
#include "Arduboy2Core.h"


#if defined (ESP32) 
// dummy functions because tone is not implemented yet

#endif

uint8_t BeepPin1::duration = 0;

void BeepPin1::begin()
{
#if !defined (ESP8266) && !defined (ESP32)
  TCCR3A = 0;
  TCCR3B = (bit(WGM32) | bit(CS31)); // CTC mode. Divide by 8 clock prescale
#endif
}

void BeepPin1::tone(uint16_t count)
{
#if defined (ESP8266)
	::tone(PIN_SPEAKER_1, count);
#elif defined (ESP32)
	
#else
  tone(count, 0);
#endif	
}

void BeepPin1::tone(uint16_t count, uint8_t dur)
{
#if defined (ESP8266)
	::tone(PIN_SPEAKER_1, count, dur);
#elif defined (ESP32)
	
#else
  duration = dur;
  TCCR3A = bit(COM3A0); // set toggle on compare mode (which connects the pin)
  OCR3A = count; // load the count (16 bits), which determines the frequency
#endif
}

void BeepPin1::timer()
{
#if !defined (ESP8266) && !defined (ESP32)
  if (duration && (--duration == 0)) {
    TCCR3A = 0; // set normal mode (which disconnects the pin)
  }
#endif	
}

void BeepPin1::noTone()
{
#if defined (ESP8266)
	::noTone(PIN_SPEAKER_1);
#elif defined (ESP32)
	
#else
  duration = 0;
  TCCR3A = 0; // set normal mode (which disconnects the pin)
#endif
}


// Speaker pin 2, Timer 4A, Port C bit 7, Arduino pin 13

uint8_t BeepPin2::duration = 0;

void BeepPin2::begin()
{
#if !defined (ESP8266) && !defined (ESP32)
  TCCR4A = 0; // normal mode. Disable PWM
  TCCR4B = bit(CS43); // divide by 128 clock prescale
  TCCR4D = 0; // normal mode
  TC4H = 0;  // toggle pin at count = 0
  OCR4A = 0; //  "
#endif
}

void BeepPin2::tone(uint16_t count)
{
#if defined (ESP8266)
	::tone(PIN_SPEAKER_2, count);
#elif defined (ESP32)
	
#else
  tone(count, 0);
#endif	
}

void BeepPin2::tone(uint16_t count, uint8_t dur)
{
#if defined (ESP8266)
	::tone(PIN_SPEAKER_2, count, dur);	
#elif defined (ESP32)
	
#else
  duration = dur;	
  TCCR4A = bit(COM4A0); // set toggle on compare mode (which connects the pin)
  TC4H = highByte(count); // load the count (10 bits),
  OCR4C = lowByte(count); //  which determines the frequency
#endif
}

void BeepPin2::timer()
{
#if !defined (ESP8266) && !defined (ESP32)	
  if (duration && (--duration == 0)) { 
    TCCR4A = 0; // set normal mode (which disconnects the pin)
  }		
#endif
}

void BeepPin2::noTone()
{
#if defined (ESP8266)
	::noTone(PIN_SPEAKER_2);
#elif defined (ESP32)
	
#else
  duration = 0;
  TCCR4A = 0; // set normal mode (which disconnects the pin)
#endif
}