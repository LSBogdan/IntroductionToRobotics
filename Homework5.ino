#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <String.h>

#define STATE_WELCOME_MESSAGE 0
#define STATE_MENU 1
#define STATE_MENU_SETTINGS 2
#define STATE_HIGHSCORE 3
#define STATE_ABOUT 4
#define STATE_HOW_TO_PLAY 5
#define STATE_GAME 6
#define STATE_SET_DIFFICULTY 7
#define STATE_SET_LCD_CONTRAST 8
#define STATE_SET_LCD_LIGHT 9
#define STATE_SET_MATRIX_LIGHT 10
#define STATE_SET_SOUND_VOLUME 11


#define MATRIX_SIZE 8
#define MATRIX_BRIGHTNESS 2
#define MATRIX_LIGHT_ITEMS_COUNT 6

#define STATE_WELCOME_MESSAGE_DURATION 5000

#define MENU_ITEMS_COUNT 6
#define SETTINGS_MENU_ITEMS_COUNT 7
#define LCD_LIGHT_ITEMS_COUNT 7
#define DIFFICULTY_ITEMS_COUNT 5
#define SOUND_VOLUME_ITEMS_COUNT 4

#define SW_PRESSED_STATE 1
#define SW_RELEASED_STATE 0

#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4

#define JOYSTICK_MIN_TRESHOLD 350
#define JOYSTICK_MAX_TRESHOLD 750

#define LCD_LIGHT_MIN_VALUE 250
#define LCD_LIGHT_MAX_VALUE 50

#define MATRIX_LIGHT_MIN_VALUE 2
#define MATRIX_LIGHT_MAX_VALUE 15

const byte pinSW = 2;
byte swState = HIGH;
byte reading = HIGH;
byte lastReading = HIGH;

unsigned long lastDebounceTime = 0;
unsigned int debounceDelay = 50;


const byte pinX = A0;
const byte pinY = A1;


int xValue = 0;
int yValue = 0;

