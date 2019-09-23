#include <Wire.h>

int outPin = 3;  // LED connected to digital pin 13
int ledPin = 13;

// the setup function runs once when you press reset or power the board
void setup() {
  Wire.begin(0x8);              // join i2c bus (address optional for master)
  Wire.onReceive(receiveEvent); //Activate the Receive Event 
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(outPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
}

// this function is registered as an event, see setup()
void receiveEvent(int howMany){
  while (Wire.available()) { // loop through all but the last
    uint8_t c = Wire.read(); // receive byte as a character
    digitalWrite(ledPin, HIGH);
    digitalWrite(outPin, HIGH);
    delay(35);
    digitalWrite(ledPin, LOW);
    digitalWrite(outPin, LOW);
  }
}
