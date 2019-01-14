#include <RH_ASK.h>
#include <SPI.h>
#include <message.h>

#include <ArduinoLog.h>

// Pin to receive rf-radio data
#define RF_DATA_PIN 11

#define MAX_IDS 32

#define DEBUG

// Create Amplitude Shift Keying Object
RH_ASK rf_driver(2000,        // speed
                 RF_DATA_PIN, // rxPin
                 12);         // txPin (not in use)

void setup() {
  // Initialize ASK Object

  // Setup Serial Monitor
  Serial.begin(9600);
  while (!Serial)
    ; // wait for serial port to connect

#ifdef DEBUG
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.verbose("Debug mode: on");
#else
  Log.begin(LOG_LEVE_ERROR, &Serial);
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
    } else {
      Log.verbose("Discarded same-resendID message (%d)" CR, resendID);
    }
  }
}
