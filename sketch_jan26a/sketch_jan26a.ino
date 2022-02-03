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

const int batteryReaderPin = A5;
const float tensionMin = 3.6;
const float tensionMax = 4.2;

bool initialized = false;
bool isFirstPlay = true;
bool volumeMode = false;
bool batteryMode = false;
bool storyMode = false;
bool randomized = false;
bool repeatOne = false;
bool playFinished = true;
String sideComboPressed = "";
int volume = 15;
int newVolume = volume;
int fileNumber = 1;
int newFileNumber = fileNumber;
int fileCount = 0;
int fileCountInMusicsFolder = 0;
int fileCountInStoriesFolder = 0;

TM1637 screen(screenCLKPin, screenDIOPin);
SoftwareSerial playerSerial(playerRXPin, playerTXPin);
DFRobotDFPlayerMini player; 

void setupPlayer() {
  playerSerial.begin(9600) ;
  player.begin(playerSerial) ;
  player.setTimeOut(500);
  player.reset();
  player.disableDAC();
}

void setupScreen() {
  screen.begin();
  screen.setBrightness(3);
}

void setupButtons() {
  pinMode(previousButtonPin, INPUT_PULLUP);
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(sideButtonAPin, INPUT_PULLUP);
  pinMode(sideButtonBPin, INPUT_PULLUP);
  pinMode(sideButtonCPin, INPUT_PULLUP);
}

void fetchFileCountInMusicsFolder() {
  while(fileCountInMusicsFolder < 2) {
    fileCountInMusicsFolder = player.readFileCountsInFolder(1);
  }
}

void fetchFileCountInStoriesFolder() {
  while(fileCountInStoriesFolder < 2 || fileCountInStoriesFolder == fileCountInMusicsFolder) {
    fileCountInStoriesFolder = player.readFileCountsInFolder(2);
  }
}

void initialize() {
  screen.display("INIT");
  
  //while(fileCount < 2) {
  //  fileCount = player.readFileCounts();
  //}
  fetchFileCountInMusicsFolder();
  //player.reset();
  //delay(500);
  fetchFileCountInStoriesFolder();
  /*
  screen.display("CHAN");
  delay(500);
  displayNumber(fileCountInMusicsFolder);
  delay(500);
  screen.display("STOR");
  delay(500);
  displayNumber(fileCountInStoriesFolder);
  delay(500);
  //*/
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
    if (storyMode) {
      fileNumber = random(1, fileCountInStoriesFolder);
    } else {
      fileNumber = random(1, fileCountInMusicsFolder);
    }
  } else {
    fileNumber = 1;
  }
  newFileNumber = fileNumber;
  
  if (storyMode) {
    player.playFolder(2, fileNumber);
  } else {
    player.playFolder(1, fileNumber);
  }

  player.volume(volume);
}

void next() {
  if (volumeMode) {
    newVolume = volume + 1;
  } else if (repeatOne) {
    newFileNumber = fileNumber;
  } else if (randomized) {
    if (storyMode) {
      newFileNumber = random(1, fileCountInStoriesFolder);
    } else {
      newFileNumber = random(1, fileCountInMusicsFolder);
    }
  } else {
    newFileNumber = fileNumber + 1;
    if (storyMode) {
      if (newFileNumber > fileCountInStoriesFolder) {
        newFileNumber = 1;
      }
    } else {
      if (newFileNumber > fileCountInMusicsFolder) {
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
      newFileNumber = random(1, fileCountInStoriesFolder);
    } else {
      newFileNumber = random(1, fileCountInMusicsFolder);
    }
  } else {
    newFileNumber = fileNumber - 1;
    if (newFileNumber < 1) {
      if (storyMode) {
        newFileNumber = fileCountInStoriesFolder;
      } else {
        newFileNumber = fileCountInMusicsFolder;
      }
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

void updateSideButtonStates() {
  bool A = false;
  bool B = false;
  bool C = false;
  
  unsigned long timer = millis();
  while(millis() - timer < 50) {
    A = (digitalRead(sideButtonAPin) == LOW);
    B = (digitalRead(sideButtonBPin) == LOW);
    C = (digitalRead(sideButtonCPin) == LOW);

    sideComboPressed = "";
    if (A && B) {
      sideComboPressed = "AB";
    } else if (A) {
      sideComboPressed = "A";
    } else if (B) {
      sideComboPressed = "B";
    } else if (C) {
      sideComboPressed = "C";
    }
  }
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
      screen.display("HIST");
      //delay(500);
    } else {
      screen.display("CHAN");
    }
    delay(1000);
    
    //batteryMode = !batteryMode;
    storyMode = !storyMode;

    firstPlay();
  }

  if (sideComboPressed == "A") {
    if (volumeMode == false) {
      screen.display("UOLU");
      delay(500);
    }
    volumeMode = true;
  } else {
    volumeMode = false;
  }

  if (sideComboPressed == "B") {
    randomized = !randomized;
    if (randomized) {
      screen.display("ALEA");
    } else {
      screen.display("LINE");
    }
    delay(500);
  }

  if (sideComboPressed == "C") {
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

  if (fileNumber != newFileNumber || playFinished) {
    playFinished = false;
    if (storyMode) {
      player.playFolder(2, newFileNumber);
    } else {
      player.playFolder(1, newFileNumber);
    }
    fileNumber = newFileNumber;
    delay(150);
  }

  // Screen
  if (volumeMode == true) {
    displayNumber(volume);  
  } else if (batteryMode) {
    float batteryValue = analogRead(batteryReaderPin);
    float batteryTension = batteryValue * 5.0 / 1023;
    /*
    int minValue = (1023 * tensionMin) / 5; // 400
    int maxValue = (1023 * tensionMax) / 5; // 838
    float percentage = ((batteryValue - minValue) / (maxValue - minValue)) * 100;
    if (percentage > 100) {
      percentage = 100;
    } else if (percentage < 0) {
      percentage = 0;
    }
    int percentageInteger = percentage;
    */
    //displayNumber(percentageInteger);
    int b = batteryValue;
    displayNumber(b); // Max 140 = 4.1V
    //delay(500);
    //displayNumber(batteryTension);
    //delay(500);
    //displayNumber(round(batteryValue * 100));
  } else {
    displayNumber(fileNumber);
  }

  if (player.available()) {
    if (player.readType() == DFPlayerPlayFinished) {
      playFinished = true;
      next();
    }
  }
}
