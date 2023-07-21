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
int folderCount; // player.readFolderCounts() doesn't work
int* fileCountInFolders;
int folderNumber = 1;
int trackNumber = 1;

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
  
  delay(1000);
  playCurrentTrack();
  player.volume(20);

  //UI.DisplayBitmap();
  UI.DisplayTrackNumber(42);
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

void loop() {
  bool button1Pressed = digitalRead(BUTTON_1) == LOW;
  bool button2Pressed = digitalRead(BUTTON_2) == LOW;
  bool button3Pressed = digitalRead(BUTTON_3) == LOW;
  bool button4Pressed = digitalRead(BUTTON_4) == LOW;

  if (button4Pressed) {
    Serial.println("BUTTON 4 pressed");
    playNextFolder();
    delay(400);
  }

  if (button3Pressed) {
    Serial.println("BUTTON 3 pressed");
    playPreviousFolder();
    delay(400);
  }

  if (button2Pressed) {
    Serial.println("BUTTON 2 pressed");
    playPreviousTrack();
    delay(400);
  }

  if (button1Pressed) {
    Serial.println("BUTTON 1 pressed");
    UI.PressMenu1();
    UI.ReleaseMenu1();
    playNextTrack();
    delay(400);
  }

  if (player.available() && player.readType() == DFPlayerPlayFinished) {
    playNextTrack();
  }
}
