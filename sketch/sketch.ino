#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define MUSIC_COUNT 23
#define STORY_COUNT 4

#define MAX_VOLT 4.2
#define MIN_VOLT 3.0
double levelByVerticalLine = (MAX_VOLT - MIN_VOLT) / SCREEN_WIDTH;

char *musics[] = {
  "1 Crocodiles",
  "2 Au feu les pompiers",
  "3 Baby shark",
  "4 Chi",
  "5 Comment ca va",
  "6 Un grand cerf",
  "7 Dansons la capucine",
  "8 Head shoulders knees and toes",
  "9 Un petit navire",
  "10 Araignee Gypsie",
  "11 Coccinelle rebelle",
  "12 Ferme de Mathurin",
  "13 Petit ver de terre",
  "14 Lundi matin",
  "15 Mon petit lapin",
  "16 Petit escargot",
  "17 Tchoupi",
  "18 Petit moulin",
  "19 Un elephant qui se balancait",
  "20 Un hippopoquoi",
  "21 Un jour dans sa cabane",
  "22 Une poule sur un mur",
  "23 Une souris verte"
};
char *stories[] = {
  "A",
  "B",
  "C",
  "D"
};


const int playerRXPin = 12;
const int playerTXPin = 11;

const int previousButtonPin = 2;
const int nextButtonPin = 3;
const int sideButtonAPin = 4;
const int sideButtonBPin = 5;
const int sideButtonCPin = 6;

bool isFirstPlay = true;
bool volumeMode = false;
bool batteryMode = false;
bool storyMode = false;
bool randomized = false;
bool repeatOne = false;
bool playFinished = true;
char *sideComboPressed = "";
int volume = 15;
int newVolume = volume;
int fileNumber = 1;
int newFileNumber = fileNumber;
int textOffset = 0;
int textMinOffset = -12;

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial playerSerial(playerRXPin, playerTXPin);
DFRobotDFPlayerMini player;

void setupPlayer() {
  playerSerial.begin(9600);
  player.begin(playerSerial);
  player.setTimeOut(500);
  player.reset();
  player.disableDAC();
}

void setupScreen() {
  if (!screen.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println("Screen allocation failed");
    for (;;);
  }
  delay(2000);
  screen.clearDisplay();
  screen.setTextWrap(false);
}

void setupButtons() {
  pinMode(previousButtonPin, INPUT_PULLUP);
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(sideButtonAPin, INPUT_PULLUP);
  pinMode(sideButtonBPin, INPUT_PULLUP);
  pinMode(sideButtonCPin, INPUT_PULLUP);
}

void displayText(char *text, int size = 3) {
  screen.clearDisplay();

  screen.setTextSize(size);
  screen.setTextColor(WHITE);
  screen.setCursor(0, 0);

  screen.println(text);
  screen.display();
}

void displayCurrentFile(int index, char *text) {
  //Serial.println(text);

  screen.clearDisplay();

  screen.setTextSize(4);
  screen.setTextColor(WHITE);
  
  screen.setCursor(textOffset, 1);
  screen.print(text);

  screen.display();
}

void displayNumber(int value) {
  screen.clearDisplay();

  screen.setTextSize(4);
  screen.setTextColor(WHITE);
  screen.setCursor(0, 0);

  screen.println(value);
  screen.display();
}

void displayBatteryLevel() {
  double currentVolt = readVcc() / 1000.0;

  screen.clearDisplay();

  // Draw gauge
  double level = currentVolt - MIN_VOLT;
  int linesToDraw = round(level / levelByVerticalLine);
  for (int column = 0; column < linesToDraw; column++) {
    screen.drawFastVLine(column, 0, SCREEN_HEIGHT, WHITE);
  }

  // Draw border rectangle
  screen.drawFastVLine(0, 0, SCREEN_HEIGHT, WHITE);
  screen.drawFastVLine(SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT, WHITE);
  screen.drawFastHLine(0, 0, SCREEN_WIDTH, WHITE);
  screen.drawFastHLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, WHITE);
  screen.drawPixel(0, 0, BLACK);
  screen.drawPixel(0, SCREEN_HEIGHT - 1, BLACK);
  screen.drawPixel(SCREEN_WIDTH - 1, 0, BLACK);
  screen.drawPixel(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BLACK);

  // Draw text
  int percent = (double(linesToDraw) / double(SCREEN_WIDTH)) * 100;
  if (percent > 100) {
    percent = 100;
  } else if (percent < 0) {
    percent = 0;
  }
  screen.setTextColor(INVERSE);
  screen.setTextSize(3);
  screen.setCursor(2, 2);
  screen.println(String(percent) + "%");
  screen.setTextSize(1);
  screen.setCursor(96, 22);
  screen.println(String(currentVolt) + "V");

  screen.display();
}

void setup() {
  setupPlayer();
  setupScreen();
  setupButtons();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Start");
}

