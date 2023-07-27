#include <SD.h>
#include "UI.h"

UI::UI(Epd epd) {
  _epd = epd;
  _menu1Pressed = false;
}
UI::~UI() {
}

void UI::DisplayBitmap(const char *filePath) {
  Serial.print("Display bitmap: ");
  Serial.println(filePath);
  
  _epd.LDirInit();

  unsigned char* bitmp;
  bitmp = GetImageData(filePath, bitmp);
  
  _epd.Display(bitmp);
  //_epd.DisplayPartBaseImage(bitmp);

  //DrawMenu();
}

void UI::DisplayTrackNumber(int trackNumber) {
  unsigned char image[1024];
  Paint paint(image, 0, 0);
  paint.SetRotate(ROTATE_180);
  paint.SetWidth(72);
  paint.SetHeight(24);

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 0, "42", &Font24, COLORED);
  
  _epd.SetFrameMemoryPartial(paint.GetImage(), 200 - paint.GetWidth(), 200 - paint.GetHeight(), paint.GetWidth(), paint.GetHeight());
  _epd.DisplayPartFrame();
}

void UI::DrawMenu() {
  unsigned char image[1024];
  Paint paint(image, 0, 0);
  paint.SetRotate(ROTATE_180);

  // Lines
  paint.SetWidth(24);
  paint.SetHeight(200);
  paint.Clear(UNCOLORED);
  paint.DrawVerticalLine(1, 0, 200, COLORED);
  paint.DrawHorizontalLine(0, 50, 24, COLORED);
  paint.DrawHorizontalLine(0, 100, 24, COLORED);
  paint.DrawHorizontalLine(0, 150, 24, COLORED);
  //_epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());

  _epd.SetFrameMemoryPartial(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  

  // Menu 1
  DrawIcon("arrow-down", 8, 15);

  // Menu 2
  DrawIcon("arrow-up", 8, 70);

  // Menu 3
  DrawIcon("arrow-right", 8, 118);

  // Menu 3
  DrawIcon("arrow-left", 8, 168);

  _epd.DisplayPartFrame();
}

void UI::PressMenu1() {
  _menu1Pressed = true;
}

void UI::ReleaseMenu1() {
  _menu1Pressed = false;
}

void UI::DrawIcon(const char* iconName, int x, int y) {
  char iconFilePath[120];
  strcpy(iconFilePath, "icons/");
  strcat(iconFilePath, iconName);
  strcat(iconFilePath, ".bmp");

  int iconWidth = 16;
  int iconHeight = 16;
  unsigned char* icon;
  icon = GetImageData(iconFilePath, icon);

  _epd.SetFrameMemoryPartial(icon, x, y, iconWidth, iconHeight);
}

int32_t UI::ReadNbytesInt(File *p_file, int position, byte nBytes)
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

unsigned char* UI::GetImageData(const char *filePath, unsigned char* bitmp) {
  Serial.print("GetImageData: ");
  Serial.println(filePath);
  
  File bmpImage = SD.open(filePath, FILE_READ);
  unsigned int fileSize = bmpImage.size();
  int32_t dataStartingOffset = ReadNbytesInt(&bmpImage, 0x0A, 4);

  int32_t width = ReadNbytesInt(&bmpImage, 0x12, 4);
  int32_t height = ReadNbytesInt(&bmpImage, 0x16, 4);

  int16_t pixelsize = ReadNbytesInt(&bmpImage, 0x1C, 2);

  if (pixelsize != 24)
  {
      Serial.print("Image ");
      Serial.print(filePath);
      Serial.println(" is not 24 bpp");
      while (1);
  }

  Serial.print("Image size: ");
  Serial.print(width);
  Serial.print(" x ");
  Serial.println(height);

  bmpImage.seek(dataStartingOffset);//skip bitmap header

  int32_t bufferWidth = width / 8;

  byte R, G, B;
  //unsigned char bitmp[height * bufferWidth];
  bitmp = new unsigned char[height * bufferWidth];
  for (int32_t h = 0; h < height; h++) {
    for (int32_t w = 0; w < bufferWidth; w++) {
      int32_t pixels = 0;
      int32_t threshold = 150;

      // 1
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 1;
      }

      // 2
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 2;
      }

      // 3
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 4;
      }

      // 4
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 8;
      }

      // 5
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 16;
      }

      // 6
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 32;
      }

      // 7
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 64;
      }

      // 8
      B = bmpImage.read();
      G = bmpImage.read();
      R = bmpImage.read();
      if (B > threshold) {
         pixels += 128;
      }

      bitmp[h * bufferWidth + (bufferWidth - w - 1)] = pixels;
    }
  }
  bmpImage.close();

  return bitmp;
}
