#include <TM1637.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

const int screenCLKPin = 3;
const int screenDIOPin = 2;

const int playerRXPin = 11;
const int playerTXPin = 12;

const int previousButtonPin = 6;
const int nextButtonPin = 5;
const int sideButtonAPin = 8;
const int sideButtonBPin = 7;
const int sideButtonCPin = 9;

bool isFirstPlay = true;
bool volumeMode = false;
int volume = 10;
int newVolume = 10;
int fileNumber = 1;
int newFileNumber = 1;

TM1637 screen(screenCLKPin, screenDIOPin);
SoftwareSerial playerSerial(playerRXPin, playerTXPin);
DFRobotDFPlayerMini player; 

void setupPlayer() {
  playerSerial.begin(9600) ;
  player.begin(playerSerial) ;
  player.setTimeOut(500);
  player.reset();
}

void setupScreen() {
  screen.begin();
}

void setupButtons() {
  pinMode(previousButtonPin, INPUT_PULLUP);
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(sideButtonAPin, INPUT_PULLUP);
  pinMode(sideButtonBPin, INPUT_PULLUP);
  pinMode(sideButtonCPin, INPUT_PULLUP);
}

void setup() {
  setupPlayer();
  setupScreen();
  setupButtons();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Start");

  //tm.display("UOLU");
  //delay(5000);
  //tm.display("ALEA");
  //delay(5000);
  //tm.display("BOUC");
  //delay(5000);
  //tm.display("OUI ");
  //delay(5000);
  //tm.display("NON ");
  //delay(5000);
  //tm.display("TOUT");
  //delay(5000);
  //tm.display("QUIT");
  //delay(5000);
}

void firstPlay() {
  //player.disableDAC();
  delay(1000);
  player.play(fileNumber);
  player.volume(volume);
}

void displayNumber(int value) {
  int offset = 0;
  if (value < 10) {
    offset = 3;
  } else if (value < 100) {
    offset = 2;
  } else if (value < 1000) {
    offset = 1;
  }
  screen.display(value, false, true, offset); 
}

void loop() {
  if (isFirstPlay) {
    firstPlay();
    isFirstPlay = false;
  }

  if (digitalRead(previousButtonPin) == LOW) {
    Serial.println("PREVIOUS pressed");
    if (volumeMode) {
      newVolume = volume - 1;
    } else {
      newFileNumber = fileNumber - 1;
    }
    delay(500);
  }

  if (digitalRead(nextButtonPin) == LOW) {
    Serial.println("NEXT pressed");
    if (volumeMode) {
      newVolume = volume + 1;
    } else {
      newFileNumber = fileNumber + 1;
    }
    delay(500);
  }

  if (digitalRead(sideButtonAPin) == LOW) {
    Serial.println("Side button A pressed");
    if (volumeMode == false) {
      screen.display("UOLU");
      delay(500);
    }
    volumeMode = true;
  } else {
    volumeMode = false;
  }

  if (digitalRead(sideButtonBPin) == LOW) {
    Serial.println("Side button B pressed");
  }

  if (digitalRead(sideButtonCPin) == LOW) {
    Serial.println("Side button C pressed");
    delay(150);
  }

  if (volume != newVolume) {
    player.volume(newVolume);
    volume = newVolume;
    delay(150);
  }

  if (fileNumber != newFileNumber) {
    player.play(newFileNumber);
    fileNumber = newFileNumber;
    delay(150);
  }

  // Screen
  if (volumeMode == true) {
    displayNumber(volume);  
  } else {
    displayNumber(fileNumber);
  }
  
  /*
  tm.display("PLAY");
  player.setTimeOut(500);
  player.randomAll(); 
  //delay(1000);
  //Serial.println(player.readFileCounts());
  tm.clearScreen();
  //tm.display(player.readCurrentFileNumber(), false, true, 3);
  //Serial.println(player.readCurrentFileNumber());
  //Serial.println(player.readType());
  //Serial.println(player.read());
  //delay(5000);
  //player.stop();
  //tm.display("STOP");
  //delay(5000);
  */
}
