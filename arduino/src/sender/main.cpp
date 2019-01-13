#include <stdio.h>

#include <RH_ASK.h>
#include <LowPower.h>
#include <Vcc.h>
#include <ArduinoLog.h>

#include <message.h>
#include <EEPROM.h>
#include <eeprom_config.h>

// // Number of sensors for this sender - each sensor counts as a separate device ID
// #define SENSORS_COUNT 2

// // Device IDs of each sensor
// uint8_t device_IDs[SENSORS_COUNT] = { 3, 4 };

// // Minimum value of soil moisture (to calculate persentage)
// #define SOIL_METER_DRY 0

// // Max value of soil moisture (to calculate percentage)
// #define SOIL_METER_WET 1024

// // Pins connected to the soil moisture probe
// uint8_t sensor_data_pins[SENSORS_COUNT] = { A0, A1 };

// // Pin to turn on the power to the rf-radio
// #define RF_POWER_PIN  4

// // Pin to send rf-radio data
// #define RF_DATA_PIN 12

// // Pins to turn on the power to sensors
// uint8_t sensor_power_pins[SENSORS_COUNT] = { 4, 5 };

// // Amount of resend for the same measurement (to mitigate concurrency between senders)
// #define MAX_RESEND 3

// // Seconds to wait between readings - in seconds (the system will go to deep sleep during the wait)
// #define INTERVAL_BETWEEN_READINGS (5*60)

// Uncomment to get some debug printed to serial
#define DEBUG

/////////////////////////////////////
// END CONF

// Structure to send to the receiver
Payload payload = payload_create();

uint8_t resendID = 0;

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

// Vcc reference value
Vcc vcc(1);

Config *config;

bool load_config_from_EEPROM(Config **config) {
  size_t count = EEPROM.read(offsetof(Config, sensor_count));
  Config *local_conf = config_create(count);
  if(local_conf == NULL) {
    Log.fatal("Not enough space for config");
  }

  Log.verbose("Going to read %d bytes from EEPROM" CR, config_sizeof(local_conf));
  for (int i=0;i<config_sizeof(local_conf);++i) {
    (*(((uint8_t *)local_conf) + i)) = EEPROM.read(i);
   }
  *config = local_conf; 
  return (config_validate(local_conf) == true);
}

void setup() {
 #ifdef DEBUG
  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.verbose("Debug mode: on");
 #endif

  if(!load_config_from_EEPROM(&config)) {
    Log.fatal("Invalid EEPROM config");
  } else {
#ifdef DEBUG 
    config_print(&Serial, config);
#endif
  }

  rf_driver = RH_ASK(2000, // speed
                 1,   // rxPin (not in use)
                 config->rf_data_pin,  // txPin
                 config->rf_power_pin); // RF_POWER_PIN
  rf_driver.init();

  for(int sensor=0;sensor<config->sensor_count;++sensor) {
    pinMode(config->sensors[sensor].power_pin, OUTPUT);
  }

  randomSeed(analogRead(2));
}

void activateSensors(uint8_t sensor, bool active) {
  if(active) {
    digitalWrite(sensor, HIGH);
  } else {
    digitalWrite(sensor, LOW);
  }
}

void sendData(uint8_t sensorID, uint8_t humidity, uint8_t battery, uint8_t retries) {
    payload.message.senderID = sensorID;

    payload.message.data[0] = humidity;
    payload.message.data[1] = battery;

    //activateSensors(RF_POWER_PIN, true);
    delay(100);
    rf_driver.setThisAddress(sensorID);
    rf_driver.setHeaderId(resendID++);
    for(int i=0;i<retries;++i) {

        rf_driver.send(payload.binary, sizeof(payload.binary));
        rf_driver.waitPacketSent();

        Log.verbose("ID: %d" CR, payload.message.senderID);
        Log.verbose(" Sending value: %d - %d" CR, payload.message.data[0], payload.message.data[1]);
        Log.verbose(" - count: %d" CR, payload.message.resendID);
        Log.verbose(" - raw payload: %B %B %B" CR, payload.binary[0], payload.binary[1], payload.binary[2]);

        delay(random(5, 100));
    }  
    //activateSensors(RF_POWER_PIN, false);
} 

void sleepFor(uint8_t seconds) {
    uint8_t sleeping_counter = 0;
    int sleep_intervals = seconds / 8;

    while(sleeping_counter++ <= sleep_intervals) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
    sleeping_counter = 0;
}

void loop()
{
    uint8_t batt = (int)vcc.Read_Perc(.2, 3.0, true);

    for(uint8_t sensor=0;sensor<config->sensor_count;++sensor) {
        uint8_t analog_pin = config->sensors[sensor].data_pin;
        uint8_t power_pin = config->sensors[sensor].power_pin;
        uint8_t sensorID = config->sensors[sensor].ID;

        activateSensors(power_pin, true);
        delay(200);
        int hum = analogRead(analog_pin);
        activateSensors(power_pin, false);

        uint8_t hum8 = map(hum, config->min_soil_value, config->min_soil_value, 0, 254);

        sendData(sensorID, hum8, batt, config->max_resends);

        Log.verbose("Battery value: %d" CR, batt);
        Log.verbose("Humidity: %d" CR, hum);
    }
    
    sleepFor(config->reading_interval);
}
