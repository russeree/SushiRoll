#include <Wire.h>

int outPin = 3;  // LED connected to digital pin 13
int ledPin = 13;

// the setup function runs once when you press reset or power the board
void setup() {
  Wire.begin(0x8);        // join i2c bus (address optional for master)
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(outPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
}
