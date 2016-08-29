
#include "LowPower.h"
#include <SoftwareSerial.h>//rx and tx pin
#include <NewPing.h>
SoftwareSerial RadioSerial(2, 3);//RX,TX

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11 // Arduino pin tied to echo pin on the ultrasonic sensor.

#define ONE_PIN_ENABLED false
#define ROUNDING_ENABLED true
#define distance 500  //Distance (centimeters) from bottom of the tank to the sensor
#define uStoMeter 57      // Microseconds (uS) it takes sound to travel round-trip 1m (2m total), uses integer to save compiled code space. Default=5700

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_SENSOR_DISTANCE); // NewPing setup of pins and maximum distance.

const int wakeUpPin = 2; // Use pin 2 as wake up pin

unsigned long prevMillis;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin (9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  while (!Serial)
  {
    //wait for serial port to connect
  }
  Serial.println ("serial coms open");

  RadioSerial.begin(9600);//set the data rate for SERIALPINS port
  prevMillis = millis();
}

// Read water depth, transmit for 5s, sleep for 15mins
void loop() {

  unsigned long crtMillis = millis();
  prevMillis = crtMillis;

  int WaterDepth = distance - sonar.ping_cm();
  Serial.print(WaterDepth);
  Serial.println(" cm");
  char data[3] = {0};
  //Transmit water depth for 5s
  while (abs(crtMillis - prevMillis) < 600) {
    
    data[0] = 0xFB; //Callsign 1
    data[1] = WaterDepth >> 8;      // Average RMS Voltage Most Significant Bits (MSB)
    data[2] = 0xFF & WaterDepth;    // Average RMS Voltage Least Significant Bits (LSB)
    
    
    String toSend("");
    //Serial.println("Bytes Transmitted: " + String(x));
    for (int i = 0; i<3; i++) {
        Serial.print((byte)data[i]);
        Serial.print(", ");
        toSend+=data[i];
    }
    Serial.println("");
    int x = RadioSerial.print(toSend);
    Serial.println("Bytes Transmitted: " + String(x));
    
    delay(1000);
    crtMillis = millis();
  }
  Serial.println("Starting Sleeping");
  Serial.flush();
  //sleep for 352 seconds, almost 15 minutes
  for (int i = 0; i<1; i++) { //Change to 44 in order to make a 15 minute delay
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  //Sleep 8 seconds
  }
  
  Serial.println("Stopping Sleping.");
}




