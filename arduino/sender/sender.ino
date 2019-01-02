#include <RH_ASK.h>
#include <LowPower.h>
#include <Vcc.h>

#include <message.h>

// Number of sensors for this sender - each sensor counts as a separate device ID
#define SENSORS_COUNT 2

// Device IDs of each sensor
uint8_t device_IDs[SENSORS_COUNT] = { 3, 4 };

// Minimum value of soil moisture (to calculate persentage)
#define SOIL_METER_MIN 27

// Max value of soil moisture (to calculate percentage)
#define SOIL_METER_MAX 152

// Pins connected to the soil moisture probe
uint8_t sensor_data_pins[SENSORS_COUNT] = { A0, A1 };

// Pin to turn on the power to the rf-radio
#define RF_POWER_PIN  9

// Pin to send rf-radio data
#define RF_DATA_PIN 12

// Pins to turn on the power to sensors
uint8_t sensor_power_pins[SENSORS_COUNT] = { 4, 5 };

// Amount of resend for the same measurement (to mitigate concurrency between senders)
#define MAX_RESEND 3

// Seconds to wait between readings - in seconds (the system will go to deep sleep during the wait)
#define INTERVAL_BETWEEN_READINGS 5 //(5*60)

// Uncomment to get some debug printed to serial
#define DEBUG

/////////////////////////////////////
// END CONF

// Structure to send to the receiver
Payload payload = { msg: { senderID: 0, resendID: 0, reserved: 0, message: 0}};

// Create Amplitude Shift Keying Object
RH_ASK rf_driver(2000, // speed
                 11,   // rxPin (not in use)
                 RF_DATA_PIN);  // txPin

// Vcc reference value
Vcc vcc(1);

void setup() {
    rf_driver.init();
    pinMode(RF_POWER_PIN, OUTPUT);
    pinMode(RF_DATA_PIN, OUTPUT);

    for(int sensor=0;sensor<SENSORS_COUNT;++sensor) {
      pinMode(sensor_power_pins[sensor], OUTPUT);
    }

    randomSeed(analogRead(2));
    
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Debug mode: on");
#endif
}

int readSoil(uint8_t sensor)
{
    int val = analogRead(sensor);
    return val;
}

int activateSensors(uint8_t sensor, bool active) {
  if(active) {
    digitalWrite(sensor, HIGH);
    delay(100);
  } else {
    digitalWrite(sensor, LOW);
  }
}

//inline char humidity_percentage(int raw_value) {
//    if (raw_value < SOIL_METER_MIN ) return raw_value;
//    else return map(raw_value, SOIL_METER_MAX, SOIL_METER_MIN, 0, 100);
//}

void sendData(uint8_t sensorID, uint8_t humidity, uint8_t battery, uint8_t retries = MAX_RESEND) {
    payload.msg.senderID = sensorID;

    setMsg(&payload, MSG_LO, humidity);
    setMsg(&payload, MSG_HI, battery);

    activateSensors(RF_POWER_PIN, true);
    for(int i=0;i<retries;++i) {
        rf_driver.send(payload.binary, sizeof(payload.binary));
        rf_driver.waitPacketSent();

        delay(random(5, 100));

#ifdef DEBUG
        Serial.print("ID: "); Serial.print(payload.msg.senderID); 
        Serial.print(" Sending value: "); Serial.print(readMsg(payload.msg.message, MSG_LO)); Serial.print(" - "); Serial.print(readMsg(payload.msg.message, MSG_HI));
        Serial.print(" - count: "); Serial.println(payload.msg.resendID);
        Serial.print("Raw payload: ");Serial.print(payload.binary[0], BIN); Serial.print(" "); Serial.print(payload.binary[1], BIN); Serial.print(" "); Serial.println(payload.binary[2], BIN);
#endif
    }  
    payload.msg.resendID++;
    activateSensors(RF_POWER_PIN, false);
}

void sleepFor(uint8_t seconds) {
    uint8_t sleeping_counter = 0;
    int sleep_intervals = seconds / SLEEP_8S;

    while(sleeping_counter++ <= sleep_intervals) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
    sleeping_counter = 0;
}

void loop()
{

    int batt = (int)vcc.Read_Perc(.2, 3.0);
    uint8_t batt8 = constrain(batt, 0, 255);

    for(uint8_t sensor=0;sensor<SENSORS_COUNT;++sensor) {
        uint8_t analog_pin = sensor_data_pins[sensor];
        uint8_t power_pin = sensor_power_pins[sensor];
        uint8_t sensorID = device_IDs[sensor];

        activateSensors(power_pin, true);
        int hum = readSoil(analog_pin);
        activateSensors(power_pin, false);

        uint8_t hum8 = constrain(hum, 0, 255);

        sendData(sensorID, hum8, batt8);

#ifdef DEBUG
        Serial.print("Batt: "); Serial.println(batt);
        Serial.print("Hum: "); Serial.println(hum);
#endif
    }
    
    sleepFor(INTERVAL_BETWEEN_READINGS);
}
