#ifdef KEYESTUDIO28LCD // Keyestudio 2.8" LCD Shield Hardware
  #include <TouchScreen.h>    // Adafruit touchscreen
  #include "TFTLCD_ILI932x.h" // Cutdown Adafruit library
  #define LCD_CS      A3 // Pin Chip Select (Shared with Touch)
  #define LCD_CD      A2 // Command/Data (Shared with Touch)
  #define LCD_WR      A1 // LCD Write
  #define LCD_RD      A0 // LCD Read
  #define LCD_RESET   A4 // Can alternately just connect to Arduino's reset pin
  #define TOUCH_YP    A3 // Pin Y+ (analog only: An) (Shared with LCD)
  #define TOUCH_XM    A2 // Pin M- (analog only: An) (Shared with LCD)
  #define TOUCH_YM     9 // Pin Y-
  #define TOUCH_XP     8 // Pin X+
  #define TOUCH_OHM  300 // Measured touch resistance in Ohms
  #define TS_MINP     10 // Min Input Pressure
  #define TS_MAXP   1000 // Max Input Preasure
  #define TS_MINX    150 // Touch Min X Position
  #define TS_MINY    170 // Touch Min Y Position
  #define TS_MAXX    920 // Touch Max X Position
  #define TS_MAXY    960 // Touch Max Y Position
  #define TS_DEBO     50 // Touch controls debounce (Noisy touch?)

  TouchScreen touch = TouchScreen(TOUCH_XP, TOUCH_YP, TOUCH_XM, TOUCH_YM, TOUCH_OHM);
  TFTLCD_ILI932x tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#endif
