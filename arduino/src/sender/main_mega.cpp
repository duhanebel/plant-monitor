#include <stdio.h>

#include <ArduinoLog.h>
#include <LowPower.h>

#include <RH_ASK.h>
#include <Vcc.h>

#include <EEPROM.h>
#include <eeprom_config.h>
#include <message.h>

// Uncomment to get some debug printed to serial
//#define DEBUG

/////////////////////////////////////
// END CONF
// Structure to send to the receiver
Message msg;

uint8_t resendID = 0;

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

// Vcc reference value
Vcc vcc(1);

Config *config;

bool load_config_from_EEPROM(Config **config) {
  size_t count = EEPROM.read(offsetof(Config, sensor_count));
  Config *local_conf = config_create(count);
  if (local_conf == NULL) {
    Log.fatal("Not enough space for config");
  }

  Log.verbose("Going to read %d bytes from EEPROM" CR,
              config_sizeof(local_conf));
  for (int i = 0; i < config_sizeof(local_conf); ++i) {
    (*(((uint8_t *)local_conf) + i)) = EEPROM.read(i);
  }
  *config = local_conf;
  return (config_validate(local_conf) == true);
}

void setup() {
#if defined(DEBUG)
  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.verbose("Debug mode: on" CR);
#endif

  if (!load_config_from_EEPROM(&config)) {
    // Log.fatal("Invalid EEPROM config" CR);
  } else {
#if defined(DEBUG)
    config_print(&Serial, config);
#endif
  }

  rf_driver = RH_ASK(2000,                  // speed
                     1,                     // rxPin (not in use)
                     config->rf_data_pin,   // txPin
                     config->rf_power_pin); // RF_POWER_PIN
  rf_driver.init();

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
  msg.battery = battery;
  msg.value = humidity;

  rf_driver.setThisAddress(sensorID);
  rf_driver.setHeaderFrom(sensorID);
  rf_driver.setHeaderId(resendID++);
  for (int i = 0; i < retries; ++i) {
#if defined(DEBUG)
    Log.verbose("ID: %d" CR, sensorID);
    Log.verbose(" Sending value: %d - %d" CR, msg.battery, msg.value);
    Log.verbose(" - count: %d" CR, resendID);
#endif
    rf_driver.send((uint8_t *)&msg, sizeof(Message));
    rf_driver.waitPacketSent();

    delay(random(5, 50));
  }
}

uint8_t reduce_value(int val, int min, int max) {
  // Log.verbose("val: %d" CR, val);
  int c_min = min < max ? min : max;
  int c_max = min < max ? max : min;
  int capped_val = constrain(val, c_min, c_max);
#if defined(DEBUG)
  Log.verbose("capped_val: %d min: %d, max: %d" CR, capped_val, min, max);
  Log.verbose("final: %d" CR, map(capped_val, min, max, 0, 255));
#endif
  return map(capped_val, min, max, 0, 255);
}

void sleepFor(uint8_t seconds) {
  uint8_t sleeping_counter = 0;
  int sleep_intervals = seconds / 8;

  while (sleeping_counter++ <= sleep_intervals) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void loop() {
  uint8_t batt = (int)vcc.Read_Perc(.2, 3.0, true);

  for (uint8_t sensor = 0; sensor < config->sensor_count; ++sensor) {
    uint8_t analog_pin = config->sensors[sensor].data_pin;
    uint8_t power_pin = config->sensors[sensor].power_pin;
    uint8_t sensorID = config->sensors[sensor].ID;

    activateSensors(power_pin, true);
    delay(200);
    int hum = analogRead(analog_pin);
    activateSensors(power_pin, false);
    uint8_t hum8 =
        reduce_value(hum, config->min_soil_value, config->max_soil_value);

    sendData(sensorID, hum8, batt, config->max_resends);

#if defined(DEBUG)
    Log.verbose("Battery value: %d" CR, batt);
    Log.verbose("Humidity: %d" CR, hum);
#endif
  }
#ifdef DEBUG
  sleepFor(5);
#else
  sleepFor(config->reading_interval);
#endif
}
