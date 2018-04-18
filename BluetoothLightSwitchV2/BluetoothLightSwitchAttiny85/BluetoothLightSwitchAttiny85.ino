#include <SoftwareSerial.h>

#define KEY_PIN 5 //Used for Reseting HM-11
#define RELAY_PIN 4 //Used to control the relay
#define SWITCH_PIN 3 //Input for switch

// HM-10 module pins
#define STATE_PIN 0
#define RX_PIN 2
#define TX_PIN 1
SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

boolean lightState = false;
char password[21] = "PWD1\r";
char debugPWD[21] = "DEBUG1\r";

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false; //Variable for if a new line of data has been receive
boolean switchState; //Current State of button
boolean actuated = false; //True when timer has been set
boolean buttonDown = false; //Used to determine if button has been debounced
boolean disconnectInit = false; //If HM-10 power cycle has been initiated
boolean debug_Mode = false; //Used to keep track of if in debug mode.  Debug mode doesn't automatically disconnect
boolean previousBluetoothState; //Used to keep track of if HM-10 previously had a connection
boolean connectionTimerSet = false; //Used to determine if connection timer has been set
boolean disconnectTimerSet = false; //Used to determine if the disconnect timer has been set

unsigned long previousMillis; //Used for debouncing switch
unsigned long connectionTimer; //Used to wait for connection to give status

const int debounceInt = 3; //Debounce interval for switch
const int powerCycleTime = 150; //Time (ms) that HM-10 is turned off for in order to remove power
const int connectionTime = 200; //Wait interval before sending status upon connection
const int disconnectTime = 200; //Wait interval before disconnection

void setup() {

  mySerial.begin(9600);
  //Declare outputs
  pinMode(KEY_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  //Declare inputs
  pinMode(SWITCH_PIN, INPUT);
  pinMode(STATE_PIN, INPUT);
  
  //Enable internal pullup resistor, reads LOW when closed
  digitalWrite(SWITCH_PIN, HIGH);
  digitalWrite(STATE_PIN, HIGH);
  //Resets when low
  digitalWrite(KEY_PIN, HIGH);

}

void loop() {
  relayState(); //Update relay state
  
  checkSwitch(); //Check to see if switch has been actuated

  checkConnection(); //Checks for bluetooth connection or disconnection
  
  recvWithEndMarker(); //Receive Data
  
  checkForPassword(); //Check Data for password
  
  bluetoothDisconnect(); //Check to see if bluetooth needs to be disconnected
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
void checkForPassword() {
  if (newData) {
    if (strcmp(receivedChars, password) == 0) {
      changeLightState();
      if (lightState) {
        mySerial.write("Light Status: On\r\n");
      }
      else if (!lightState) {
        mySerial.write("Light Status: Off\r\n");
      }
      if (!debug_Mode) {
        mySerial.write("Disconnecting");
        disconnectInit = true;
      }
    }
    else if (strcmp(receivedChars, debugPWD) == 0) {
      debug_Mode = !debug_Mode;
      if (debug_Mode) {
        mySerial.write("Entering debug mode\r\n");
      }
      else if (!debug_Mode) {
        mySerial.write("Exiting debug mode\r\n");
      }
    }
    else{
      mySerial.write("Incorrect Password\r\n");
    }
  }
  newData = false;
}


void bluetoothDisconnect() {
  if (disconnectInit) {
    if(digitalRead(STATE_PIN)) {
       digitalWrite(KEY_PIN, LOW);
    }
    else {
      digitalWrite(KEY_PIN, HIGH);
      disconnectInit = false;
    }
  }
}

//Changes lightState when called
void changeLightState() {
  lightState = !lightState;
}

//Switch relay based on lightState
void relayState() {
  digitalWrite(RELAY_PIN, lightState);
}

void checkSwitch() {
  switchState = digitalRead(SWITCH_PIN);
  if (!switchState) {
    if (!actuated) {
      previousMillis = millis();
      actuated = true;
    }
    if ((millis() - previousMillis >= debounceInt) && actuated && !buttonDown) {
      changeLightState();
      buttonDown = true;
    }
  }
  if (switchState && (millis() - previousMillis >= debounceInt)) {
    actuated = false;
    buttonDown = false;
  }
}

//Checks for new connection, sends light status upon new connection
//Disables debug_mode upon disconnection
void checkConnection() {
  if(previousBluetoothState && !digitalRead(STATE_PIN)){
    debug_Mode = false; //Reset debug mode if client disconnects
  }
  else if(!previousBluetoothState && digitalRead(STATE_PIN)) {
    connectionTimer = millis();
    connectionTimerSet = true;
  }
  if (connectionTimerSet && ((millis() - connectionTimer) >= connectionTime)){
      connectionTimerSet = false;
      if (lightState) {
        mySerial.write("Light Status: On\r\n");
      }
      else {
        mySerial.write("Light Status: Off\r\n");
      }
    }
  previousBluetoothState = digitalRead(STATE_PIN);
}
