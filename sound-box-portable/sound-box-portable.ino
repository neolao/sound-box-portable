/**
 * EPAPER_RST  = GPIO 12
 * EPAPER_DC   = GPIO 8
 * EPAPER_BUSY = GPIO 13
 * EPAPER_CS   = GPIO 9
 * EPAPER_CLK  = GPIO 18
 * EPAPER_DIN  = GPIO 19
 * 
 * SD_CS       = GPIO 17
 * SD_SCK      = GPIO 18
 * SD_MOSI     = GPIO 19
 * SD_MISO     = GPIO 16
 * 
 * PLAYER_RX   = GPIO 4
 * PLAYER_TX   = GPIO 5
 * 
 * BUTTON_1    = GPIO 15
 * BUTTON_2    = GPIO 14
 * BUTTON_3    = GPIO 11
 * BUTTON_4    = GPIO 10
 */
#include <SPI.h>
#include <stdio.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "epd1in54_V2.h"
#include "epdpaint.h"
#include "UI.h"

// Wiring
#define SD_CS 17
#define PLAYER_RX_PIN 4
#define PLAYER_TX_PIN 5
#define BUTTON_1 15
#define BUTTON_2 14
#define BUTTON_3 11
#define BUTTON_4 10

// Screen variables
Epd epd;

// Player variables
SoftwareSerial playerSerial(PLAYER_RX_PIN, PLAYER_TX_PIN);
DFRobotDFPlayerMini player;

// State
int currentVolume = 22;
int folderCount; // player.readFolderCounts() doesn't work
int* fileCountInFolders;
int folderNumber = 1;
int trackNumber = 1;
char folderName[2];
char trackName[3];
const char* trackTitle;
char currentImagePath[20] = "";
char nextImagePath[20] = "";
long int nextImageTime = 0;

enum Mode {
  NORMAL,
  VOLUME
};
Mode currentMode;

// UI
UI UI(epd);

void setupButtons() {
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
}

void setupPlayer() {
  playerSerial.begin(9600);
  player.begin(playerSerial);
  player.setTimeOut(2000);
  player.reset();
  player.disableDAC();

  Serial.println("Setup player...");

  // Get folder count
  File folderCountFile = SD.open("folderCount.txt", FILE_READ);
  folderCount = folderCountFile.parseInt();
  folderCountFile.close();
  Serial.print("Folder count: ");
  Serial.println(folderCount);
  /*
  while(folderCount < 1) {
    folderCount = player.readFolderCounts();
    Serial.print("Folder count: ");
    Serial.println(folderCount);
    delay(50);
  }
  */

  fileCountInFolders = new int[folderCount + 1];
  for (int index = 1; index < folderCount + 1; index++) {
    fileCountInFolders[index] = 0;
  }

  // Get last folder
  File lastFolderFile = SD.open("lastFolder.txt", FILE_READ);
  if (lastFolderFile) {
    folderNumber = lastFolderFile.parseInt();
    lastFolderFile.close();
  }
  if (folderNumber > folderCount) {
    folderNumber = 1;
  }

  // Get last track
  File lastTrackFile = SD.open("lastTrack.txt", FILE_READ);
  if (lastTrackFile) {
    trackNumber = lastTrackFile.parseInt();
    lastTrackFile.close();
  }

  // Get volume
  File volumeFile = SD.open("volume.txt", FILE_READ);
  if (volumeFile) {
    currentVolume = volumeFile.parseInt();
    volumeFile.close();
  }
  
  Serial.println("Setup player: Done");
}

void setupSDCard() {
  if (!SD.begin(SD_CS)) {
    // TODO Display error
    while (1);
  }
}

void setupScreen() {
  epd.LDirInit();
  epd.Clear();
}

void setup()
{
  Serial.begin(115200);

  setupButtons();
  setupSDCard();
  setupPlayer();
  setupScreen();

  delay(500);

  bool button4Pressed = digitalRead(BUTTON_4) == LOW;
  if (button4Pressed) {
    currentMode = VOLUME;
    UI.DisplayModeTitle("VOLUME");
    UI.DisplayCurrentVolume(currentVolume);
  } else {
    currentMode = NORMAL;
    playCurrentTrack();
  }
}

void fetchFileCountInCurrentFolder() {
  int fileCountInFolder = fileCountInFolders[folderNumber];
  
  if (fileCountInFolder < 2) {
    while(fileCountInFolder < 2) {
      fileCountInFolder = player.readFileCountsInFolder(folderNumber);
    }
    fileCountInFolders[folderNumber] = fileCountInFolder;

    Serial.print("File count in folder ");
    Serial.print(folderNumber);
    Serial.print(": ");
    Serial.println(fileCountInFolder);
  }
}

