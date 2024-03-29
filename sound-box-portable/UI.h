#include "epd1in54_V2.h"
#include "epdpaint.h"

#ifndef UI_h
#define UI_h
#define COLORED     0
#define UNCOLORED   1

class UI {
  public:
    UI(Epd epd);
    ~UI();
    void DisplayBitmap(const char* filePath);
    void DisplayModeTitle(const char* modeTitle);
    void DisplayCurrentVolume(int currentVolume);
    void DisplayFolderNumber(char* folderNumber);
    void DisplayTrackNumber(char* trackNumber);
    void DisplayTrack(const char* trackTitle, char* trackNumber);
    void PressMenu1();
    void ReleaseMenu1();
    
  private:
    Epd _epd;
    bool _menu1Pressed;
    int32_t ReadNbytesInt(File *p_file, int position, byte nBytes);
    unsigned char* GetImageData(const char *filePath, unsigned char* bitmp);
    void DrawMenu();
    void DrawIcon(const char* iconName, int x, int y);
};

#endif
