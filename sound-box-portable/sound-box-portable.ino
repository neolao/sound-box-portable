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
 */
#include <SPI.h>
#include "epd1in54_V2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <stdio.h>
#include <SD.h>
#include <DFRobotDFPlayerMini.h>

#define SD_CS 17

Epd epd;
unsigned char image[1024];
Paint paint(image, 0, 0);

unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1

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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

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

  Serial.println("e-Paper show pic");
  epd.HDirInit();
  epd.Display(IMAGE_DATA);

  //Part display
  epd.HDirInit();
  epd.DisplayPartBaseImage(IMAGE_DATA);

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

  //Serial.println("e-Paper clear and goto sleep");
  //epd.HDirInit();
  //epd.Clear();
  //epd.Sleep();
}

void loop()
{

}
