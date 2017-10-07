#include <SoftwareSerial.h>

#define HM10_PWR_PIN 4 //Used to turn HM-10 on and off, used to disconnect current clients
#define RELAY_PIN 3 //Used to control the relay
#define SWITCH_PIN 12 //Input for switch

// HM-10 module pins
#define STATE_PIN 7
#define RX_PIN 8
#define TX_PIN 9
SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

boolean lightState;

void setup() {
  Serial.begin(9600);
  while(!Serial) {
    ; // wait for serial port to connect.  Needed for native USB port only
  }

  mySerial.begin(9600);
  //Declare outputs
  pinMode(HM10_PWR_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  //Declare inputs
  pinMode(SWITCH_PIN, INPUT);
  pinMode(STATE_PIN, INPUT);
  
  digitalWrite(SWITCH_PIN, HIGH); //Enable internal pullup resistor, reads LOW when closed
  digitalWrite(HM10_PWR_PIN, HIGH);

}

void loop() {
  boolean switchState = digitalRead(SWITCH_PIN);

  relayState(lightState);
  
  //Interface serial outputs
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }

}




void changeLightState(boolean lightState){
  lightState != lightState;
}

void relayState(boolean lightState){
  digitalWrite(RELAY_PIN, lightState);
}

