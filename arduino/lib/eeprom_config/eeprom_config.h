#include <stdint.h>
#include <stdio.h>

#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#ifdef __AVR__
#include <Arduino.h>
#endif

/* EEPROM FORMAT
| Byte(s)     | Format | Description                     |
|-------------|--------|---------------------------------|
| B0          | uint8  | Signature (00111010b, 0x3A, 58) |
| B1, B2      | uint16 | Version (only 1 for now)        |
| B3,B4,B5,B6 | uint32 | Interval between readings (sec) |
| B7          | uint8  | Max number of resends           |
| B8          | uint8  | RF radio power pin              |
| B9          | uint8  | RF radio data pin               |
| B10,B11     | uint8  | Min soil value                  |
| B12,B13     | uint8  | Max soil value                  |
| B10         | uint8  | Sensor count                    |
| B11         | uint8  | Sensor #1 ID                    |
| B12         | uint8  | Sensor #1 data pin              |
| B13         | uint8  | Sensor #1 power pin             |
| B11         | uint8  | Sensor #2 ID                    |
| B12         | uint8  | Sensor #2 data pin              |
| B13         | uint8  | Sensor #2 power pin             |
| ...         | ...    | ... etc ...                     |

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
} Config;

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