bool joystickMoved = false;

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); // DIN, CLK, LOAD, No. DRIVER

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte arrowUp[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

const byte arrowDown[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

const byte currentSelectionPointer[8] = {
  B00000,
  B00100,
  B01000,
  B11111,
  B01000,
  B00100,
  B00000,
  B00000
};
 
unsigned long welcomeMessageTimer = millis();

const char* menuItems[] = {
  "Menu list",
  "1.Play",
  "2.Highscore",
  "3.Settings",
  "4.About",
  "5.How to play"
};

byte menuIndex = 0;
byte selectedMenuIndex = 0;

const char* settingsItems[] = {
  "Settings list",
  "1.Difficulty",
  "2.LCD contrast",
  "3.LCD light",
  "4.Matrix light",
  "5.Sound",
  "6.Back to menu"
};

byte settingsMenuIndex = 0;
byte selectedSettingsMenuIndex = 0;

const char* aboutItems[] = {
  "Lolot Bogdan",
  "Github:LSBogdan"
};

const char* lcdLightItems[] = {
  "LCD Light",
  "1.Verry low",
  "2.Low",
  "3.Medium",
  "4.High",
  "5.Verry high",
  "6.Back"
};

const byte lcdLightPin = 3;

byte lcdLight = 50;

byte lightLCDIndex = 0;
byte selectedLightLCDIndex = 0;

const char* matrixLightItems[] = {
  "Matrix Light list",
  "1.Low",
  "2.Medium",
  "3.High",
  "4.Verry high",
  "5.Back"
};

byte matrixLight = 2;

byte lightMatrixIndex = 0;
byte selectedLightMatrixIndex = 0;

const char* difficultySetItems[] = {
  "Difficulty",
  "1.Eassy :)",
  "2.Medium :|",
  "3.Hard :(",
  "4.Back"
};

byte difficultyItemsIndex = 0;
byte selectedDifficultyItemsIndex = 0;

const char* soundSetItems[] = {
  "Sound",
  "1.ON",
  "2.OF",
  "3.Back"
};

byte soundItemsIndex = 0;
byte selectedSoundItemsIndex = 0;

byte matrix[MATRIX_SIZE][MATRIX_SIZE] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

const byte moveInterval = 100;
unsigned long long lastMoved = 0;

bool matrixChanged = true;

byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;

byte newRandomCol = 0;
byte newRandomRow = 0;

int currentScore = 0;

struct Settings {
	
	byte difficulty;
	byte lcdContrast;
	byte lcdLight;
	byte matrixLight;
	bool soundStatus;

} settingsData;


byte currentState = STATE_WELCOME_MESSAGE;

void setup() {

  Serial.begin(9600);

  pinMode(pinSW, INPUT_PULLUP);
  pinMode(lcdLightPin, OUTPUT);

  //analogWrite(lcdLightPin, 100);

  readFromStorage();

  analogWrite(lcdLightPin, settingsData.lcdLight);

  lc.shutdown(0, false);
  lc.setIntensity(0, settingsData.matrixLight);
  lc.clearDisplay(0);
 
  lcd.begin(16, 2);
 
  //analogWrite(lcdBrightnessPin, 150);

  lcd.createChar(0, arrowUp);
  lcd.createChar(1, arrowDown);
  lcd.createChar(2, currentSelectionPointer);

}

void readFromStorage() {
  EEPROM.get(0, settingsData);  
}

void writeSettingsInStorage() {
  EEPROM.put(0, settingsData);
}

void displayWelcomeMessage() {

  String messageToDisplay = "You play the";

  lcd.setCursor(0, 0);
  lcd.print(messageToDisplay);

  messageToDisplay = "BEST SNAKE EVER";

  lcd.setCursor(0, 1);
  lcd.print(messageToDisplay);

  if (millis() - welcomeMessageTimer > STATE_WELCOME_MESSAGE_DURATION) {  
    currentState = STATE_MENU;
    lcd.clear();
  }
}

void processSWState() {

  getSWState();

  if (swState == SW_PRESSED_STATE && currentState == STATE_MENU) {

    if (selectedMenuIndex == 1) {
       lcd.clear();
       currentState = STATE_GAME;
       swState = SW_RELEASED_STATE;
       return;
    }

    if (selectedMenuIndex == 2) {
      lcd.clear();
      currentState = STATE_HIGHSCORE;
      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedMenuIndex == 3) {
      lcd.clear();
      currentState = STATE_MENU_SETTINGS;
      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedMenuIndex == 4) {
      lcd.clear();
      currentState = STATE_ABOUT;
      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedMenuIndex == 5) {
      lcd.clear();
      currentState = STATE_HOW_TO_PLAY;
      swState = SW_RELEASED_STATE;
      return;
    }
  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_MENU_SETTINGS) {

    if (selectedSettingsMenuIndex == 1) {
      lcd.clear();
      currentState = STATE_SET_DIFFICULTY;
      swState = SW_RELEASED_STATE;
      return;
    }
   
    if (selectedSettingsMenuIndex == 2) {
      lcd.clear();
      currentState = STATE_SET_LCD_CONTRAST;
      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedSettingsMenuIndex == 3) {
      lcd.clear();
      currentState = STATE_SET_LCD_LIGHT;
      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedSettingsMenuIndex == 4) {
      lcd.clear();
      currentState = STATE_SET_MATRIX_LIGHT;
      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedSettingsMenuIndex == 5) {
      lcd.clear();
      currentState = STATE_SET_SOUND_VOLUME;
      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedSettingsMenuIndex == 6) {
      lcd.clear();
      currentState = STATE_MENU;
      swState = SW_RELEASED_STATE;
      return;
    }
  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_ABOUT) {
    lcd.clear();
    currentState = STATE_MENU;
    swState = SW_RELEASED_STATE;
    return;
  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_HOW_TO_PLAY) {
   
    lcd.clear();
    currentState = STATE_MENU;
    swState = SW_RELEASED_STATE;
    return;

  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_HIGHSCORE) {
   
    lcd.clear();
    currentState = STATE_MENU;
    swState = SW_RELEASED_STATE;
    return;
   
  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_GAME) {
   
    lcd.clear();
    currentState = STATE_MENU;
    swState = SW_RELEASED_STATE;
    return;
   
  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_SET_DIFFICULTY) {

    if (selectedDifficultyItemsIndex == 4) {
      lcd.clear();
      currentState = STATE_MENU_SETTINGS;
      swState = SW_RELEASED_STATE;
      return;
    }

  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_SET_LCD_CONTRAST) {
    lcd.clear();
    currentState = STATE_MENU_SETTINGS;
    swState = SW_RELEASED_STATE;
    return;
  }

  // if (swState == SW_PRESSED_STATE &&  currentState == STATE_SET_LCD_LIGHT) {
  //   lcd.clear();
  //   currentState = STATE_MENU_SETTINGS;
  //   swState = SW_RELEASED_STATE;
  //   return;
  // }
  // if (swState == SW_PRESSED_STATE &&  currentState == STATE_SET_MATRIX_LIGHT) {
  //   lcd.clear();
  //   currentState = STATE_MENU_SETTINGS;
  //   swState = SW_RELEASED_STATE;  
  //   return;
  // }

  if (swState == SW_PRESSED_STATE && currentState == STATE_SET_SOUND_VOLUME) {

    if (selectedSoundItemsIndex == 3) {
      lcd.clear();
      currentState = STATE_MENU_SETTINGS;
      swState = SW_RELEASED_STATE;
      return;
    }
  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_SET_LCD_LIGHT) {

    if (selectedLightLCDIndex == 1) {
      lcd.clear();
      lcdLight = 50;
      analogWrite(lcdLightPin, lcdLight);

      settingsData.lcdLight = lcdLight;
      writeSettingsInStorage();

      swState = SW_RELEASED_STATE;
      return;
    }

    if (selectedLightLCDIndex == 2) {
      lcd.clear();
      lcdLight = 100;
      analogWrite(lcdLightPin, lcdLight);
      swState = SW_RELEASED_STATE;

      settingsData.lcdLight = lcdLight;
      writeSettingsInStorage();

      return;
    }
   
    if (selectedLightLCDIndex == 3) {
      lcd.clear();
      lcdLight = 150;
      analogWrite(lcdLightPin, lcdLight);

      settingsData.lcdLight = lcdLight;
      writeSettingsInStorage();

      swState = SW_RELEASED_STATE;
      
      return;
    }

    if (selectedLightLCDIndex ==4) {
      lcd.clear();
      lcdLight = 200;
      analogWrite(lcdLightPin, lcdLight);
      swState = SW_RELEASED_STATE;

      settingsData.lcdLight = lcdLight;
      writeSettingsInStorage();
      
      return;
    }
   
    if (selectedLightLCDIndex == 5) {
      lcd.clear();
      lcdLight = 250;
      analogWrite(lcdLightPin, lcdLight);
      swState = SW_RELEASED_STATE;

      settingsData.lcdLight = lcdLight;
      writeSettingsInStorage();

      return;
    }

    if (selectedLightLCDIndex == 6) {
      lcd.clear();
      currentState = STATE_MENU_SETTINGS;
      swState = SW_RELEASED_STATE;

      settingsData.lcdLight = lcdLight;
      writeSettingsInStorage();

      return;
    }
   
  }

  if (swState == SW_PRESSED_STATE && currentState == STATE_SET_MATRIX_LIGHT) {

    if (selectedLightMatrixIndex == 1) {
      //lcd.clear();
      matrixLight = 2;
      lc.setIntensity(0, matrixLight);
      swState = SW_RELEASED_STATE;

      settingsData.matrixLight = matrixLight;
      writeSettingsInStorage();

      return;
    }

    if (selectedLightMatrixIndex == 2) {
      //lcd.clear();
      matrixLight = 5;
      lc.setIntensity(0, matrixLight);
      swState = SW_RELEASED_STATE;

      settingsData.matrixLight = matrixLight;
      writeSettingsInStorage();

      return;
    }

    if (selectedLightMatrixIndex == 3) {
      //lcd.clear();
      matrixLight = 10;
      lc.setIntensity(0, matrixLight);
      swState = SW_RELEASED_STATE;

      settingsData.matrixLight = matrixLight;
      writeSettingsInStorage();

      return;
    }

    if (selectedLightMatrixIndex == 4) {
      //lcd.clear();
      matrixLight = 15;
      lc.setIntensity(0, matrixLight);
      swState = SW_RELEASED_STATE;

      settingsData.matrixLight = matrixLight;
      writeSettingsInStorage();
      
      return;
    }

    if (selectedLightMatrixIndex == 5) {
      lcd.clear();
      currentState = STATE_MENU_SETTINGS;
      swState = SW_RELEASED_STATE;
      return;
    }
  }

}

byte getSWState() {

  // reading = !digitalRead(pinSW);

  // if (reading != lastReading) {
  //   lastDebounceTime = millis();
  // }

  // if (millis() - lastDebounceTime > debounceDelay) {
  //   if(swState != reading) {
  //       swState = reading;

  //   if (swState == LOW) {
  //     return SW_PRESSED_STATE;
  //     }
  //   }
  // }

  // lastReading = reading;
  // return SW_RELEASED_STATE;

  swState = !digitalRead(pinSW);
 
  if (swState != lastReading) {
    return SW_PRESSED_STATE;
  }

   lastReading = swState;
   return SW_RELEASED_STATE;
}

void readAxes() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
}

byte getJoystickState() {

  readAxes();

  if (xValue < JOYSTICK_MIN_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    return RIGHT;
  }

  if (xValue > JOYSTICK_MAX_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    return LEFT;
  }

  if (yValue < JOYSTICK_MIN_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    return UP;
  }

  if (yValue > JOYSTICK_MAX_TRESHOLD && !joystickMoved) {
    joystickMoved = true;
    return DOWN;
  }

  if (xValue >= JOYSTICK_MIN_TRESHOLD && xValue <= JOYSTICK_MAX_TRESHOLD && yValue >= JOYSTICK_MIN_TRESHOLD && yValue <= JOYSTICK_MAX_TRESHOLD) {
    joystickMoved = false;
    return 0;
  }

  return 0;
}

void menuProcessJoystickState() {

  byte newDirection = getJoystickState();

  if (newDirection == 0 || (newDirection != UP && newDirection != DOWN)) {
    return;
  }

  selectedMenuIndex = newDirection == DOWN ? selectedMenuIndex + 1 : selectedMenuIndex - 1;
  selectedMenuIndex = constrain(selectedMenuIndex, 0, MENU_ITEMS_COUNT  - 1);

  if (newDirection == DOWN && selectedMenuIndex % 2 == 0) {
    menuIndex = selectedMenuIndex;
  }
  else if (newDirection == UP && selectedMenuIndex % 2 == 1) {
    menuIndex = selectedMenuIndex - 1;
  }

  menuIndex = constrain(menuIndex, 0, MENU_ITEMS_COUNT - 1);

  lcd.clear();

}

void displayMenu() {
 
 
  lcd.setCursor(0, 0);  
  lcd.print(menuItems[menuIndex]);

  if (menuIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }


  if (menuIndex != MENU_ITEMS_COUNT - 1) {
    lcd.setCursor(0, 1);
    lcd.print(menuItems[menuIndex + 1]);
  }

  if (menuIndex < MENU_ITEMS_COUNT - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }

  if (selectedMenuIndex) {
    byte currentLinePosition = selectedMenuIndex % 2;
    byte currentLineLength = strlen(menuItems[selectedMenuIndex]);
    lcd.setCursor(currentLineLength, currentLinePosition);
    lcd.write((byte)2);
  }

  processSWState();
 
 
  menuProcessJoystickState();
}

void settingsMenuProcessJoystickState() {

  byte newDirection = getJoystickState();

  if (newDirection == 0 || (newDirection != UP && newDirection != DOWN)) {
    return;
  }

  selectedSettingsMenuIndex = newDirection == DOWN ? selectedSettingsMenuIndex + 1 : selectedSettingsMenuIndex - 1;
  selectedSettingsMenuIndex = constrain(selectedSettingsMenuIndex, 0, SETTINGS_MENU_ITEMS_COUNT - 1);

  if (newDirection == DOWN && selectedSettingsMenuIndex % 2 == 0) {
    settingsMenuIndex = selectedSettingsMenuIndex;
  }

  else if (newDirection == UP && selectedSettingsMenuIndex% 2 == 1) {
    settingsMenuIndex = selectedSettingsMenuIndex - 1;
  }

  settingsMenuIndex = constrain(settingsMenuIndex, 0, SETTINGS_MENU_ITEMS_COUNT - 1);
 
  lcd.clear();
}

void displaySettingsMenu() {

  lcd.setCursor(0, 0);
  lcd.print(settingsItems[settingsMenuIndex]);

  if (settingsMenuIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }  

  if (settingsMenuIndex != SETTINGS_MENU_ITEMS_COUNT - 1) {
    lcd.setCursor(0, 1);
    lcd.print(settingsItems[settingsMenuIndex + 1]);
  }
 
  if (settingsMenuIndex < SETTINGS_MENU_ITEMS_COUNT - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }

  if (selectedSettingsMenuIndex) {
    byte currentLinePosition = selectedSettingsMenuIndex % 2;
    byte currentLineLength = strlen(settingsItems[selectedSettingsMenuIndex]);
    lcd.setCursor(currentLineLength, currentLinePosition);
    lcd.write((byte)2);
  }

  processSWState();

  settingsMenuProcessJoystickState();
 
}

void displayAbout() {

  lcd.setCursor(2, 0);
  lcd.print(aboutItems[0]);

  lcd.setCursor(0, 1);
  lcd.print(aboutItems[1]);

  processSWState();

}

void displayHowToPlay() {


  String messageToDisplay = "Run after food";

  lcd.setCursor(1, 0);
  lcd.print(messageToDisplay);

  processSWState();
}

void displayHighscore() {

  String messageToDisplay = "!No highscore!";

  lcd.setCursor(1, 0);
  lcd.print(messageToDisplay);

  processSWState();  
}

void displayContrast() {

  String messageToDisplay = "Just use the";

  lcd.setCursor(0, 0);
  lcd.print(messageToDisplay);

  messageToDisplay = "potentiometer";

  lcd.setCursor(0, 1);
  lcd.print(messageToDisplay);

  processSWState();
}

void displayLCDLightProcessJoystickState() {

  byte newDirection = getJoystickState();

  if (newDirection == 0 || (newDirection != UP && newDirection != DOWN)) {
    return;
  }

  selectedLightLCDIndex = newDirection == DOWN ? selectedLightLCDIndex + 1 : selectedLightLCDIndex - 1;
  selectedLightLCDIndex = constrain(selectedLightLCDIndex, 0, LCD_LIGHT_ITEMS_COUNT - 1);

  if (newDirection == DOWN && selectedLightLCDIndex % 2 == 0) {
    lightLCDIndex = selectedLightLCDIndex;
  }

  else if (newDirection == UP && selectedLightLCDIndex % 2 == 1) {
    lightLCDIndex = selectedLightLCDIndex - 1;
  }

  lightLCDIndex = constrain(lightLCDIndex, 0, LCD_LIGHT_ITEMS_COUNT - 1);

  lcd.clear();
}

void displayLCDLight() {
   
  lcd.setCursor(0, 0);
  lcd.print(lcdLightItems[lightLCDIndex]);

  if (lightLCDIndex  > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }

  if (lightLCDIndex != LCD_LIGHT_ITEMS_COUNT - 1) {
    lcd.setCursor(0, 1);
    lcd.print(lcdLightItems[lightLCDIndex + 1]);
  }

  if (lightLCDIndex < LCD_LIGHT_ITEMS_COUNT - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }

  if (selectedLightLCDIndex > 0) {
    byte currentLinePosition = selectedLightLCDIndex % 2;
    byte currentLineLength = strlen(lcdLightItems[selectedLightLCDIndex]);
    lcd.setCursor(currentLineLength, currentLinePosition);
    lcd.write((byte)2);
  }

  processSWState();

  displayLCDLightProcessJoystickState();

}

void matrixLightUp() {

  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      lc.setLed(0, i, j, true);
    }
  }  
}

