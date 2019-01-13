#include <stdio.h>
#include <stdint.h>

#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#ifdef __AVR__
#include <Arduino.h>
#endif

/* EEPROM FORMAT 
 B0 - Signature 00111010
 B1, B2 - version (1)
 B3,B4,B5,B6 - interval between readings
 B7 - max resends
 B8 - RF power pin
 B9 - RF data pin
 B10,B11 - min soil value
 B12,B13 - max soil value
 B10 - Sensor count (n)
 B11, B11+n - sensor IDs
 B11+n, B11+2n - sensor data pins
 B11+2n, B11+3n - sensor power pins
| B0        | B0 | B1 | B3           | B4 ... BX  | BX+1 ... BY] | BY+1 ... BZ |
| SIGNATURE | VERSION | SENSOR COUNT | SENSOR IDS | DATA PINS    | POWER PINS  |
*/

typedef struct __attribute__((packed, aligned(1))) {
   uint8_t ID;
   uint8_t data_pin;
   uint8_t power_pin;
} Sensor;

typedef struct __attribute__((packed, aligned(1))) {
   uint8_t signature; // 3A = 58 = 00111010
   uint16_t version;
   uint32_t reading_interval;
   uint8_t max_resends;
   uint8_t rf_power_pin;
   uint8_t rf_data_pin;
   uint16_t min_soil_value;
   uint16_t max_soil_value;
   uint8_t sensor_count;
   Sensor sensors[];
} Config ;

size_t config_sizeof(Config *config);
Config *config_create(int sensor_count);
void config_delete(Config *config);

int config_validate(Config *config);

#ifdef __AVR__
void config_print(Print *output, Config *config);
#else
void config_print(FILE *output, Config *config);
#endif

#endif
