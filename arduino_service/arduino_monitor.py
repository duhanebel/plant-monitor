import serial
from datetime import datetime
from influxdb import InfluxDBClient
from influxdb import SeriesHelper

# InfluxDB connections settings
host = 'nas.dune.uk'
port = 8086
user = 'root'
password = 'root'
dbname = 'plants'

dbclient = InfluxDBClient(host, port, user, password, dbname)

ser = serial.Serial('/dev/ttyACM0', 9600)
ser.flushInput()

class Measurement:
    plantID = "000"
    humidity = 0
    battery = 0

    def __init__(self, plantID, humidity, battery):
        self.plantID = plantID
        self.humidity = humidity
        self.battery = battery

def measurement_from_data(data):
    tokens = data.split(';')
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


while True:
    ser_bytes = ser.readline()
    time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print("[{}] Received {}".format(time, ser_bytes))

    measurement = measurement_from_data(ser_bytes)
    if measurement == None:
        print "Invalid data received"
    else:
        PlantSeries(plant=measurement.plantID, humidity=measurement.humidity, battery=measurement.battery)
        PlantSeries.commit()