void matrixLightProcessJoystickState() {

  byte newDirection = getJoystickState();

  if (newDirection == 0 || (newDirection != UP && newDirection != DOWN)) {
    return;
  }

  selectedLightMatrixIndex = newDirection == DOWN ? selectedLightMatrixIndex + 1 : selectedLightMatrixIndex - 1;
  selectedLightMatrixIndex = constrain(selectedLightMatrixIndex, 0, SETTINGS_MENU_ITEMS_COUNT - 1);

  if (newDirection == DOWN && selectedLightMatrixIndex % 2 == 0) {
    lightMatrixIndex = selectedLightMatrixIndex;
  }

  else if (newDirection == UP && selectedLightMatrixIndex% 2 == 1) {
    lightMatrixIndex = selectedLightMatrixIndex - 1;
  }

  lightMatrixIndex = constrain(lightMatrixIndex, 0, SETTINGS_MENU_ITEMS_COUNT - 1);
 
  lcd.clear();
}

void displayMatrixLight() {

  matrixLightUp();

  lcd.setCursor(0, 0);
  lcd.print(matrixLightItems[lightMatrixIndex]);

  if (lightMatrixIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }

  if (lightMatrixIndex != MATRIX_LIGHT_ITEMS_COUNT - 1) {
    lcd.setCursor(0, 1);
    lcd.print(matrixLightItems[lightMatrixIndex + 1]);
  }

  if (lightMatrixIndex < MATRIX_LIGHT_ITEMS_COUNT - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }

  if (selectedLightMatrixIndex) {
    byte currentLinePosition = selectedLightMatrixIndex % 2;
    byte currentLineLength = strlen(matrixLightItems[selectedLightMatrixIndex]);
    lcd.setCursor(currentLineLength, currentLinePosition);
    lcd.write((byte)2);
  }

  processSWState();

  matrixLightProcessJoystickState();
}

