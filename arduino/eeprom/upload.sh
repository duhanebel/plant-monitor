#!/bin/bash

hex_file="$1"
if [ -z "$hex_file" ]; then
   echo "Missing paramenter"
   echo "Usage: $0 [file_to_uploade]"
   exit -1
fi

if [ ! -f "${hex_file}" ]; then
  echo "File \"${hex_file}\" not found!"
  exit -2
fi

echo "Writing to EEPROM..."
avrdude -pm328p -c usbasp -b 9600 -P usb  -v -Ueeprom:w:${hex_file}:r
echo "Reading from EEPROM..."
