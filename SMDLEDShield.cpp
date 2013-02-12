#include "SMDLEDShield.h"

EEMEM char _store[256];

void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);
  for (int k=7; k>=0; k--)  {
    digitalWrite(myClockPin, 0);
    if (myDataOut & (1 << k)) {
      digitalWrite(myDataPin, 1);
    } else {	
      digitalWrite(myDataPin, 0);
    }
    digitalWrite(myClockPin, 1);
    digitalWrite(myDataPin, 0);
  }
  digitalWrite(myClockPin, 0);
}

PROGMEM const prog_int8_t SMDLEDShield::columns[] = {
  0b01111111, 0b10111111, 0b11011111, 0b11101111,
  0b11110111, 0b11111011, 0b11111101, 0b11111110
};

PROGMEM const prog_int8_t SMDLEDShield::charset[26][8] = {
    { 0x9, 0x9, 0x9, 0x9,
      0xf, 0x9, 0x9, 0x6 }, // A
    { 0x7, 0x9, 0x9, 0x9,
      0x7, 0x9, 0x9, 0x7 }, // B
    { 0x6, 0x9, 0x1, 0x1,
      0x1, 0x1, 0x9, 0x6 }, // C
    { 0x7, 0x9, 0x9, 0x9,
      0x9, 0x9, 0x9, 0x7 }, // D
    { 0xF, 0x1, 0x1, 0x1,
      0x7, 0x1, 0x1, 0xF }, // E
    { 0x1, 0x1, 0x1, 0x1,
      0x7, 0x1, 0x1, 0xF }, // F
    { 0x6, 0x9, 0x9, 0x9,
      0xD, 0x1, 0x9, 0x6 }, // G
    { 0x9, 0x9, 0x9, 0x9,
      0xF, 0x9, 0x9, 0x9 }, // H
    { 0x2, 0x2, 0x2, 0x2,
      0x2, 0x2, 0x2, 0x2 }, // I
    { 0x6, 0x9, 0x8, 0x8,
      0x8, 0x8, 0x8, 0x8 }, // J
    { 0x9, 0x9, 0x9, 0x5,
      0x3, 0x5, 0x9, 0x9 }, // K
    { 0xF, 0x1, 0x1, 0x1,
      0x1, 0x1, 0x1, 0x1 }, // L
    { 0x9, 0x9, 0x9, 0x9,
      0xB, 0xF, 0x9, 0x9 }, // M
    { 0x9, 0x9, 0xD, 0xD,
      0xB, 0xB, 0x9, 0x9 }, // N
    { 0x6, 0x9, 0x9, 0x9,
      0x9, 0x9, 0x9, 0x6 }, // O
    { 0x1, 0x1, 0x1, 0x1,
      0x7, 0x9, 0x9, 0x7 }, // P
    { 0x6, 0xD, 0x9, 0x9,
      0x9, 0x9, 0x9, 0x6 }, // Q
    { 0x9, 0x9, 0x5, 0x1,
      0x7, 0x9, 0x9, 0x7 }, // R
    { 0x6, 0x9, 0x8, 0x8,
      0x6, 0x1, 0x9, 0x6 }, // S
    { 0x2, 0x2, 0x2, 0x2,
      0x2, 0x2, 0x2, 0xF }, // T
    { 0x6, 0x9, 0x9, 0x9,
      0x9, 0x9, 0x9, 0x9 }, // U
    { 0x2, 0x2, 0x5, 0x5,
      0x9, 0x9, 0x9, 0x9 }, // V
    { 0x9, 0x9, 0xF, 0xB,
      0x9, 0x9, 0x9, 0x9 }, // W
    { 0x9, 0x9, 0x9, 0x6,
      0x6, 0x9, 0x9, 0x9 }, // X
    { 0x2, 0x2, 0x2, 0x2,
      0x2, 0x5, 0x9, 0x9 }, // Y
    { 0xF, 0x1, 0x2, 0x2,
      0x4, 0x4, 0x8, 0xF }, // Z
  };

SMDLEDShield::SMDLEDShield(const int _step) {
  step = _step;
  left_bit = 0;
  eeprom_read_block(text, _store, 256);
  message(text,false);
  pinMode(SMDLatchPin, OUTPUT);
}

SMDLEDShield::SMDLEDShield(const int _step, const char *_text) {
  step = _step;
  left_bit = 0;
  if(text)
    message(_text);
  pinMode(SMDLatchPin, OUTPUT);
}

void SMDLEDShield::message(const char *_text, bool store) {
  int i, j;
  int temp;
  //save a private copy of the text string
  strcpy(text, _text);
  //save to eeprom
  if (store)
    eeprom_write_block(text, _store, strlen(text)+1);
  buffer_width = strlen(_text)*4 + 16;
  memset(buffer,0,128*8);
  //Preprocess the message into font indeces
  for(i=0; i< strlen(text); i++) {
    if((64 < text[i])  && (text[i] < 91))//upper case
      temp = text[i] - 65;
    else if((96 < text[i]) && (text[i] < 123))//lower case
      temp = text[i] - 97;
    else //unmapped
      temp = -1;
    //Render the buffer
    for(j=0; j<8; j++) {
      buffer[j][(i/2)+1] |= (temp > -1)?pgm_read_byte(&(charset[(int)temp][j])) << 4*(i%2):0;
    }
  }
  
  left_bit = 0;
  //Render the initial window buffer
  renderWindow();
}

void SMDLEDShield::scroll(direction d) {
  shiftWindow(d);
  renderWindow();
}

void SMDLEDShield::draw() {
  int i;
  for (int i = 0; i < 8; i++) {
    //1. Disable all pixels (all rows low, all columns high)
    digitalWrite(SMDLatchPin, 0);
    shiftOut(SMDDataPin, SMDClockPin, 0x00); 
    shiftOut(SMDDataPin, SMDClockPin, 0x255);
    digitalWrite(SMDLatchPin, 1);
    
    //2. Push enabled rows (high) and enabled columns (low)
    digitalWrite(SMDLatchPin, 0);
    shiftOut(SMDDataPin, SMDClockPin, window[i]);
    shiftOut(SMDDataPin, SMDClockPin, pgm_read_byte(&(columns[i])));
    digitalWrite(SMDLatchPin, 1);
  }
  
  //Disable all pixels
  digitalWrite(SMDLatchPin, 0);
  shiftOut(SMDDataPin, SMDClockPin, 0x00); 
  shiftOut(SMDDataPin, SMDClockPin, 0x255);
  digitalWrite(SMDLatchPin, 1);
}

/* private interface */
void SMDLEDShield::renderWindow() {
  int i;
  int byte_index, right_offset;
  uint16_t *region;

  byte_index = left_bit/8;
  right_offset = left_bit%8;

  for(i=0; i<8; i++) {
    //capture the 16 bit region address
    region = (uint16_t*)(&(buffer[i][byte_index]));
    //dereference, shift right, and stuff the first byte into the window
    window[i] = (uint8_t)(*region>>right_offset);
  }
}

void SMDLEDShield::shiftWindow(direction d) {
  left_bit = (left_bit + d*step)%buffer_width;
}
