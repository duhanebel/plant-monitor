#include <arduino.h>
#include <avr/power.h>
#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/wdt.h>

#include "tiny_sleep.h"

typedef enum {
  SLEEP_16_MS = 0,
  SLEEP_32_MS = 1,
  SLEEP_64_MS = 2,
  SLEEP_128_MS = 3,
  SLEEP_250_MS = 4,
  SLEEP_500_MS = 5,
  SLEEP_1_S = 6,
  SLEEP_2_S = 7,
  SLEEP_4_S = 8,
  SLEEP_8_S = 9
} Sleep_interval;

void setup_watchdog(Sleep_interval interval);
void sleep_for(Sleep_interval interval);
void enable_adc();
void disable_adc();

void sleep_for_seconds(int seconds) {
  disable_adc();
  power_all_disable();
  int count = seconds / 8;
  int remaining = seconds % 8;

  while (count-- > 0) {
    sleep_for(SLEEP_8_S);
  }

  if (remaining / 4 != 0) {
    remaining %= 4;
    sleep_for(SLEEP_4_S);
  }

  while (remaining-- > 0) {
    sleep_for(SLEEP_1_S);
  }
  power_all_enable();
  enable_adc();
}

void random_short_sleep() {
  Sleep_interval interval = (Sleep_interval)random(SLEEP_16_MS, SLEEP_250_MS);
  disable_adc();
  power_all_disable();
  sleep_for(interval);
  power_all_enable();
  enable_adc();
}

void sleep_for(Sleep_interval interval) {
  setup_watchdog(interval); // Setup watchdog to go off after 1sec
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode(); // Go to sleep! Wake up 1sec later and check water
  sleep_disable();
}

void enable_adc() {
  (_SFR_BYTE(ADCSRA) |= _BV(ADEN));
  // ADCSRA |= (1 << ADEN); // Enable ADC
}

void disable_adc() {
  (_SFR_BYTE(ADCSRA) &= ~_BV(ADEN));
  // ADCSRA &= ~(1 << ADEN); // Disable ADC, saves ~230uA
}

ISR(WDT_vect) {
  // Don't do anything. This is just here so that we wake up.
}

void setup_watchdog(Sleep_interval interval) {

  if (interval > SLEEP_8_S)
    interval = SLEEP_8_S; // Limit incoming amount to legal settings
  else if (interval < SLEEP_16_MS)
    interval = SLEEP_16_MS;

  byte bb = interval & 7;
  if (interval > 7)
    bb |= (1 << 5); // Set the special 5th bit if necessary

  // This order of commands is important and cannot be combined
  MCUSR &= ~(1 << WDRF);             // Clear the watch dog reset
  WDTCR |= (1 << WDCE) | (1 << WDE); // Set WD_change enable, set WD enable
  WDTCR = bb;                        // Set new watchdog timeout value
  WDTCR |= _BV(WDIE); // Set the interrupt enable, this will keep unit from
                      // resetting after each int
}