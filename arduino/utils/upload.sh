#!/bin/bash


function die() {
  echo $1
  exit -1
}

function show_help() {
  me=`basename "$0"`
  echo "Usage: $me -c [attiny|atmega] -p [usbasp|usbtiny] [binary_file.hex]"
}
# A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.

# Initialize our own variables:

chip=""
programmer=""
verbose=""

while getopts "hvc:p:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    v)  verbose="-v"
        ;;
    c)  chip=$OPTARG
        ([ "$chip" = "attiny" ] || [ "$chip" = "atmega" ]) || die "Invalid chip \"$chip\". Only attiny and atmega are supported."
        ;;
    p)  programmer=$OPTARG
        ([ "$programmer" = "usbasp" ] || [ "$programmer" = "usbtiny" ]) || die "Invalid programmer \"$programmer\". Only usbasp and usbtiny are supported." 
        ;;
    esac
done

shift $((OPTIND-1))

[ "${1:-}" = "--" ] && shift
input_file="$1"

[ -f "$input_file" ] || die "File $input_file not found!"

[ "$chip" = "attiny" ] && chip="t85"
[ "$chip" = "atmega" ] && chip="m328p"

echo "Writing to EEPROM..."
avrdude -p$chip -c $programmer -b 9600 -P usb $verbose -Ueeprom:w:${input_file}:r
