int outPin = 3;  // LED connected to digital pin 13
int ledPin = 13;
int inPin = 2;    // pushbutton connected to digital pin 7
int val = 0;      // variable to store the read value

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(inPin, INPUT);
  pinMode(outPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  val = digitalRead(inPin);   // read the input pin
  digitalWrite(ledPin, val);  // sets the LED to the button's value
  digitalWrite(outPin, val);  // sets the LED to the button's value
}
