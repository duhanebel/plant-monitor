#!/usr/bin/env python3
import sys, getopt, os
import logging
from datetime import datetime
from influxdb import InfluxDBClient
from influxdb import SeriesHelper
import serial

class Measurement:
    plantID = "000"
    humidity = 0
    battery = 0

    def __init__(self, plantID, humidity, battery):
        self.plantID = plantID
        self.humidity = humidity
        self.battery = battery

def measurement_from_data(data):
    tokens = data.decode().split(';')
    id = "999"
    humidity = 0
    battery = 0
    if len(tokens) >= 3:
        token_id = tokens[0].split('=')
        token_hum = tokens[1].split('=')
        token_bat = tokens[2].split('=')
        if len(token_id) == 2 and len(token_hum) == 2 and len(token_bat) == 2:
            id = token_id[1]
            humidity = token_hum[1]
            battery = token_bat[1]
        else:
            return None
    else:
        return None
    return Measurement(id, humidity, battery)

def printUsage():
    print("")
    print("Usage: {} [OPTIONS]".format(__file__))
    print("  -s,--serial-port [/dev/ttyUSB0]        Serial port to listen on")
    print("  -i,--host [localhost]                  InfluxDB IP (default localhost)")
    print("  -p,--db-port [8060]                    InfluxDB port (default 8060)")
    print("  -u,--user [username]                   InfluxDB username")
    print("  -p,--pass [password]                   InfluxDB password")
    print("  -b,--db [db_name]                      InfluxDB database")
    print("  -h,--help                              Display this help and exit")
    print("")
    print("NOTE: Username and password can be passed via env variables as PLANTMON_USERNAME, PLANTMON_PASSWORD")
    print("")

# Default settings
host = ""
port = 8086
user = os.getenv('PLANTMON_USERNAME', "")
password = os.getenv('PLANTMON_PASSWORD', "")
dbname = ""
serialPort = ""

try:
    opts, args = getopt.getopt(sys.argv[1:],"s:i:p:u:p:b:h",["serial-port=","host=","db-port=","user=","pass=","db=","help"])
except getopt.GetoptError:
    print("{}: invalid option.".format(__file__))
    printUsage()
    sys.exit(-1)
for opt, arg in opts:
    if opt in ("-h", "--help"):
        printUsage()
        sys.exit()
    elif opt in ("-s", "--serial-port"):
        serialPort = arg
    elif opt in ("-i", "--host"):
        host = arg
    elif opt in ("-p", "--port"):
        port = arg
    elif opt in ("-u", "--user"):
        user = arg
    elif opt in ("-p", "--pass"):
        password = arg
    elif opt in ("-b", "--db"):
        dbname = arg

if not host:
    print("{}: missing hostname".format(__file__))
    printUsage()
    sys.exit(-1)
if(not user):
    print("{}: missing username".format(__file__))
    printUsage()
    sys.exit(-1)
if(not password):
    print("{}: missing password".format(__file__))
    printUsage()
    sys.exit(-1)
if(not dbname):
    print("{}: missing InfluxDB name".format(__file__))
    printUsage()
    sys.exit(-1)
if(not serialPort):
    print("{}: missing serial port".format(__file__))
    printUsage()
    sys.exit(-1)

logging.basicConfig(filename='/var/log/plant_monitor.log', filemode='w', level=logging.INFO)
logging.info("Started")

dbclient = InfluxDBClient(host, port, user, password, dbname)

class PlantSeries(SeriesHelper):
    class Meta:

        client = dbclient
        # The series name must be a string. Add dependent fields/tags
        # in curly brackets.
        series_name = 'soil'

        # Defines all the fields in this time series.
        fields = ['humidity', 'battery']

        # Defines all the tags for the series.
        tags = ['plant']

        # Defines the number of data points to store prior to writing
        # on the wire.
        bulk_size = 5

        # autocommit must be set to True when using bulk_size
        autocommit = True

try:
    ser = serial.Serial(serialPort, 9600)
except serial.SerialException as e:
    print("Could not open serial port '{}': {}".format(serialPort, e))
    sys.exit(-2)

ser.flushInput()

while True:
    ser_bytes = ser.readline()
    time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    logging.info("[{}] Received {}".format(time, ser_bytes))

    measurement = measurement_from_data(ser_bytes)
    if measurement == None:
        logging.warning("Invalid data received!")
    else:
        PlantSeries(plant=measurement.plantID, humidity=measurement.humidity, battery=measurement.battery)
        try:
            PlantSeries.commit()
        except Exception as e:
            logging.error("Error writing to database: %s', e)")
            sys.exit(-3)
