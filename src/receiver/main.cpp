#include <RH_ASK.h>
#include <SPI.h>
#include <message.h>

#include <ArduinoLog.h>

// Pin to receive rf-radio data
#define RF_DATA_PIN 11

#define MAX_IDS 32

//#define DEBUG

// Create Amplitude Shift Keying Object
RH_ASK rf_driver(2000, // speed
                 RF_DATA_PIN,   // rxPin 
                 12);  // txPin (not in use)

void setup()
{
    // Initialize ASK Object
    
    // Setup Serial Monitor
    Serial.begin(9600);
    while (!Serial); // wait for serial port to connect

#ifdef DEBUG
    Log.begin(LOG_LEVEL_VERBOSE, &Serial);
    Log.verbose("Debug mode: on");
else
    Log.begin(LOG_LEVE_ERROR, &Serial);
#endif

    if(!rf_driver.init()) {
      Log.fatal("RF init failed" CR);
    }
}

uint8_t prevResendIDs[MAX_IDS] = {0};
Payload payload;// = { binary: 0 };

void dumpPacketToSerial(Print *serial, Payload *payload) 
{
    uint8_t sender = payload->message.senderID;
    uint8_t hum = payload->message.data[0];
    uint8_t batt = payload->message.data[1];

    // Print values to Serial Monitor
    serial->print("id="); serial->print(sender);
    serial->print(";hum="); serial->print(hum);
    serial->print(";batt="); serial->println(batt);
}

void loop()
{
    // Set buffer to size of expected message
    uint8_t buf[3];
    uint8_t buflen = sizeof(buf);
    // Check if received packet is correct size
    if (rf_driver.recv(buf, &buflen)) {
        memcpy(payload.binary, buf, sizeof(payload.binary));

        if(!payload_validate(&payload)) return;

        if(payload.message.resendID != prevResendIDs[payload.message.senderID]) {
            Log.verbose("Received payload: %B, %B, %B" CR, payload.binary[0], payload.binary[1], payload.binary[2]);
            dumpPacketToSerial(&Serial, &payload);
            prevResendIDs[payload.message.senderID] = payload.message.resendID;
        }
        else {
            Log.verbose("Discarded same-resendID message" CR);
        }
    }
    else {
        Log.verbose("Invalid payload received!" CR);
    }
    
}
