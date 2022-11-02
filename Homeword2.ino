#define CARS_CROSSING_STATE 0
#define YELLOW_LIGHT_STATE 1
#define PEDESTRIANS_CROSSING_STATE 2
#define PEDESTRIANS_BLINKING_STATE 3

#define CARS_CROSSING_STATE_DURATION 8000
#define YELLOW_LIGHT_STATE_DURATION 3000
#define PEDESTRIANS_CROSSING_STATE_DURATION 8000
#define PEDESTRIANS_BLINKING_STATE_DURATION 4000

#define STATE_BUTTON_RELEASED 0
#define STATE_BUTTON_PRESSED 1

#define IDLE_STATE -1


const int buzzerPin = 3;
const int buttonPin = 4;

const int carLedRedPin = 5;
const int carLedYellowPin = 6;
const int carLedGreenPin = 7;

const int pedestrianLedRedPin = 8;
const int pedestrianLedGreenPin = 9;

int buttonState = LOW;
int lastButtonState = LOW;
int buttonValue = 0;
int reading = 0;

unsigned int currentTime = 0;
unsigned int lastChangeTime = IDLE_STATE; 

unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 50;

unsigned int pedestrianGreenLedState = LOW;
unsigned int pedestrianLedBlinkingInterval = 400;
unsigned int lastPedestrianLedChangeTime = 0;

unsigned int buzzerState = LOW;
unsigned int lastBuzzerChangeTime = 0;
unsigned int buzzerCrossingTone = 500;
unsigned int buzzerBlinkingTone = 700;

unsigned int buzzerCrossingInterval = 800;
unsigned int buzzerBlinkingInterval = 400;

unsigned int currentState = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  pinMode(carLedRedPin, OUTPUT);
  pinMode(carLedYellowPin, OUTPUT);
  pinMode(carLedGreenPin, OUTPUT);

  pinMode(pedestrianLedRedPin, OUTPUT);
  pinMode(pedestrianLedGreenPin, OUTPUT);

  setState(CARS_CROSSING_STATE);

  Serial.begin(9600);

}

int getButtonState() {

  reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {

    lastDebounceTime = millis();
  
  }

  if (millis() - lastDebounceTime > debounceDelay) {

    if (reading != buttonState) {
      
      buttonState = reading;

      if (buttonState == LOW) {
      
        return STATE_BUTTON_PRESSED;

      }

    }

  }

  lastButtonState = reading;
  
  return STATE_BUTTON_RELEASED;

}

void setLedValues(byte carRedValue, byte carYellowValue, byte carGreenValue, byte pedestrianRedValue, byte pedestrianGreenValue) { 

  digitalWrite(carLedRedPin, carRedValue);
  digitalWrite(carLedYellowPin, carYellowValue);
  digitalWrite(carLedGreenPin, carGreenValue);

  digitalWrite(pedestrianLedRedPin, pedestrianRedValue);
  digitalWrite(pedestrianLedGreenPin, pedestrianGreenValue);

}

void setState(int state) { 

  currentState = state;

  if (currentState == CARS_CROSSING_STATE) {

    lastChangeTime = IDLE_STATE;
    resetBuzzer();
    setLedValues(LOW, LOW, HIGH, HIGH, LOW);
  
  } else if (currentState == YELLOW_LIGHT_STATE) {

    lastChangeTime = millis();
    setLedValues(LOW, HIGH, LOW, HIGH, LOW);

  } else if (currentState == PEDESTRIANS_CROSSING_STATE) { 

      lastChangeTime = millis();
      resetBuzzer();
      setLedValues(HIGH, LOW, LOW, LOW, HIGH);
  
  } else if (currentState == PEDESTRIANS_BLINKING_STATE) {

      lastChangeTime = millis();
      resetBuzzer();
      setLedValues(HIGH, LOW, LOW, LOW, HIGH);

  }

}

void resetBuzzer() { 

  noTone(buzzerPin);
  buzzerState = LOW;
  lastBuzzerChangeTime = 0;

}

void playBuzzer(int buzzerTone) {

    if (buzzerState == HIGH) { 

      tone(buzzerPin, buzzerTone);

    } else {

      noTone(buzzerPin);

    }
}

void setBuzzerState (int buzzerTone, int buzzerInterval) {

  if (currentTime - lastBuzzerChangeTime > buzzerInterval) { 

    lastBuzzerChangeTime = currentTime;
    buzzerState = !buzzerState;

  }

  playBuzzer(buzzerTone);

}

void setGreenBlinkingLedState() {

  if (currentTime - lastPedestrianLedChangeTime > pedestrianLedBlinkingInterval) { 

    lastPedestrianLedChangeTime = currentTime;
    pedestrianGreenLedState = !pedestrianGreenLedState;

  }

  setLedValues(HIGH, LOW, LOW, LOW, pedestrianGreenLedState);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  buttonValue = getButtonState();
  currentTime = millis();

  if (currentState == CARS_CROSSING_STATE && lastChangeTime == IDLE_STATE && buttonValue == STATE_BUTTON_PRESSED) {
    
    lastChangeTime = millis();

  }
  
  if (lastChangeTime != IDLE_STATE) {

    if(currentState == CARS_CROSSING_STATE && currentTime - lastChangeTime > CARS_CROSSING_STATE_DURATION) {
       
       setState(YELLOW_LIGHT_STATE);
   
    } 

    if (currentState == YELLOW_LIGHT_STATE && currentTime - lastChangeTime > YELLOW_LIGHT_STATE_DURATION) {

      setState(PEDESTRIANS_CROSSING_STATE);

    }

    if (currentState == PEDESTRIANS_CROSSING_STATE && currentTime - lastChangeTime > PEDESTRIANS_CROSSING_STATE_DURATION) {

      setState(PEDESTRIANS_BLINKING_STATE);

    }

    if (currentState == PEDESTRIANS_BLINKING_STATE && currentTime - lastChangeTime > PEDESTRIANS_BLINKING_STATE_DURATION) {

      setState(CARS_CROSSING_STATE);
    }

  }

  
  
  if (currentState == PEDESTRIANS_CROSSING_STATE) {

    setBuzzerState(buzzerCrossingTone, buzzerCrossingInterval);

  } else if (currentState == PEDESTRIANS_BLINKING_STATE) {

    setBuzzerState(buzzerBlinkingTone, buzzerBlinkingInterval);
    setGreenBlinkingLedState();

  }

}
