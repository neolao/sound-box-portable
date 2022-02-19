#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define PLAYER_RX_PIN 12
#define PLAYER_TX_PIN 11
#define PREVIOUS_BUTTON_PIN 2
#define NEXT_BUTTON_PIN 3
#define SIDE_BUTTON_A_PIN 4
#define SIDE_BUTTON_B_PIN 5
#define SIDE_BUTTON_C_PIN 6
#define MODE_MUSIC 1
#define MODE_BATTERY 3
#define CHAR_SIZE 24

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define MUSIC_COUNT 23

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

uint8_t mode = MODE_MUSIC;
bool volumeMode = false;
bool batteryMode = false;
bool randomized = false;
bool repeatOne = false;
char *sideComboPressed = "";
uint8_t volume = 15;
uint8_t fileNumber = 1;
int textOffset = 0;
int textMinOffset = 0;

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial playerSerial(PLAYER_RX_PIN, PLAYER_TX_PIN);
DFRobotDFPlayerMini player;

void setupPlayer() {
  playerSerial.begin(9600);
  player.begin(playerSerial);
  player.setTimeOut(500);
  player.reset();
  player.disableDAC();
  player.volume(volume);
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
  pinMode(PREVIOUS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SIDE_BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(SIDE_BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(SIDE_BUTTON_C_PIN, INPUT_PULLUP);

  //attachInterrupt(digitalPinToInterrupt(PREVIOUS_BUTTON_PIN), previous, FALLING);
  //attachInterrupt(digitalPinToInterrupt(NEXT_BUTTON_PIN), next, FALLING);
  //attachInterrupt(digitalPinToInterrupt(SIDE_BUTTON_C_PIN), toggleLoop, FALLING);
}

void displayText(char *text) {
  screen.clearDisplay();

  screen.setTextSize(4);
  screen.setTextColor(WHITE);
  screen.setCursor(0, 0);

  screen.println(text);
  screen.display();
}

void displayCurrentFile(char *text) {
  //Serial.println(text);

  screen.clearDisplay();

  screen.setTextSize(4);
  screen.setTextColor(WHITE);

  screen.setCursor(textOffset, 2);
  screen.print(text);
  screen.setCursor(textOffset - textMinOffset + CHAR_SIZE, 2);
  screen.print(text);


  screen.display();
}

void displayVolume(uint8_t value) {
  screen.clearDisplay();

  screen.setTextSize(4);
  screen.setTextColor(WHITE);
  screen.setCursor(0, 0);

  screen.print("V ");
  screen.print(value);
  screen.display();
}

void displayBatteryLevel() {
  double currentVolt = readVcc() / 1000.0;

  screen.clearDisplay();

  // Draw gauge
  double level = currentVolt - MIN_VOLT;
  uint8_t linesToDraw = round(level / levelByVerticalLine);
  for (uint8_t column = 0; column < linesToDraw; column++) {
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
  uint8_t percent = (double(linesToDraw) / double(SCREEN_WIDTH)) * 100;
  if (percent > 100) {
    percent = 100;
  } else if (percent < 0) {
    percent = 0;
  }
  screen.setTextColor(INVERSE);
  screen.setTextSize(3);
  screen.setCursor(3, 6);
  screen.print(percent);
  screen.print("%");
  screen.setTextSize(1);
  screen.setCursor(96, 22);
  screen.print(currentVolt);
  screen.print("V");
  //Serial.println(currentVolt);

  screen.display();
}

void setup() {
  setupPlayer();
  setupScreen();
  setupButtons();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Start");

  playMusic();
}

void playMusic() {
  player.playFolder(1, fileNumber);

  textOffset = screen.width() / 2;
  textMinOffset = -CHAR_SIZE * strlen(musics[fileNumber - 1]);
}

void next() {
  if (repeatOne) {
    playMusic();
  } else if (randomized) {
    fileNumber = random(1, MUSIC_COUNT);
    playMusic();
  } else {
    fileNumber = fileNumber + 1;
    if (fileNumber > MUSIC_COUNT) {
      fileNumber = 1;
    }
    playMusic();
  }
}

void previous() {
  if (repeatOne) {
    playMusic();
  } else if (randomized) {
    fileNumber = random(1, MUSIC_COUNT);
    playMusic();
  } else {
    fileNumber = fileNumber - 1;
    if (fileNumber < 1) {
      fileNumber = MUSIC_COUNT;
    }
    playMusic();
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
  bool previousButtonPressed = digitalRead(PREVIOUS_BUTTON_PIN) == LOW;
  bool nextButtonPressed = digitalRead(NEXT_BUTTON_PIN) == LOW;
  bool sideButtonAPressed = digitalRead(SIDE_BUTTON_A_PIN) == LOW;
  bool sideButtonBPressed = digitalRead(SIDE_BUTTON_B_PIN) == LOW;
  bool sideButtonCPressed = digitalRead(SIDE_BUTTON_C_PIN) == LOW;

  if (!sideButtonAPressed && previousButtonPressed) {
    Serial.println("PREVIOUS pressed");
    previous();
    delay(400);
  }

  if (!sideButtonAPressed && nextButtonPressed) {
    Serial.println("NEXT pressed");
    next();
    delay(400);
  }

  if (sideButtonAPressed && previousButtonPressed) {
    player.volume(volume--);
    displayVolume(volume);
    delay(500);
  }

  if (sideButtonAPressed && nextButtonPressed) {
    player.volume(volume++);
    displayVolume(volume);
    delay(500);
  }

  if (sideButtonAPressed && sideButtonBPressed) {
    displayBatteryLevel();
    delay(2000);
  }

  if (sideButtonBPressed) {
    Serial.println("Toggle random");
    randomized = !randomized;
    if (randomized) {
      displayText("Aleatoire");
    } else {
      displayText("Lineaire");
    }
    delay(400);
  }

  if (sideButtonCPressed) {
    Serial.println("Toggle loop");
    repeatOne = !repeatOne;
    if (repeatOne) {
      displayText("Boucle");
    } else {
      displayText("No boucle");
    }
    delay(400);
  }


    displayCurrentFile(musics[fileNumber - 1]);
    textOffset = textOffset - 1;
    if (textOffset < textMinOffset - CHAR_SIZE) {
      textOffset = 0;
    }
  

  if (player.available() && player.readType() == DFPlayerPlayFinished) {
    next();
  }
}
