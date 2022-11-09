const int pinSW = 2; 
const int pinX = A0; 
const int pinY = A1;

const int pinA = 4;
const int pinB = 5;
const int pinC = 6;
const int pinD = 7;
const int pinE = 8;
const int pinF = 9;
const int pinG = 10;
const int pinDP = 11;

volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
const unsigned long debounceDelayTillRestart = 3000;

byte swState = HIGH;
bool switchState;
int currentState = 1;
int restart = 1;

const int segSize = 8;

bool commonAnode = false;

const int minThreshold = 400;
const int maxThreshold = 600;

bool joystickMoved;

int segments[segSize] = { 
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

byte segmentsState[segSize] = {
  LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
};

int xValue = 0; 
int yValue = 0;

const int nowhere = -1;

const int ledNeighbours[segSize][4]{ 
  { nowhere,      pinG - pinA,  	pinF - pinA,    pinB - pinA   },
  { pinA - pinA,  pinG - pinA,    pinF - pinA,    nowhere       },
  { pinG - pinA,  pinD - pinA,    pinE - pinA,    pinDP - pinA  },
  { pinG - pinA,  nowhere,        pinE - pinA,    pinC - pinA   },
  { pinG - pinA,  pinD - pinA,    nowhere,        pinC - pinA   },
  { pinA - pinA,  pinG - pinA,    nowhere,        pinB - pinA   },
  { pinA - pinA,  pinD - pinA,    nowhere,        nowhere       },
  { nowhere,      nowhere,        pinC - pinA,    nowhere       }
};

int currentPosition = pinDP - pinA;

const unsigned long blinkInterval = 250;
unsigned long blinkTime = 0;
byte blinkState = LOW;

void pressedSW()
{
  lastDebounceTime = millis();
}

void setup() {
  
  for (int i = 0; i < segSize; i++) {
    pinMode(segments[i], OUTPUT);
  }
  if(commonAnode == true)
  {
    for(int i = 0; i <= segSize; i++)
    {
      segmentsState[i] = HIGH;
    }
  }

  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(pinSW), pressedSW, CHANGE);
  
}

void statesModification()
{
  byte currentStateSW = digitalRead(pinSW);
  
  if (millis() - lastDebounceTime >= debounceDelay) { 
    
    if (swState != currentStateSW) {
    
      swState = currentStateSW;
    
      if (swState == HIGH) {
        
        if(restart == 3){
          restart = 1;
        }
    
        else {
        switchState = true; 
        }
      }  
    } else if (currentState == 1 && currentStateSW == LOW && restart == 1 && millis() - lastDebounceTime >= debounceDelayTillRestart) {
        restart = 2;
    }
  }
}

void readAxes() {

  xValue = analogRead(A0);
  yValue = analogRead(A1);
  
}

void displaySegments() {

  for (int i = 0; i < segSize; i++)
  {
    digitalWrite(segments[i], segmentsState[i]);
  }
}

void restartSegments() {

  for (int i = 0; i < segSize; i++)
  {
    segmentsState[i] = LOW;
  }
  currentPosition = pinDP - pinA;
  restart = 3;
}

void state1(){

  if(!joystickMoved){
    
    if (xValue > maxThreshold) {                     
      currentPosition = (ledNeighbours[currentPosition][0] != nowhere) ? ledNeighbours[currentPosition][0] : currentPosition;
    }  
  
    if (xValue < minThreshold) {                   
      currentPosition = (ledNeighbours[currentPosition][1] != nowhere) ? ledNeighbours[currentPosition][1] : currentPosition;
    }  
    
    if (yValue > maxThreshold) {                    
      currentPosition = (ledNeighbours[currentPosition][3] != nowhere) ?ledNeighbours[currentPosition][3] : currentPosition;
    }  
   
    if (yValue < minThreshold) {                  
      currentPosition = (ledNeighbours[currentPosition][2] != nowhere) ?ledNeighbours[currentPosition][2] : currentPosition;
    }

    joystickMoved = true;  

  }

  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
    joystickMoved = false;
  }

  blinkLed(currentPosition + pinA); 
}

void blinkLed(int pin) {

  digitalWrite(pin, blinkState);
  
  if (millis() - blinkTime > blinkInterval) {
    blinkTime = millis();
    blinkState = !blinkState;
  }
}

void state2() {

  if (!joystickMoved) {
    if (yValue > maxThreshold) {                    
      segmentsState[currentPosition] = LOW;
    }  
   
    else if (yValue < minThreshold) {                  
      segmentsState[currentPosition] = HIGH;
    }

    joystickMoved = true;
  }

  if (xValue >= minThreshold && xValue <= maxThreshold) {
    joystickMoved = false;
  } 
}

void loop() {

  statesModification();

  readAxes();

  displaySegments();

   if (currentState == 1 && restart == 2) {
    restartSegments();
    switchState = false;
  }

  if (currentState == 1 && switchState == true) {
    currentState = 2;
    switchState = false;
  }
 
  if (currentState == 2 && switchState == true){
    currentState = 1;
    switchState = false;
  }

  switch (currentState) {
  case 1:
    state1();
    break;
  case 2:
    state2();
    break; 
  default:
    break;   
  }
}
