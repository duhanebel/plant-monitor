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

uint8_t prevResendID = 0;

Payload payload = { binary: 0 };

void loop()
{
    // Set buffer to size of expected message
    uint8_t buf[2];
    uint8_t buflen = sizeof(buf);
    // Check if received packet is correct size
    if (rf_driver.recv(buf, &buflen)) {
      payload.binary = buf;
      if(validate(&payload)) {
          if(payload.msg.resendID != prevResendID) {
              // Print values to Serial Monitor
              Serial.print("id=");
              Serial.print(payload.msg.senderID);
              Serial.print(";hum=");
              Serial.println(payload.msg.message);

              prevResendID = payload.msg.resendID;
          }
      }
#ifdef DEBUG
       else {
           Serial.println("Invalid payload received!");
       }
#endif
    }
}
