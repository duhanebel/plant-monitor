#include <stdio.h>

#include <Vcc.h>

#include <EEPROM.h>
#include <eeprom_config.h>
#include <message.h>

#include "tiny_sleep.h"

#include <util/delay.h>

#include <VirtualWire.h>

typedef struct {
  uint8_t dest;
  uint8_t sender;
  uint8_t id;
  uint8_t flags;
  Message message;
} RHMessageWrapper;

// Message to send
RHMessageWrapper rhmsg = {0xFF, 0, 0, 0, 0, 0};

// Vcc reference value
Vcc vcc(1);

// EEPROM configuration
Config *config;

bool load_config_from_EEPROM(Config **config) {
  size_t count = EEPROM.read(offsetof(Config, sensor_count));
  Config *local_conf = config_create(count);

  for (int i = 0; i < config_sizeof(local_conf); ++i) {
    (*(((uint8_t *)local_conf) + i)) = EEPROM.read(i);
  }
  local_conf->rf_power_pin = 0;
  *config = local_conf;

  return (config_validate(local_conf) == true);
}

void setup() {
  load_config_from_EEPROM(&config);

  // vw_set_ptt_inverted(true);
  vw_set_tx_pin(config->rf_data_pin);
  vw_setup(2000);

  pinMode(config->rf_power_pin, OUTPUT);
  for (int sensor = 0; sensor < config->sensor_count; ++sensor) {
    pinMode(config->sensors[sensor].power_pin, OUTPUT);
  }

  randomSeed(analogRead(2));
}

void activateSensors(uint8_t sensor, bool active) {
  if (active) {
    digitalWrite(sensor, HIGH);
  } else {
    digitalWrite(sensor, LOW);
  }
}

void sendData(uint8_t sensorID, uint8_t humidity, uint8_t battery,
              uint8_t retries) {
  rhmsg.sender = sensorID;
  rhmsg.message.battery = battery;
  rhmsg.message.value = humidity;
  rhmsg.id++;

  for (int i = 0; i < retries; ++i) {
    activateSensors(config->rf_power_pin, true);
    vw_send((uint8_t *)&rhmsg, sizeof(RHMessageWrapper));
    vw_wait_tx();
    activateSensors(config->rf_power_pin, false);
    random_short_sleep();
  }
}

uint8_t reduce_value(int val, int min, int max) {
  int c_min = min < max ? min : max;
  int c_max = min < max ? max : min;
  int capped_val = constrain(val, c_min, c_max);
  return map(capped_val, min, max, 0, 255);
}

void loop() {
  uint8_t batt = (int)vcc.Read_Perc(.2, 3.0, true);

  for (uint8_t sensor = 0; sensor < config->sensor_count; ++sensor) {
    uint8_t analog_pin = config->sensors[sensor].data_pin;
    uint8_t power_pin = config->sensors[sensor].power_pin;
    uint8_t sensorID = config->sensors[sensor].ID;

    activateSensors(power_pin, true);
    _delay_ms(20);
    int hum = analogRead(analog_pin);
    activateSensors(power_pin, false);
    uint8_t hum8 =
        reduce_value(hum, config->min_soil_value, config->max_soil_value);

    sendData(sensorID, hum8, batt, config->max_resends);
  }
#ifdef DEBUG
  sleep_for_seconds(5);
#else
  sleep_for_seconds(config->reading_interval);
#endif
}
