const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

const int redPotPin = A0;
const int greenPotPin = A1;
const int bluePotPin = A2;

const int potLowerBound = 0;
const int potUpperBound = 1023;
const int ledLowerBound = 0;
const int ledUpperBound = 255;

int redPotValue = 0;
int greenPotValue = 0;
int bluePotValue = 0;

int redLedBrightness = 0;
int greenLedBrightness  = 0;
int blueLedBrightness = 0;

void setup() {
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(9600);

}

void loop() {
  
  redPotValue = analogRead(redPotPin);
  greenPotValue = analogRead(greenPotPin);
  bluePotValue = analogRead(bluePotPin);

  redLedBrightness = map(redPotValue, potLowerBound, potUpperBound, ledLowerBound, ledUpperBound);
  greenLedBrightness = map(greenPotValue, potLowerBound, potUpperBound, ledLowerBound, ledUpperBound);
  blueLedBrightness =  map(bluePotValue, potLowerBound, potUpperBound, ledLowerBound, ledUpperBound);

  analogWrite(redPin, redLedBrightness);
  analogWrite(greenPin, greenLedBrightness);
  analogWrite(bluePin, blueLedBrightness);
  
}