void firstPlay() {
  player.volume(volume);
  if (randomized) {
    if (storyMode) {
      fileNumber = random(1, STORY_COUNT);
    } else {
      fileNumber = random(1, MUSIC_COUNT);
    }
  } else {
    fileNumber = 1;
  }
  newFileNumber = fileNumber;

  if (storyMode) {
    player.playFolder(2, fileNumber);
  } else {
    player.playFolder(1, fileNumber);
    textOffset = screen.width() / 2;
    textMinOffset = -12 * strlen(musics[fileNumber - 1]);
  }
}

void next() {
  if (volumeMode) {
    newVolume = volume + 1;
  } else if (repeatOne) {
    newFileNumber = fileNumber;
  } else if (randomized) {
    if (storyMode) {
      newFileNumber = random(1, STORY_COUNT);
    } else {
      newFileNumber = random(1, MUSIC_COUNT);
    }
  } else {
    newFileNumber = fileNumber + 1;
    if (storyMode) {
      if (newFileNumber > STORY_COUNT) {
        newFileNumber = 1;
      }
    } else {
      if (newFileNumber > MUSIC_COUNT) {
        newFileNumber = 1;
      }
    }
  }
}

void previous() {
  if (volumeMode) {
    newVolume = volume - 1;
  } else if (repeatOne) {
    newFileNumber = fileNumber;
  } else if (randomized) {
    if (storyMode) {
      newFileNumber = random(1, STORY_COUNT);
    } else {
      newFileNumber = random(1, MUSIC_COUNT);
    }
  } else {
    newFileNumber = fileNumber - 1;
    if (newFileNumber < 1) {
      if (storyMode) {
        newFileNumber = STORY_COUNT;
      } else {
        newFileNumber = MUSIC_COUNT;
      }
    }
  }
}

void updateSideButtonStates() {
  bool A = (digitalRead(sideButtonAPin) == LOW);
  bool B = (digitalRead(sideButtonBPin) == LOW);
  bool C = (digitalRead(sideButtonCPin) == LOW);

  sideComboPressed = "";
  if (A && B) {
    sideComboPressed = "AB";
    Serial.println("A & B pressed");
  } else if (A && C) {
    sideComboPressed = "AC";
    Serial.println("A & C pressed");
  } else if (A) {
    sideComboPressed = "A";
    Serial.println("A pressed");
  } else if (B) {
    sideComboPressed = "B";
    Serial.println("B pressed");
  } else if (C) {
    sideComboPressed = "C";
    Serial.println("C pressed");
  }

}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void loop() {
  if (isFirstPlay) {
    firstPlay();
    isFirstPlay = false;
  }

  if (digitalRead(previousButtonPin) == LOW) {
    Serial.println("PREVIOUS pressed");
    previous();
    delay(500);
  }

  if (digitalRead(nextButtonPin) == LOW) {
    Serial.println("NEXT pressed");
    next();
    delay(500);
  }

  updateSideButtonStates();

  if (sideComboPressed == "AB") {
    //if (batteryMode == false) {
    if (storyMode == false) {
      displayText("Histoire", 2);
      //delay(500);
    } else {
      displayText("Chanson");
    }
    delay(1000);

    //batteryMode = !batteryMode;
    storyMode = !storyMode;

    firstPlay();
  }

  if (sideComboPressed == "AC") {
    if (batteryMode == false) {
      displayText("Batterie", 2);
      delay(500);
    }

    batteryMode = true;
  } else {
    batteryMode = false;
  }

  if (sideComboPressed == "A") {
    if (volumeMode == false) {
      displayText("Volume");
      delay(500);
    }
    volumeMode = true;
  } else {
    volumeMode = false;
  }

  if (sideComboPressed == "B") {
    randomized = !randomized;
    if (randomized) {
      displayText("Aleatoire", 2);
    } else {
      displayText("Lineaire", 2);
    }
    delay(500);
  }

  if (sideComboPressed == "C") {
    repeatOne = !repeatOne;
    if (repeatOne) {
      displayText("Boucle");
    } else {
      displayText("Pas de boucle", 2);
    }
    delay(500);
  }

  if (volume != newVolume) {
    player.volume(newVolume);
    volume = newVolume;
    delay(150);
  }

  if (fileNumber != newFileNumber || playFinished) {
    playFinished = false;
    if (storyMode) {
      player.playFolder(2, newFileNumber);
    } else {
      player.playFolder(1, newFileNumber);
    }
    fileNumber = newFileNumber;

    //textOffset = screen.width();
    textOffset = screen.width() / 2;
    textMinOffset = -12 * strlen(musics[fileNumber - 1]);
    //displayCurrentFile(fileNumber, musics[fileNumber - 1]);
    //delay(150);
  }

  // Screen
  if (volumeMode == true) {
    displayNumber(volume);
  } else if (batteryMode) {
    displayBatteryLevel();
  } else {
    //displayNumber(fileNumber);
    displayCurrentFile(fileNumber, musics[fileNumber - 1]);
    delay(50);
    textOffset = textOffset - 1;
    if (textOffset < textMinOffset) {
      textOffset = screen.width() / 2;
    }
  }
  

  if (player.available()) {
    if (player.readType() == DFPlayerPlayFinished) {
      playFinished = true;
      next();
    }
  }
}
