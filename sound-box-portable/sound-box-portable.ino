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
  player.setTimeOut(500);
  player.reset();
  player.disableDAC();
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
  setupPlayer();
  setupSDCard();
  setupScreen();
  
  delay(1000);
  player.playFolder(1, 2);
  player.volume(19);

  UI.DisplayBitmap();
}

void loop()
{
  bool button1Pressed = digitalRead(BUTTON_1) == LOW;
  bool button2Pressed = digitalRead(BUTTON_2) == LOW;
  bool button3Pressed = digitalRead(BUTTON_3) == LOW;
  bool button4Pressed = digitalRead(BUTTON_4) == LOW;

  if (button1Pressed) {
    Serial.println("BUTTON 1 pressed");
    UI.PressMenu1();
    UI.ReleaseMenu1();
    delay(400);
  }

  if (button2Pressed) {
    Serial.println("BUTTON 2 pressed");
    delay(400);
  }

  if (button3Pressed) {
    Serial.println("BUTTON 3 pressed");
    delay(400);
  }

  if (button4Pressed) {
    Serial.println("BUTTON 4 pressed");
    delay(400);
  }
}
