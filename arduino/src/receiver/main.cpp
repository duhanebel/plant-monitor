#include <RH_ASK.h>
#include <SPI.h>
#include <message.h>

#include <ArduinoLog.h>

#if defined(ESP8266)

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <InfluxDb.h>
// Wi-Fi settings
const char *ssid = "dune.iot";
const char *password = "4YFtCDoaHguiQtkmyypWjKcFt44gBnV)>>opojbj";

// Sensor settings
const uint8_t sensor_address = 0x76;

// Corlysis Setting - click to the database to get those info
#define INFLUXDB_HOST "nas.dune.uk"
#define INFLUXDB_PORT 8086
#define INFLUXDB_DATABASE "plants"
#define INFLUXDB_USER "root"
#define INFLUXDB_PASS "root"

HTTPClient http;

#endif

// Pin to receive rf-radio data
#define RF_DATA_PIN 12

#define MAX_IDS 128

//#define DEBUG

// Create Amplitude Shift Keying Object
RH_ASK rf_driver(2000,             // speed
                 RF_DATA_PIN,      // rxPin
                 RF_DATA_PIN + 1); // txPin (not in use)

#if defined(ESP8266)
void connect_wifi() {
  // Wi-Fi connection
  Serial.print("Connecting to the: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());
}

Influxdb influx(INFLUXDB_HOST, INFLUXDB_PORT);

void connect_to_influxdb() {
  influx.setDbAuth(INFLUXDB_DATABASE, INFLUXDB_USER, INFLUXDB_PASS);
}

void send_packet_to_influxdb(uint8_t senderID, Message *msg) {
  String series = "soil,plant=" + String(senderID) +
                  " battery=" + String(msg->battery) + "i" +
                  ",humidity=" + String(msg->value) + "i";
  // write it into db
  if (!influx.write(series)) {
    Log.verbose("Error sending");
  }
}
#endif

void setup() {
  // Initialize ASK Object

  // Setup Serial Monitor
  Serial.begin(9600);
  while (!Serial)
    ; // wait for serial port to connect
#if defined(ESP8266)
  connect_wifi();
  connect_to_influxdb();
#endif

#ifdef DEBUG
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.verbose("Debug mode: on");
#else
  Log.begin(LOG_LEVEL_ERROR, &Serial);
#endif

  if (!rf_driver.init()) {
    Log.fatal("RF init failed" CR);
  }
}

uint8_t prevResendIDs[MAX_IDS] = {255};
Message msg;

void dumpPacketToSerial(Print *serial, uint8_t senderID, Message *msg) {
  serial->print("id=");
  serial->print(senderID);
  serial->print(";hum=");
  serial->print(msg->value);
  serial->print(";batt=");
  serial->println(msg->battery);
}

void loop() {

  // Set buffer to size of expected message
  uint8_t buf[sizeof(Message)];
  uint8_t buflen = sizeof(buf);

  if (rf_driver.recv(buf, &buflen)) {
    if (buflen != sizeof(Message)) {
      Log.verbose("Wrong size message received (should be: %d but is %d",
                  sizeof(Message), buflen);
      return;
    }
    memcpy(&msg, buf, sizeof(Message));
    uint8_t senderID = rf_driver.headerFrom();
    uint8_t resendID = rf_driver.headerId();

    if (prevResendIDs[senderID] != resendID) {
      prevResendIDs[senderID] = resendID;
      dumpPacketToSerial(&Serial, senderID, &msg);
#if defined(ESP8266)
      send_packet_to_influxdb(senderID, &msg);
#endif
    } else {
      Log.verbose("Discarded same-resendID message (%d) from: %d" CR, resendID,
                  senderID);
    }
  }
}
