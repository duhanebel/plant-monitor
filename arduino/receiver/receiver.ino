#include <RH_ASK.h>
#include <SPI.h>
#include <message.h>

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

void setup()
{
    // Initialize ASK Object
    rf_driver.init();
    // Setup Serial Monitor
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect
    }
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
      }
#ifdef DEBUG
       else {
           Serial.println("Invalid payload received!");
       }
#endif
    }
}
