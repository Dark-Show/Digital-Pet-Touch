/*
 * Digital Pet Touch - Based on Python Emulator by ryesalvador: https://gist.github.com/ryesalvador/e88cb2b4bbe0694d175ef2d7338abd07
 * Copyright (C) 2020 Greg Michalik
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//////////////////////////////
// Tested Working Shields
//////////////////////////////

#ifdef KEYESTUDIO28LCD // Keyestudio 2.8" LCD Shield Hardware
  #define DRIVER_CHOPPED_TFTLCD_ILI932X  // Adafruit Chopped TFTLCD Driver
  #define DEFPIN_ADAFRUIT_TFTLCD         // Default Pinout
  #define DEFCONF_ADAFRUIT_TFTLCD        // Default Configuration
  
  #define DRIVER_ADAFRUIT_TOUCHSCREEN    // Adafruit TouchScreen Driver
  #define DEFPIN_ADAFRUIT_TOUCHSCREEN    // Default Pinout
  #define DEFCONF_ADAFRUIT_TOUCHSCREEN   // Default Configuration
#endif

#ifdef ILI9486_35LCD
  #define DRIVER_MCUFRIEND_KBV // MCUFriend_kbv Drvier
  #define DEFPIN_MCUFRIEND_KBV // Default Pinout
  
  #define DRIVER_ADAFRUIT_TOUCHSCREEN // Adafruit TouchScreen Driver
  // Non-default Pinout
  #define TOUCH_YP    A1 // Pin Y+ (analog only: An) (Shared with LCD)
  #define TOUCH_XM    A2 // Pin M- (analog only: An) (Shared with LCD)
  #define TOUCH_YM     7 // Pin Y-
  #define TOUCH_XP     6 // Pin X+
  #define TOUCH_OHM  300 // Measured touch resistance in Ohms
  #define DEFCONF_ADAFRUIT_TOUCHSCREEN // Default Configuration
#endif

//////////////////////////////
// Unstable Shields
//////////////////////////////

#ifdef ADAFRUIT28LCD // Adafruit 2.8" Arduino LCD Shield [working]
  #define DRIVER_ADAFRUIT_ILI9341   // Adafruit ILI9341 Driver
  #define DEFPIN_ADAFRUIT_ILI9341   // Default Pinout
  
  #define DRIVER_ADAFRUIT_STMPE610  // Adafruit STMPE610 Touch Driver [unstable]
  #define DEFPIN_ADAFRUIT_STMPE610  // Default Pinout
  #define DEFCONF_ADAFRUIT_STMPE610 // Default Configuration
#endif

//////////////////////////////
// Default Configurations / Pinouts
//////////////////////////////

#ifdef DEFPIN_ADAFRUIT_TFTLCD
  // Default Adafruit TFTLCD settings
  #define LCD_CS      A3 // Pin Chip Select (Shared with Touch)
  #define LCD_CD      A2 // Command/Data (Shared with Touch)
  #define LCD_WR      A1 // LCD Write
  #define LCD_RD      A0 // LCD Read
  #define LCD_RESET   A4 // Can alternately just connect to Arduino's reset pin
#endif

#ifdef DEFPIN_ADAFRUIT_ILI9341
  #define LCD_CS 10 // Pin Chip Select
  #define LCD_DC  9 // Command/Data
#endif

#ifdef DEFPIN_MCUFRIEND_KBV
  #define LCD_CS    A3
  #define LCD_RS    A2 // Shared with touch
  #define LCD_WR    A1 // Shared with touch
  #define LCD_RD    A0
  #define LCD_RESET A4
#endif

#ifdef DEFPIN_ADAFRUIT_TOUCHSCREEN
  #define TOUCH_YP    A3 // Pin Y+ (analog only: An) (Shared with LCD)
  #define TOUCH_XM    A2 // Pin M- (analog only: An) (Shared with LCD)
  #define TOUCH_YM     9 // Pin Y-
  #define TOUCH_XP     8 // Pin X+
  #define TOUCH_OHM  300 // Measured touch resistance in Ohms
#endif

#ifdef DEFCONF_ADAFRUIT_TOUCHSCREEN
  #define TS_MINP     10 // Min Input Pressure
  #define TS_MAXP   1000 // Max Input Preasure
  #define TS_MINX    150 // Touch Min X Position
  #define TS_MINY    170 // Touch Min Y Position
  #define TS_MAXX    920 // Touch Max X Position
  #define TS_MAXY    960 // Touch Max Y Position
  #define TS_DEBO     50 // Touch controls debounce (Noisy touch?)
#endif

#ifdef DEFPIN_ADAFRUIT_STMPE610
  #define TOUCH_CS     8 // Touch CS
#endif

#ifdef DEFCONF_ADAFRUIT_STMPE610
  #define TS_MINP     10 // Min Input Pressure
  #define TS_MAXP   1000 // Max Input Preasure
  #define TS_MINX    150 // Touch Min X Position
  #define TS_MINY    130 // Touch Min Y Position
  #define TS_MAXX   3800 // Touch Max X Position
  #define TS_MAXY   4000 // Touch Max Y Position
  #define TS_DEBO     50 // Touch controls debounce (Noisy touch?)
#endif

//////////////////////////////
// Touch Screen Driver Selections
//////////////////////////////

#ifdef DRIVER_ADAFRUIT_TOUCHSCREEN
  #include <TouchScreen.h> // Adafruit touchscreen
  TouchScreen touch = TouchScreen(TOUCH_XP, TOUCH_YP, TOUCH_XM, TOUCH_YM, TOUCH_OHM);
#endif

#ifdef DRIVER_ADAFRUIT_STMPE610
  #include <Adafruit_STMPE610.h> // Adafruit STMPE610 Resistive touchscreen
  Adafruit_STMPE610 touch = Adafruit_STMPE610(TOUCH_CS);
#endif

//////////////////////////////
// Display Driver Selections
//////////////////////////////

#ifdef DRIVER_MCUFRIEND_KBV
  #include <MCUFRIEND_kbv.h> // MCUFriend ILI9486 Driver
  MCUFRIEND_kbv tft = MCUFRIEND_kbv(LCD_CS, LCD_RS, LCD_WR, LCD_RD, LCD_RESET);
#endif

#ifdef DRIVER_ADAFRUIT_ILI9341
  #include <Adafruit_ILI9341.h> // Adafruit ILI9341 Driver
  Adafruit_ILI9341 tft = Adafruit_ILI9341(LCD_CS, LCD_DC);
#endif

#ifdef DRIVER_CHOPPED_TFTLCD_ILI932X
  #include <TFTLCD_ILI932x.h> // Chopped Adafruit ILI932x driver
  TFTLCD_ILI932x tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#endif

#ifdef DRIVER_CHOPPED_TFTLCD_ILI9341
  #include <TFTLCD_ILI9341.h> // Chopped Adafruit ILI9341 driver
  TFTLCD_ILI9341 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#endif

#ifdef DRIVER_CHOPPED_TFTLCD_HX8347G
  #include <TFTLCD_HX8347G.h> // Chopped Adafruit HX8347G driver
  TFTLCD_HX8347G tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#endif

#ifdef DRIVER_CHOPPED_TFTLCD_HX8357D
  #include <TFTLCD_HX8357D.h> // Chopped Adafruit HX8357D driver
  TFTLCD_HX8357D tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#endif
