import serial
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

    def __init__(self, plantID, humidity):
        self.plantID = plantID
        self.humidity = humidity

def measurement_from_data(data):
    tokens = data.split(';')
    id = "999"
    humidity = 0
    if len(tokens) >= 2:
        token_id = tokens[0].split('=')
        token_hum = tokens[1].split('=')
        if len(token_id) == 2 and len(token_hum) == 2:
            id = token_id[1]
            humidity = token_hum[1]
    return Measurement(id, humidity)

def write_to_db(data):
    measurement = measurement_from_data(data)
    PlantSeries(plant=measurament.plantID, humidity=measurament.humidity)
    PlantSeries.commit()

class PlantSeries(SeriesHelper):
    class Meta:

        client = dbclient
        # The series name must be a string. Add dependent fields/tags
        # in curly brackets.
        series_name = 'soil'

        # Defines all the fields in this time series.
        fields = ['humidity']

        # Defines all the tags for the series.
        tags = ['plant']

        # Defines the number of data points to store prior to writing
        # on the wire.
        bulk_size = 5

        # autocommit must be set to True when using bulk_size
        autocommit = True


while True:
    ser_bytes = ser.readline()
    print(ser_bytes)

    measurement = measurement_from_data(data)
    PlantSeries(plant=measurament.plantID, humidity=measurament.humidity)
    PlantSeries.commit()
