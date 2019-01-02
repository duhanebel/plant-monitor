#include <RH_ASK.h>
#include <SPI.h>
#include <message.h>

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
    while (!Serial) {
        ; // wait for serial port to connect
    }

    if(!rf_driver.init()) {
      Serial.println("RF init failed");
    }
  
#ifdef DEBUG
    Serial.println("Debug mode: on");
#endif

}

uint8_t prevResendIDs[MAX_IDS] = {0};
Payload payload;// = { binary: 0 };

void loop()
{
    // Set buffer to size of expected message
    uint8_t buf[3];
    uint8_t buflen = sizeof(buf);
    // Check if received packet is correct size
    if (rf_driver.recv(buf, &buflen)) {
      memcpy(payload.binary, buf, sizeof(payload.binary));

      if(validate(&payload)) {
          if(payload.msg.resendID != prevResendIDs[payload.msg.senderID]) {
 #ifdef DEBUG
              Serial.println(payload.binary[0], BIN);
              Serial.println(payload.binary[1], BIN);
              Serial.println(payload.binary[2], BIN);
 #endif
              uint8_t hum = readMsg(&payload, MSG_LO);
              uint8_t batt = readMsg(&payload, MSG_HI);
              // Print values to Serial Monitor
              Serial.print("id=");
              Serial.print(payload.msg.senderID);
 #ifdef DEBUG
              Serial.print(";resendID=");
              Serial.print(payload.msg.resendID);
 #endif

              Serial.print(";hum=");
              Serial.print(hum);
              Serial.print(";batt=");
              Serial.println(batt);

              prevResendIDs[payload.msg.senderID] = payload.msg.resendID;
          }
#ifdef DEBUG
            else {
              Serial.println("Discarded same-resendID message");
            }
#endif
      }
#ifdef DEBUG
       else {
           Serial.println("Invalid payload received!");
       }
#endif
    }
}