void soundProcessJoystickState() {
  
    byte newDirection = getJoystickState();

  if (newDirection == 0 || (newDirection != UP && newDirection != DOWN)) {
    return;
  }

  selectedSoundItemsIndex  = newDirection == DOWN ? selectedSoundItemsIndex  + 1 : selectedSoundItemsIndex  - 1;
  selectedSoundItemsIndex  = constrain(selectedSoundItemsIndex , 0, SOUND_VOLUME_ITEMS_COUNT  - 1);

  if (newDirection == DOWN && selectedSoundItemsIndex  % 2 == 0) {
    soundItemsIndex  = selectedSoundItemsIndex ;
  }
  else if (newDirection == UP && selectedSoundItemsIndex  % 2 == 1) {
    soundItemsIndex  = selectedSoundItemsIndex  - 1;
  }

  soundItemsIndex  = constrain(soundItemsIndex , 0, SOUND_VOLUME_ITEMS_COUNT - 1);

  lcd.clear();
}

void displaySoundVolume() {

  lcd.setCursor(0, 0);  
  lcd.print(soundSetItems[soundItemsIndex]);

  if (soundItemsIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }


  if (soundItemsIndex != SOUND_VOLUME_ITEMS_COUNT - 1) {
    lcd.setCursor(0, 1);
    lcd.print(soundSetItems[soundItemsIndex  + 1]);
  }

  if (menuIndex < SOUND_VOLUME_ITEMS_COUNT - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }

  if (selectedSoundItemsIndex ) {
    byte currentLinePosition = selectedSoundItemsIndex  % 2;
    byte currentLineLength = strlen(soundSetItems[selectedSoundItemsIndex ]);
    lcd.setCursor(currentLineLength, currentLinePosition);
    lcd.write((byte)2);
  }

  processSWState();

  soundProcessJoystickState();
}


