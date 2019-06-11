#!/bin/bash


function die() {
  echo $1
  exit -1
}

function show_help() {
  me=`basename "$0"`
  echo "This script sets the fuses on the attiny chip so that it runs at 8mhz on the internal clock and doesn't delete the EEPROM on upload"
  echo "Usage: $me -p [usbasp|usbtiny]"
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
    p)  programmer=$OPTARG
        ([ "$programmer" = "usbasp" ] || [ "$programmer" = "usbtiny" ]) || die "Invalid programmer \"$programmer\". Only usbasp and usbtiny are supported." 
        ;;
    esac
done

shift $((OPTIND-1))

[ "${1:-}" = "--" ] && shift

avrdude -pt85 -c $programmer -b 9600 -P usb -v -U lfuse:w:0xE2:m -U hfuse:w:0xD7:m 
