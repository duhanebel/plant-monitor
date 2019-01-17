// struct to file
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "eeprom_config.h"

#define READ_INT_NO_DEFAULT -1
int read_int(char *question, int default_value) {
  int value;
  char buf[10];
  char *end;
  do {
    printf("%s", question);
    if (default_value != READ_INT_NO_DEFAULT)
      printf("[%d] ", default_value);

    fgets(buf, sizeof(buf), stdin);
    if (buf[0] == '\n' && default_value != READ_INT_NO_DEFAULT) {
      return default_value;
    }
    value = strtoul(buf, &end, 10);
  } while (buf == end);

  return value;
}

// void swap_bytes(void *buf, int from_idx, int to_idx) {
//     char *a = (buf + from_idx);
//     char *b = (buf + to_idx);
//     a = *a | *b;
//     b = *a | *b;
//     a = *a | *b;
// }

//   uint8_t *int_buf = buf;
//   uint8_t temp = int_buf[from_idx];
//   int_buf[from_idx] = int_buf[to_idx];
//   int_buf[to_idx] = temp;
// }

// void reverse_endian(void *buff, size_t buff_size) {
//   for(int i=0; i<(buff_size/2); ++i) {
//     swap_bytes(buff, i, (buff_size-1)-i);
//   }
// }

#define MODE_UNDEFINED -1
#define MODE_READ 0
#define MODE_WRITE 1

int write_to(char *file_path) {

  int sensor_count = read_int("How many sensors? ", READ_INT_NO_DEFAULT);
  Config *config = config_create(sensor_count);

  config->version = 1;
  config->reading_interval = read_int("Reading interval in seconds? ", 300);
  config->max_resends = read_int("Max resends? ", 3);
  config->rf_power_pin = read_int("RF power pin? ", 9);
  config->rf_data_pin = read_int("RF data pin? ", 10);
  config->min_soil_value = read_int("Min humidity value? ", 846);
  config->max_soil_value = read_int("Max humidity value? ", 506);
  for (int i = 0; i < sensor_count; ++i) {
    char question[100];
    sprintf(question, "[SENSOR %d] Sensor ID? ", i + 1);
    config->sensors[i].ID = read_int(question, READ_INT_NO_DEFAULT);
    sprintf(question, "[SENSOR %d] Sensor data pin? (A0 = 14, A1 = 15, etc.) ",
            i + 1);
    config->sensors[i].data_pin = read_int(question, READ_INT_NO_DEFAULT);
    sprintf(question, "[SENSOR %d] Sensor power pin? ", i + 1);
    config->sensors[i].power_pin = read_int(question, READ_INT_NO_DEFAULT);
    printf("\n");
  }

  FILE *outfile;

  outfile = fopen(file_path, "wb");
  if (outfile == NULL)
    return -1;

  if (fwrite(config, config_sizeof(config), 1, outfile) == 0) {
    fclose(outfile);
    return -3;
  }

  fclose(outfile);
  return 0;
}

int read_from(char *file_path) {

  FILE *file = fopen(file_path, "rb");
  if (file == NULL)
    return -1;

  fseek(file, 0, SEEK_END);
  unsigned long file_len = ftell(file);

  if (file_len > 4096) {
    fclose(file);
    return -2;
  } else if (file_len < 1) {
    fclose(file);
    return -1;
  }

  fseek(file, 0, SEEK_SET);
  printf("Reading %lu bytes from \"%s\"...\n", file_len, file_path);

  Config *config = (Config *)malloc(file_len + 1);
  size_t read = fread(config, file_len, 1, file);
  fclose(file);
  if (read < 1) {
    return -4;
  }

  config_print(stdout, config);
  free(config);
  return 0;
}

int main(int argc, char **argv) {
  int mode = MODE_UNDEFINED;
  char *file_path = NULL;
  int c;

  opterr = 0;

  while ((c = getopt(argc, argv, "w:r:h")) != -1) {
    switch (c) {
    case 'r':
      mode = MODE_READ;
      file_path = optarg;
      break;
    case 'w':
      mode = MODE_WRITE;
      file_path = optarg;
      break;
    case 'h':
      printf("Usage: %s [OPTIONS]\n", argv[0]);
      printf("  -w [filepath]         write configuration to file\n");
      printf("  -r [filepath]         read configuration from file\n");
      printf("  -h                    print this help and exit\n");
      printf("\n");
      return (0);
      break;
    case ':':
    case '?':
      if (optopt == 'w' || optopt == 'r')
        fprintf(stderr, "%s: option -%c requires a filepath.\n", argv[0],
                optopt);
      else if (isprint(optopt))
        fprintf(stderr, "%s: unknown option -- %c.\n", argv[0], optopt);
      else
        fprintf(stderr, "%s: invalid option -- %c.\n", argv[0], optopt);
      fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
      return (-2);
      break;
    default:
      fprintf(stderr, "%s: invalid option -- %c\n", argv[0], optopt);
      fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
      return (-2);
    }
  }

  int result = 0;
  if (mode == MODE_READ) {
    result = read_from(file_path);
  } else if (mode == MODE_WRITE) {
    result = write_to(file_path);
  } else {
    fprintf(stderr, "%s: invalid option\n", argv[0]);
    fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
    result = -255;
  }

  switch (result) {
  case 0:
    printf("\nDone!\n");
    break;
  case -1:
    fprintf(stderr, "%s: Error opening file: %s\n", argv[0], file_path);
    break;
  case -2:
    fprintf(stderr, "%s: the file is too big!\n", argv[0]);
    break;
  case -3:
    fprintf(stderr, "%s: error writing to file: %s\n", argv[0], file_path);
    break;
  case -4:
    fprintf(stderr, "%s: error reading from file: %s\n", argv[0], file_path);
    break;
  default:
    return -255;
  }
  return result;
}