void difficultyProcessJoystickState() {

  byte newDirection = getJoystickState();

  if (newDirection == 0 || (newDirection != UP && newDirection != DOWN)) {
    return;
  }

  selectedDifficultyItemsIndex  = newDirection == DOWN ? selectedDifficultyItemsIndex  + 1 : selectedDifficultyItemsIndex  - 1;
  selectedDifficultyItemsIndex  = constrain(selectedDifficultyItemsIndex , 0, DIFFICULTY_ITEMS_COUNT - 1);

  if (newDirection == DOWN && selectedDifficultyItemsIndex  % 2 == 0) {
    difficultyItemsIndex  = selectedDifficultyItemsIndex ;
  }
  else if (newDirection == UP && selectedDifficultyItemsIndex  % 2 == 1) {
    difficultyItemsIndex  = selectedDifficultyItemsIndex  - 1;
  }

  difficultyItemsIndex  = constrain(difficultyItemsIndex , 0, MENU_ITEMS_COUNT - 1);

  lcd.clear();

}

void displayDifficulty() {

  lcd.setCursor(0, 0);
  lcd.print(difficultySetItems[difficultyItemsIndex]);

  if (difficultyItemsIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }

  if (difficultyItemsIndex  != DIFFICULTY_ITEMS_COUNT - 1) {
    lcd.setCursor(0, 1);
    lcd.print(difficultySetItems[difficultyItemsIndex + 1]);
  } 

  if (difficultyItemsIndex < DIFFICULTY_ITEMS_COUNT - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }

  if (selectedDifficultyItemsIndex) {
    byte currentLinePosition = selectedDifficultyItemsIndex % 2;
    byte currentLineLength = strlen(difficultySetItems[selectedDifficultyItemsIndex]);
    lcd.setCursor(currentLineLength, currentLinePosition);
    lcd.write((byte)2);
  }

  processSWState();

  difficultyProcessJoystickState();
}


