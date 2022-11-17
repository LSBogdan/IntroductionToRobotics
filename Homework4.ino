const int latchPin = 11;
const int clockPin = 10;  
const int dataPin = 12;   

const int segD1 = 7;
const int segD2 = 6;
const int segD3 = 5;
const int segD4 = 4;

const int regSize = 8;

const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;

const int encodingsNumber = 16;

byte byteEncodings[encodingsNumber] = {
  //A B C D E F G DP
  B11111100,  // 0
  B01100000,  // 1
  B11011010,  // 2
  B11110010,  // 3
  B01100110,  // 4
  B10110110,  // 5
  B10111110,  // 6
  B11100000,  // 7
  B11111110,  // 8
  B11110110,  // 9
  B11101110,  // A
  B00111110,  // b
  B10011100,  // C
  B01111010,  // d
  B10011110,  // E
  B10001110   // F
};

byte displayDigits[] = { segD1, segD2, segD3, segD4 };

const int displayCount = 4;

byte dpState[] = { 0, 0, 0, 0 };

int displayValue[] = { 0, 0, 0, 0 };

const int systemStateUnlocked = 1;
const int systemStateLocked = 2;


const int noPressButtonState = 0;
const int longPressButtonState = 1;
const int shortPressButtonState = 2;

const int up = 1;
const int right = 1;
const int down = -1;
const int left = -1;

const int minThreshold = 200;
const int maxThreshold = 800;
const int safeMinThreshold = 400;
const int safeMaxThreshold = 600;

unsigned long lastBlinkTime = 0;
unsigned long pressTime = 0;
unsigned long lastDebounceTime;

const int debounceDelay = 50;
const int blinkingTime = 500;
const int longPressButton = 3500;

int xValue = 512;
int yValue = 512;
int stateJoystickX = 0;
int stateJoystickY = 0;
bool joystickMoved = false;

int currentDisplayIndex = 0;

byte buttonState = HIGH;
byte reading = HIGH;
byte lastReading = HIGH;
int buttonPressState = 0;

int currentState = 1;

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);  
  pinMode(dataPin, OUTPUT);   

  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }
  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(9600);
}

void writeReg(byte encoding) { 

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, encoding);
  digitalWrite(latchPin, HIGH);
}

void activateDisplay(int digit) {

  for(int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }

  digitalWrite(displayDigits[digit], LOW);
}

void writeAllDigits() {

  for(int i = 0; i < displayCount; i++) {
    writeReg(byteEncodings[displayValue[i]] ^ dpState[i]);
    activateDisplay(i);
    delay(5);
  }
  digitalWrite(displayDigits[displayCount - 1], HIGH);
}

void blinkDPLed() {

  if(millis() - lastBlinkTime > blinkingTime) {
    lastBlinkTime = millis();
    dpState[currentDisplayIndex] = !dpState[currentDisplayIndex];
  }
}

int getButtonPressType() {

  reading = digitalRead(pinSW);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    if (buttonState != reading) {
      buttonState = reading;

      if (buttonState == LOW){
        pressTime = millis();
      
      } else if (millis() - pressTime < longPressButton) {
        return shortPressButtonState;
      }
    } else if (buttonState == LOW && millis() - pressTime >= longPressButton) { 
        return longPressButtonState;
    }
  }

  lastReading = reading;
  return noPressButtonState;
}

void reset() {

  currentState = systemStateUnlocked;
  currentDisplayIndex = 0;

  for (int i = 0; i < displayCount; i++) {
    displayValue[i] = 0;
    dpState[i] = 0;
  }
}

void getJoystickState() {

  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  stateJoystickX = 0;
  stateJoystickY = 0;

    if (safeMinThreshold <= xValue && xValue <= safeMaxThreshold && safeMinThreshold <= yValue && yValue <= safeMaxThreshold) {
    joystickMoved = false;
  }

  if (xValue > maxThreshold && !joystickMoved) {
    stateJoystickX = 1;
    joystickMoved = true;
  } else if (xValue < minThreshold && !joystickMoved) {
    stateJoystickX = -1;
    joystickMoved = true;
  }

  if (yValue > maxThreshold && !joystickMoved) {
    stateJoystickY = 1;
    joystickMoved = true;
  } else if (yValue < minThreshold && !joystickMoved) {
    stateJoystickY = -1;
    joystickMoved = true;
  }
}

int goToNextDisplay() {

  getJoystickState();

  int nextDisplay = currentDisplayIndex;

  if (stateJoystickX == right) {
    if (currentDisplayIndex + 1 < displayCount) {
      nextDisplay = currentDisplayIndex + 1;
    } else nextDisplay = 0;
  }

  if (stateJoystickX == left) {
    if (currentDisplayIndex  - 1 >= 0) {
      nextDisplay = currentDisplayIndex - 1;
    } else {
      nextDisplay = displayCount - 1;
    }
  }

  if (nextDisplay != currentDisplayIndex) {
    dpState[currentDisplayIndex] = 0;
  }

  return nextDisplay;
}


void state1() {

  blinkDPLed();

  buttonPressState = getButtonPressType();

  if (buttonPressState == longPressButtonState){ 
      reset();
  } else if (buttonPressState == shortPressButtonState) {
    currentState = systemStateLocked;
  } else {
    currentDisplayIndex = goToNextDisplay();
  }

}

void modifyDigit(int direction) {

  if (direction == up) {
    if (displayValue[currentDisplayIndex] + 1 < encodingsNumber) {
      displayValue[currentDisplayIndex]++;
    } else {
      displayValue[currentDisplayIndex] = 0;
    }
  } else {
    if(displayValue[currentDisplayIndex] - 1 >= 0) {
      displayValue[currentDisplayIndex]--;
    } else {
      displayValue[currentDisplayIndex] = encodingsNumber - 1;
    }
  }

}

void state2() {
  
  dpState[currentDisplayIndex] = 1;

  getJoystickState();

  if (stateJoystickY != 0) {
    modifyDigit(stateJoystickY);
  }

  buttonPressState = getButtonPressType();

  if (buttonPressState == shortPressButtonState) {
    currentState = systemStateUnlocked;
  }

}

void loop() {
  // put your main code here, to run repeatedly:

  writeAllDigits();

  switch(currentState) {

      case systemStateUnlocked: 
        state1();
        break;
      
      case systemStateLocked:
        state2();
        break;

      default:
        break;
  }
}
