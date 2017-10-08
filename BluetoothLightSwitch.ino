#include <SoftwareSerial.h>

#define HM10_PWR_PIN 4 //Used to turn HM-10 on and off, used to disconnect current clients
#define RELAY_PIN 3 //Used to control the relay
#define SWITCH_PIN 12 //Input for switch

// HM-10 module pins
#define STATE_PIN 7
#define RX_PIN 8
#define TX_PIN 9
SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

boolean lightState = false;
char PASSWORD[21] = "manmilk\r";

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false; //Variable for if a new line of data has been receive
boolean switchState; //Current State of button
boolean actuated = false; //Previous state of button

unsigned long currentMillis;
unsigned long previousMillis;
const int debounceInt = 3000;

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
  relayState();
  checkSwitch();
  //Interface serial outputs
  serialInterface();
  //Receive Data
  recvWithEndMarker();
  //Check Data for password
  checkForPassword();
}

//Interfaces USB serial to bluetooth serial
void serialInterface(){
  /*if (mySerial.available()) {
    Serial.write(mySerial.read());
  }*/
  if (Serial.available()) {
    mySerial.write(Serial.read()); 
  }
}

//Receives data with end of line marker
void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
 

  while (mySerial.available() > 0 && newData == false) {
    //Serial.write(mySerial.read());
    rc = mySerial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
    receivedChars[ndx] = '\0'; // terminate the string
    ndx = 0;
    newData = true;
    }
  }
}

//Checks if password matches received data, resets newData
void checkForPassword(){
  if (newData){
        Serial.println(receivedChars);
        Serial.print("Difference: ");
        Serial.println(strcmp(receivedChars, PASSWORD));
    if(strcmp(receivedChars, PASSWORD) == 0){
      changeLightState();
      mySerial.write("Correct Password\r\n");
    }
    else{
      mySerial.write("Incorrect Password\r\n");
    }
  }
  newData = false;
}

//Changes lightState when called
void changeLightState(){
  lightState = !lightState;
  //digitalWrite(13, lightState);
}

//Switch relay based on lightState
void relayState(){
  digitalWrite(RELAY_PIN, lightState);
}

void checkSwitch(){
  switchState = digitalRead(SWITCH_PIN);
  currentMillis = millis();
  if (!switchState){
    if(!actuated){
      previousMillis = millis();
    }
    if (currentMillis - previousMillis >= debounceInt){
      changeLightState();
      actuated = true;
    }
  }
  if (switchState){
    actuated = false;
  }
}

