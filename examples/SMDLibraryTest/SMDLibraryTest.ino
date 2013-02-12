#include <SMDLEDShield.h>

char buf[256] = "USE FREEBSD";

unsigned int tcnt2;

SMDLEDShield *s;
direction d = right;
int j = 0;
int frames;

void setup() {
  Serial.begin(9600);
  s = new SMDLEDShield(1);
  
  /* First disable the timer overflow interrupt while we're configuring */  
  TIMSK2 &= ~(1<<TOIE2);  
  
  /* Configure timer2 in normal mode (pure counting, no PWM etc.) */  
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));  
  TCCR2B &= ~(1<<WGM22);  
  
  /* Select clock source: internal I/O clock */  
   ASSR &= ~(1<<AS2);  
  
  /* Disable Compare Match A interrupt enable (only want overflow) */  
  TIMSK2 &= ~(1<<OCIE2A);  
  
  /* Now configure the prescaler to CPU clock divided by 128 */  
  TCCR2A |= (1<<CS22)  | (1<<CS20); // Set bits  
  TCCR2B &= ~(1<<22) | (1<<CS20) ; 
  
  /* We need to calculate a proper value to load the timer counter. 
   * The following loads the value 131 into the Timer 2 counter register 
   * The math behind this is: 
   * (CPU frequency) / (prescaler value) = 125000 Hz = 8us. 
   * (desired period) / 8us = 125. 
   * MAX(uint8) + 1 - 125 = 131; 
   */  
  /* Save value globally for later reload in ISR */  
  tcnt2 = 1;   
  
  /* Finally load end enable the timer */  
  TCNT2 = tcnt2;
  TIMSK2 |= (1<<TOIE2);  
}

/* 
 * Install the Interrupt Service Routine (ISR) for Timer2 overflow. 
 * This is normally done by writing the address of the ISR in the 
 * interrupt vector table but conveniently done by using ISR()  */  
ISR(TIMER2_OVF_vect) {  
  /* Reload the timer */ 
  static int i=0;
   i = ++i % 30;
  if(i%3 == 0)
     s->draw();
  if(i == 0)
    s->scroll(d);
  TCNT2=tcnt2;
}  

void loop() {
  int i=0;
  int n=0;
  n=0;
  if(Serial.available()) {
    //disable the timer2 ISR while we read serial
    TIMSK2 &= ~(1<<TOIE2);
    do {
      if(Serial.available()) {
         buf[n] = Serial.read();
         n++;
      }
    } while((n <= 256) && (buf[n-1] != '\n'));
    TIMSK2 |= (1<<TOIE2);  
  }
  if(n > 0) {
    buf[n]=0;
    s->message(buf);
  }
}
