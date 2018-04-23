FROM python:3

WORKDIR /usr/src/monitor
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY arduino_monitor.py ./


CMD [ "python", "-u", "/usr/local/bin/arduino_monitor.py"]