void playCurrentTrack() {
  player.playFolder(folderNumber, trackNumber);
  player.volume(currentVolume);

  if (folderNumber < 10) {
    sprintf(folderName, "0%d", folderNumber);
  } else {
    sprintf(folderName, "%d", folderNumber);
  }

  if (trackNumber < 10) {
    sprintf(trackName, "00%d", trackNumber);
  } else if (trackNumber < 100) {
    sprintf(trackName, "0%d", trackNumber);
  } else {
    sprintf(trackName, "%d", trackNumber);
  }

  UI.DisplayTrackNumber(trackName);
  
  sprintf(nextImagePath, "%s/%s.bmp", folderName, trackName);
  nextImageTime = millis();

  // Save folder
  File lastFolderFile = SD.open("lastFolder.txt", O_TRUNC | O_WRITE);
  if (lastFolderFile) {
    lastFolderFile.print(folderNumber);
    lastFolderFile.close();
  }

  // Save track
  File lastTrackFile = SD.open("lastTrack.txt", O_TRUNC | O_WRITE);
  if (lastTrackFile) {
    lastTrackFile.print(trackNumber);
    lastTrackFile.close();
  }
}

void playNextTrack() {
  trackNumber++;
  fetchFileCountInCurrentFolder();
  if (trackNumber > fileCountInFolders[folderNumber]) {
    trackNumber = 1;
  }
  playCurrentTrack();
}

void playPreviousTrack() {
  trackNumber--;
  if (trackNumber < 1) {
    fetchFileCountInCurrentFolder();
    trackNumber = fileCountInFolders[folderNumber];
  }
  playCurrentTrack();
}

void playNextFolder() {
  trackNumber = 1;
  folderNumber++;
  if (folderNumber > folderCount) {
    folderNumber = 1;
  }
  playCurrentTrack();
}

void playPreviousFolder() {
  trackNumber = 1;
  folderNumber--;
  if (folderNumber < 1) {
    folderNumber = folderCount;
  }
  playCurrentTrack();
}

void loopVolume() {
  bool button1Pressed = digitalRead(BUTTON_1) == LOW;
  bool button2Pressed = digitalRead(BUTTON_2) == LOW;
  bool button3Pressed = digitalRead(BUTTON_3) == LOW;
  bool button4Pressed = digitalRead(BUTTON_4) == LOW;

  if (button4Pressed) {
    
  }

  if (button3Pressed) {
    
  }

  if (button2Pressed) {
    currentVolume++;
    
    // Save volume
    File volumeFile = SD.open("volume.txt", O_TRUNC | O_WRITE);
    if (volumeFile) {
      volumeFile.print(currentVolume);
      volumeFile.close();
    }

    UI.DisplayCurrentVolume(currentVolume);
  }

  if (button1Pressed) {
    currentVolume--;

    // Save volume
    File volumeFile = SD.open("volume.txt", O_TRUNC | O_WRITE);
    if (volumeFile) {
      volumeFile.print(currentVolume);
      volumeFile.close();
    }

    UI.DisplayCurrentVolume(currentVolume);
  }
}

void loopNormal() {
  bool button1Pressed = digitalRead(BUTTON_1) == LOW;
  bool button2Pressed = digitalRead(BUTTON_2) == LOW;
  bool button3Pressed = digitalRead(BUTTON_3) == LOW;
  bool button4Pressed = digitalRead(BUTTON_4) == LOW;

  if (button4Pressed) {
    //Serial.println("BUTTON 4 pressed");
    playNextFolder();
  }

  if (button3Pressed) {
    //Serial.println("BUTTON 3 pressed");
    playPreviousFolder();
  }

  if (button2Pressed) {
    //Serial.println("BUTTON 2 pressed");
    playPreviousTrack();
  }

  if (button1Pressed) {
    //Serial.println("BUTTON 1 pressed");
    UI.PressMenu1();
    UI.ReleaseMenu1();
    playNextTrack();
  }

  if (player.available() && player.readType() == DFPlayerPlayFinished) {
    playNextTrack();
  }

  if (nextImageTime > 0 && millis() - nextImageTime > 1500 && strcmp(nextImagePath, currentImagePath) != 0) {
    if (!SD.exists(nextImagePath)) {
      sprintf(nextImagePath, "%s/unknown.bmp", folderName);
    }
    strcpy(currentImagePath, nextImagePath);
    UI.DisplayBitmap(currentImagePath);
    //UI.DisplayTrackNumber(trackName);

    /*
    char trackTitleFilePath[20];
    sprintf(trackTitleFilePath, "%s/%s.txt", folderName, trackName);
    if (SD.exists(trackTitleFilePath)) {
      File trackTitleFile = SD.open(trackTitleFilePath);
      trackTitle = trackTitleFile.readStringUntil('\n').c_str();
    } else {
      trackTitle = "";
    }
    UI.DisplayTrack(trackTitle, trackName);
    */
  }
}

void loop() {
  switch (currentMode) {
    case VOLUME:
      loopVolume();
      break;
    default:
      loopNormal();
  }
}
