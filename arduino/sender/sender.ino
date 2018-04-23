#include <DHT.h>
#define DHTTYPE DHT22   // DHT Type is DHT 22 (AM2302)

#include <RH_ASK.h>
#include <SPI.h>

#include <message.h>

#define DEVICE_ID 1
#define DHTPIN 2       // DHT-22 Output Pin connection

Payload payload = { msg: { senderID: DEVICE_ID, resendID: 0, reserved: 0, message: 0}};

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    // Initialize ASK Object
    rf_driver.init();
    // Start DHT Sensor
    dht.begin();
    Serial.begin(9600);
    randomSeed(analogRead(0));
}

#define SOIL_METER_MIN 0
#define SOIL_METER_MAX 465

#define MAX_RESEND 5

inline char humidity_percentage(int raw_value) {
    return ((raw_value - SOIL_METER_MIN) * 100) / (SOIL_METER_MAX - SOIL_METER_MIN);
}

void loop()
{
    delay(5000);  // Delay so DHT-22 sensor can stabalize

    float hum = dht.readHumidity();  // Get Humidity value
    payload.msg.message = humidity_percentage((int)hum);

#ifdef DEBUG
    Serial.println("Sending value: %d", (int)hum);
#endif
    payload.msg.resendID = 0;
    do {
        rf_driver.send(payload.binary, sizeof(payload.binary));
        rf_driver.waitPacketSent();
        delay(random(100, 5000));
    } while(++payload.msg.resendID < MAX_RESEND);
}