void updateMatrix() {

  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      lc.setLed(0, i, j, matrix[i][j]);
    }
  }
}

void generateFood() {

  newRandomCol = random(MATRIX_SIZE);
  newRandomRow = random(MATRIX_SIZE);
  matrix[newRandomRow][newRandomCol] = 1;

  updateMatrix;
}

void updatePosition() {
 
  readAxes();

  xLastPos = xPos;
  yLastPos = yPos;

  if (xValue > JOYSTICK_MIN_TRESHOLD) {
    if (xPos < MATRIX_SIZE - 1) {
      xPos++;
    }
    else {
      xPos = 0;
    }
  }

  if (xValue < JOYSTICK_MAX_TRESHOLD) {
    if (xPos > 0) {
      xPos--;
    } 
    else {
      xPos = MATRIX_SIZE - 1;
    }
  }

  if (yValue < JOYSTICK_MIN_TRESHOLD) {
    if (yPos > 0) {
      yPos--;
    }
    else {
      yPos = MATRIX_SIZE - 1;
    }
  }

  if (yValue > JOYSTICK_MAX_TRESHOLD) {
    if (yPos < MATRIX_SIZE - 1) {
      yPos++;
    }
    else {
      yPos = 0;
    }
  }

  if (xLastPos != xPos || yLastPos != yPos) {
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos]=1;
    matrixChanged = true;
    lastMoved = millis();
  }

}

