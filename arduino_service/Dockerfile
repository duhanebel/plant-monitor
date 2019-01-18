FROM python:3

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY arduino_monitor.py /usr/local/bin
RUN chmod 755 "/usr/local/bin/arduino_monitor.py"

ENV PLANTMON_PASSWORD=$DB_PASSWORD
ENV PLANTMON_USERNAME=$DB_USERNAME
CMD python /usr/local/bin/arduino_monitor.py -s $SERIAL_PORT -i $DB_ADDRESS -p $DB_PORT -b $DB_NAME

