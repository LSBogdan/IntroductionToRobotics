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

int redBrightness = 0;
int greenBrightness  = 0;
int blueBrightness = 0;

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

  redBrightness = map(redPotValue, potLowerBound, potUpperBound, ledLowerBound, ledUpperBound);
  greenBrightness = map(greenPotValue, potLowerBound, potUpperBound, ledLowerBound, ledUpperBound);
  blueBrightness =  map(bluePotValue, potLowerBound, potUpperBound, ledLowerBound, ledUpperBound);

  analogWrite(redPin, redBrightness);
  analogWrite(greenPin, greenBrightness);
  analogWrite(bluePin, blueBrightness);
  
}
