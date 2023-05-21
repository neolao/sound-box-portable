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
#include "epd1in54_V2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <stdio.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define SD_CS 17
#define PLAYER_RX_PIN 4
#define PLAYER_TX_PIN 5
#define BUTTON_1 15
#define BUTTON_2 14
#define BUTTON_3 11
#define BUTTON_4 10

Epd epd;
unsigned char image[1024];
Paint paint(image, 0, 0);

unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1

SoftwareSerial playerSerial(PLAYER_RX_PIN, PLAYER_TX_PIN);
DFRobotDFPlayerMini player;

void printDirectory(File dir, int numTabs) {
  while (true) {
 
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm * tmstruct = localtime(&cr);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}

void setupButtons() {
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  setupButtons();

  playerSerial.begin(9600);
  player.begin(playerSerial);
  player.setTimeOut(500);
  player.reset();
  player.disableDAC();
  delay(1000);
  player.playFolder(1, 2);
  player.volume(8);

  Serial.print("\nInitializing SD card...");
 
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }
  // 0 - SD V1, 1 - SD V2, or 3 - SDHC/SDXC
  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (SD.type()) {
    case 0:
      Serial.println("SD1");
      break;
    case 1:
      Serial.println("SD2");
      break;
    case 3:
      Serial.println("SDHC/SDXC");
      break;
    default:
      Serial.println("Unknown");
  }
 
  Serial.print("Cluster size:          ");
  Serial.println(SD.clusterSize());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(SD.blocksPerCluster());
  Serial.print("Blocks size:  ");
  Serial.println(SD.blockSize());
 
  Serial.print("Total Blocks:      ");
  Serial.println(SD.totalBlocks());
  Serial.println();
 
  Serial.print("Total Cluster:      ");
  Serial.println(SD.totalClusters());
  Serial.println();
 
  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(SD.fatType(), DEC);
 
  volumesize = SD.totalClusters();
  volumesize *= SD.clusterSize();
  volumesize /= 1000;
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);
 
  Serial.print("Card size:  ");
  Serial.println((float)SD.size()/1000);
 
  FSInfo fs_info;
  SDFS.info(fs_info);
 
  Serial.print("Total bytes: ");
  Serial.println(fs_info.totalBytes);
 
  Serial.print("Used bytes: ");
  Serial.println(fs_info.usedBytes);
 
  File dir =  SD.open("/");
  printDirectory(dir, 0);



  
  Serial.println("e-Paper init and clear");
  epd.LDirInit();
  epd.Clear();

/*
  paint.SetWidth(200);
  paint.SetHeight(24);

  Serial.println("e-Paper paint");
  paint.Clear(COLORED);
  paint.DrawStringAt(30, 4, "Hello world!", &Font16, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 10, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(30, 4, "e-Paper Demo", &Font16, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 30, paint.GetWidth(), paint.GetHeight());

  paint.SetWidth(64);
  paint.SetHeight(64);

  paint.Clear(UNCOLORED);
  paint.DrawRectangle(0, 0, 40, 50, COLORED);
  paint.DrawLine(0, 0, 40, 50, COLORED);
  paint.DrawLine(40, 0, 0, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledRectangle(0, 0, 40, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 130, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 130, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();
  delay(2000);
  */

  Serial.println("e-Paper show pic");
  epd.HDirInit();
  //epd.Display(IMAGE_DATA);
  //Serial.printf("IMAGE DATA Size = %d\n", sizeof(IMAGE_DATA));

  //epd.HDirInit();
  //const unsigned char* bitmp = getImageData("hello.bmp");

  File bmpImage = SD.open("hello.bmp", FILE_READ);
    unsigned int fileSize = bmpImage.size();

    Serial.printf("File Size = %d\n", fileSize);
    

    int32_t dataStartingOffset = readNbytesInt(&bmpImage, 0x0A, 4);

    // Change their types to int32_t (4byte)
    int32_t width = readNbytesInt(&bmpImage, 0x12, 4);
    int32_t height = readNbytesInt(&bmpImage, 0x16, 4);
    Serial.println(width);
    Serial.println(height);

    int16_t pixelsize = readNbytesInt(&bmpImage, 0x1C, 2);

    if (pixelsize != 24)
    {
        Serial.println("Image is not 24 bpp");
        while (1);
    }

    bmpImage.seek(dataStartingOffset);//skip bitmap header

    int32_t bufferWidth = width / 8;

    // 24bpp means you have three bytes per pixel, usually B G R

    byte R, G, B;
    unsigned char bitmp[height * bufferWidth];
    for(int32_t i = 0; i < height; i ++) {
        for (int32_t j = 0; j < bufferWidth; j ++) {
            int32_t pixels = 0;

            // 1
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 128;
            }

            // 2
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 64;
            }

            // 3
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 32;
            }

            // 4
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 16;
            }

            // 5
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 8;
            }

            // 6
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 4;
            }

            // 7
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 2;
            }

            // 8
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            if (B > 0) {
               pixels += 1;
            }

            Serial.print(pixels);
            Serial.print(" ");

            bitmp[i * bufferWidth + j] = pixels;
            
            /*
            Serial.print("R");
            Serial.print(R);
            Serial.print("G");
            Serial.print(G);
            Serial.print("B");
            Serial.print(B);
            Serial.print(" ");
            //*/

   
        }
    }

    bmpImage.close();
  
  epd.Display(bitmp);
  Serial.println("e-Paper show pic DONE");

  //Part display
  //epd.HDirInit();
  //epd.DisplayPartBaseImage(IMAGE_DATA);

  /*
  paint.SetWidth(50);
  paint.SetHeight(60);
  paint.Clear(UNCOLORED);

  char i = 0;
  char str[10][10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  for (i = 0; i < 10; i++) {
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(10, 10, str[i], &Font24, COLORED);
    epd.SetFrameMemoryPartial(paint.GetImage(), 80, 70, paint.GetWidth(), paint.GetHeight());
    epd.DisplayPartFrame();
    delay(100);
  }
  */

  //Serial.println("e-Paper clear and goto sleep");
  //epd.HDirInit();
  //epd.Clear();
  //epd.Sleep();
}

int32_t readNbytesInt(File *p_file, int position, byte nBytes)
{
    if (nBytes > 4)
        return 0;

    p_file->seek(position);

    int32_t weight = 1;
    int32_t result = 0;
    for (; nBytes; nBytes--)
    {
        result += weight * p_file->read();
        weight <<= 8;
    }
    return result;
}

void loop()
{
  bool button1Pressed = digitalRead(BUTTON_1) == LOW;
  bool button2Pressed = digitalRead(BUTTON_2) == LOW;
  bool button3Pressed = digitalRead(BUTTON_3) == LOW;
  bool button4Pressed = digitalRead(BUTTON_4) == LOW;

  if (button1Pressed) {
    Serial.println("BUTTON 1 pressed");
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