void displayScore(int score) {

  String messageToDisplay = "Good luck!";


  lcd.setCursor(3, 0);
  lcd.print(messageToDisplay);

  messageToDisplay = "Score: ";

  lcd.setCursor(0, 1);
  lcd.print(messageToDisplay);

  lcd.setCursor(7, 1);
  lcd.print(score);
}

void playGame() {

  currentScore == 0 ? displayScore(0) : displayScore(currentScore);

  if (millis() - lastMoved > moveInterval) {
    updatePosition();
  }

  if (matrixChanged == true) {
    updateMatrix();
    matrixChanged = false;
  }

  if (xPos == newRandomRow && yPos == newRandomCol) {
    generateFood();
    currentScore++;
  }
}

void loop() {
 
  switch(currentState){

    case STATE_WELCOME_MESSAGE:
      displayWelcomeMessage();
      break;
   
    case STATE_MENU:
      delay(50);
      displayMenu();
      break;
   
    case STATE_MENU_SETTINGS:
      delay(50);
      displaySettingsMenu();
      break;
     
    case STATE_ABOUT:
      delay(50);
      displayAbout();
      break;

    case STATE_HOW_TO_PLAY:
      delay(50);
      displayHowToPlay();
      break;

    case STATE_HIGHSCORE:
      delay(50);
      displayHighscore();
      break;
     
    case STATE_SET_LCD_CONTRAST:
      delay(50);
      displayContrast();
      break;

    case STATE_SET_LCD_LIGHT:
      delay(50);
      displayLCDLight();
      break;

    case STATE_SET_MATRIX_LIGHT:
      delay(50);
      displayMatrixLight();
      break;

    case STATE_SET_SOUND_VOLUME:
      delay(50);
      displaySoundVolume();
      break;
   
    case STATE_SET_DIFFICULTY:
      delay(50);
      displayDifficulty();
      break;

    case STATE_GAME:
      delay(50);
      playGame();
      processSWState();
      break;
  }
}
