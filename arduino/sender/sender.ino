
#include <RH_ASK.h>
#include <SPI.h>

#include <LowPower.h>
#include <Vcc.h>

#include <message.h>

#define DEVICE_ID 2

Payload payload = { msg: { senderID: DEVICE_ID, resendID: 0, reserved: 0, message: 0}};

// Create Amplitude Shift Keying Object
RH_ASK rf_driver(2000, 12, 11);

// Initialize DHT sensor for normal 16mhz Arduino

void setup() {
    // Initialize ASK Object
    rf_driver.init();
    // Start DHT Sensor

    Serial.begin(9600);
    randomSeed(analogRead(2));
}

#define SOIL_METER_MIN 27
#define SOIL_METER_MAX 152

#define SOIL_READ_PIN A0
#define SOIL_POWER_PIN 2

#define MAX_RESEND 3
#define DEBUG 
int readSoil()
{
    digitalWrite(SOIL_POWER_PIN, HIGH);
    delay(10);//wait 10 milliseconds 
    int val = analogRead(SOIL_READ_PIN);//Read the SIG value form sensor 
    digitalWrite(SOIL_POWER_PIN, LOW);//turn D7 "Off"

    return val;//send current moisture value

}

volatile bool  adcDone;
ISR(ADC_vect) { adcDone = true; }

static byte readVcc (byte count =4) {
  
  ADMUX = bit(REFS0) | 14; // use VCC and internal bandgap
  bitSet(ADCSRA, ADIE);
  while (count-- > 0) {
    adcDone = false;
    while (!adcDone)
      delay(15);
  }
  bitClear(ADCSRA, ADIE);  
  // convert ADC readings to fit in one byte, i.e. 20 mV steps:
  //  1.0V = 0, 1.8V = 40, 3.3V = 115, 5.0V = 200, 6.0V = 250
  return (55U * 1023U) / (ADC + 1) - 50;
}

inline char humidity_percentage(int raw_value) {
    if (raw_value < SOIL_METER_MIN ) return raw_value;
    else return map(raw_value, SOIL_METER_MAX, SOIL_METER_MIN, 0, 100);
}

Vcc vcc(1);
#define LOW_VALUE_MASK 0x00ff
#define HIGH_VALUE_MASK 0xff00
void setMsg_(Payload *payload, MsgType type, uint8_t value)
{
  Serial.print("Setting: "); Serial.println(value);
  switch(type) {
    case MSG_LO:
      Serial.print("L1 "); Serial.print(payload->binary[1], BIN); Serial.print(" "); Serial.println(payload->binary[2], BIN);
      payload->msg.message &= ~LOW_VALUE_MASK;
      Serial.print("L2 "); Serial.print(payload->binary[1], BIN); Serial.print(" "); Serial.println(payload->binary[2], BIN);
      payload->msg.message |= value;
      Serial.print("L3 "); Serial.print(payload->binary[1], BIN); Serial.print(" "); Serial.println(payload->binary[2], BIN);
      break;
    case MSG_HI:
      Serial.print("H1 "); Serial.print(payload->binary[1], BIN); Serial.print(" "); Serial.println(payload->binary[2], BIN);
      payload->msg.message &= ~HIGH_VALUE_MASK;
      Serial.print("H2 "); Serial.print(payload->binary[1], BIN); Serial.print(" "); Serial.println(payload->binary[2], BIN);
      payload->msg.message |= (value << 8);
      Serial.print("H3 "); Serial.print(payload->binary[1], BIN); Serial.print(" "); Serial.println(payload->binary[2], BIN);
      break;
  }
}

void loop()
{

    int hum = readSoil();  // Get Humidity value
    //Serial.print(hum);
    //payload.msg.message = constrain(hum, 0, 255); //humidity_percentage(hum);
    setMsg_(&payload, MSG_LO, constrain(hum, 0, 255));
    int batt = (int)vcc.Read_Perc(.2, 3.0);
    setMsg_(&payload, MSG_HI, constrain(batt, 0, 255));

    Serial.print("Batt: ");
    Serial.println(batt);
    Serial.print("Hum: ");
    Serial.println(hum);
    for(int i=0;i<MAX_RESEND;++i) {
#ifdef DEBUG

        Serial.print("Sending value: ");
        Serial.print(payload.msg.message);

        Serial.print(" - count: ");
        Serial.println(payload.msg.resendID);
        Serial.print(payload.binary[0], BIN);
        Serial.print(" ");
        Serial.print(payload.binary[1], BIN);
        Serial.print(" ");
        Serial.println(payload.binary[2], BIN);

#endif
        digitalWrite(SOIL_POWER_PIN, HIGH);
        delay(10);//wait 10 milliseconds 
        rf_driver.send(payload.binary, sizeof(payload.binary));
        rf_driver.waitPacketSent();
        digitalWrite(SOIL_POWER_PIN, LOW);//turn D7 "Off"
        
        delay(random(5, 100));
    } 
    payload.msg.resendID++;
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

}


