/*
  Copyright (c) 2021 Historic Electric Ltd
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:
  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.


  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.cur
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


// Detects Presence of charger plug using PP pin pulling D2 to ground

#define F_CPU 8000000  // This is used by delay.h library

#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions

#include <avr/sleep.h>
#include <avr/interrupt.h>



// Routines to set and claer bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

const int plugPin = 0;
const int out1 = 1;
const int out2 = 2;
const int out3 = 3;
const int out4 = 4;

volatile boolean f_wdt = 1;


void setup()
{
    cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
    pinMode(plugPin, INPUT_PULLUP);
    pinMode(out1, OUTPUT);
    pinMode(out2, OUTPUT);
    pinMode(out3, OUTPUT);
    pinMode(out4, OUTPUT);
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(out4, LOW);

    setup_watchdog(6); // approximately 0.5 seconds sleep

}

void loop() {
  if (f_wdt==1) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt=0;       // reset flag

   int plugState = digitalRead(plugPin);
    if (plugState == 0){
      pinMode(out1, OUTPUT);
      pinMode(out2, OUTPUT);
      pinMode(out3, OUTPUT);
      pinMode(out4, OUTPUT);
      
      digitalWrite(out1, HIGH);
      digitalWrite(out2, HIGH);
      digitalWrite(out3, HIGH);
      digitalWrite(out4, HIGH);
    }
    else {
      digitalWrite(out1, LOW);
      digitalWrite(out2, LOW);
      digitalWrite(out3, LOW);
      digitalWrite(out4, LOW);

      pinMode(out1, INPUT);
      pinMode(out2, INPUT);
      pinMode(out3, INPUT);
      pinMode(out4, INPUT);

      system_sleep();  // Send the unit to sleep
    }
 }
}

void system_sleep() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System actually sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out

}


// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}
