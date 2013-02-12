#ifndef SMDLEDSHIELD_H
#define SMDLEDSHIELD_H
#include <Arduino.h>
#include <WString.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <inttypes.h>

#define SMDLatchPin  8
#define SMDDataPin  11
#define SMDClockPin 12

enum direction {left=-1, right=1};

class SMDLEDShield {
  /* class members */
  
 public:
  
 private:
  PROGMEM static const prog_int8_t columns[];
  
  //Each character is actually a half-byte per row
  PROGMEM static const prog_int8_t charset[26][8];
  
  /* interface */
 public:
  SMDLEDShield(const int step);
  SMDLEDShield(const int step, const char *_text);
  
  void message(const char *_text, bool store=true);
  void scroll(const direction d);
  void draw();
 private:
  void renderWindow();
  void shiftWindow(const direction d);
  
  /* instance members */
 public:
  uint8_t buffer[8][128];
  uint8_t window[8];
  unsigned int left_bit;
  unsigned int buffer_width;
  unsigned int step;
  char text[256];
};

#endif
