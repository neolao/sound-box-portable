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

bool initialized = false;
bool isFirstPlay = true;
bool volumeMode = false;
bool randomized = false;
bool repeatOne = false;
int volume = 10;
int newVolume = 10;
int fileNumber = 1;
int newFileNumber = 1;
int fileCount = 0;

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

void initialize() {
  while(fileCount < 2) {
    fileCount = player.readFileCounts();
  }
  screen.display("TOTA");
  delay(200);
  displayNumber(fileCount);
  delay(200);
  initialized = true;
}

void setup() {
  setupPlayer();
  setupScreen();
  setupButtons();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Start");

  initialize();
}

void firstPlay() {
  if (randomized) {
    player.play(random(1, fileCount));
  } else {
    player.play(fileNumber);
  }
  player.volume(volume);
}

void next() {
  if (volumeMode) {
    newVolume = volume + 1;
  } else if (repeatOne) {
    newFileNumber = fileNumber;
  } else if (randomized) {
    newFileNumber = random(1, fileCount);
  } else {
    newFileNumber = fileNumber + 1;
    if (newFileNumber > fileCount) {
      newFileNumber = 1;
    }
  }
}

void previous() {
  if (volumeMode) {
    newVolume = volume - 1;
  } else if (repeatOne) {
    newFileNumber = fileNumber;
  } else if (randomized) {
    newFileNumber = random(1, fileCount);
  } else {
    newFileNumber = fileNumber - 1;
    if (newFileNumber < 1) {
      newFileNumber = fileCount;
    }
  }
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
    previous();
    delay(500);
  }

  if (digitalRead(nextButtonPin) == LOW) {
    Serial.println("NEXT pressed");
    next();
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
    randomized = !randomized;
    if (randomized) {
      screen.display("ALEA");
    } else {
      screen.display("LINE");
    }
    delay(500);
  }

  if (digitalRead(sideButtonCPin) == LOW) {
    Serial.println("Side button C pressed");
    repeatOne = !repeatOne;
    if (repeatOne) {
      screen.display("LOOP");
    } else {
      screen.display("NOLO");
    }
    delay(500);
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

  if (player.available()) {
    if (player.readType() == DFPlayerPlayFinished) {
      next();
    }
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
