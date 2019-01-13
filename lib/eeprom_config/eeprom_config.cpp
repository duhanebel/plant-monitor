#include <stdlib.h>
#include "eeprom_config.h"


Config *config_create(int sensor_count) {
  Config *config = (Config *)malloc(sizeof(Config) + sizeof(Sensor[sensor_count]));
  config->signature = 0x3A;
  config->sensor_count = sensor_count;
  return config;
}

size_t config_sizeof(Config *config) {
  return (sizeof(Config) + sizeof(Sensor[config->sensor_count]));
}

void config_delete(Config *config) {
  free(config->sensors);
  free(config);
  config = NULL;
}

int config_validate(Config *config) {
  return (config->signature == 0x3A); // 3A = 58 = 00111010
}

#ifdef __AVR__
void config_print(Print *output, Config *config)
{
  output->print("Version:              "); output->println(config->version);
  output->print("Reading interval (s): "); output->println(config->reading_interval);
  output->print("Max resends:          "); output->println(config->max_resends);
  output->print("RF power pin:         "); output->println(config->rf_power_pin);
  output->print("RF data pin:          "); output->println(config->rf_data_pin);
  output->print("Min soil humidity:    "); output->println(config->min_soil_value);
  output->print("Max soil humidity:    "); output->println(config->max_soil_value);
  output->print("Number of sensors:    "); output->println(config->sensor_count);
  output->println("Sensors:");
  for(int i = 0;i<config->sensor_count;++i) {
    output->print("  #"); output->println(i);
    output->print("    Sensor ID:          "); output->println(config->sensors[i].ID);
    output->print("    Sensor data pin:    "); output->println(config->sensors[i].data_pin);
    output->print("    Sensor power pin:   "); output->println(config->sensors[i].power_pin);
  }
}
#else 
void config_print(FILE *output, Config *config)
{
  fprintf(output, "Version:              "); fprintf(output, "%d\n", config->version);
  fprintf(output, "Reading interval (s): "); fprintf(output, "%d\n", config->reading_interval);
  fprintf(output, "Max resends:          "); fprintf(output, "%d\n", config->max_resends);
  fprintf(output, "RF power pin:         "); fprintf(output, "%d\n", config->rf_power_pin);
  fprintf(output, "RF data pin:          "); fprintf(output, "%d\n", config->rf_data_pin);
  fprintf(output, "Min soil humidity:    "); fprintf(output, "%d\n", config->min_soil_value);
  fprintf(output, "Max soil humidity:    "); fprintf(output, "%d\n", config->max_soil_value);
  fprintf(output, "Number of sensors:    "); fprintf(output, "%d\n", config->sensor_count);
  fprintf(output, "Sensors:");
  for(int i = 0;i<config->sensor_count;++i) {
    fprintf(output, "  #"); fprintf(output, "%d\n", i);
    fprintf(output, "    Sensor ID:          "); fprintf(output, "%d\n", config->sensors[i].ID);
    fprintf(output, "    Sensor data pin:    "); fprintf(output, "%d\n", config->sensors[i].data_pin);
    fprintf(output, "    Sensor power pin:   "); fprintf(output, "%d\n", config->sensors[i].power_pin);
  }
}
#endif