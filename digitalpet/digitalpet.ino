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

#include "gfx.h";            // PROGMEM Graphics
#include <string.h>
#include <Adafruit_GFX.h>    // Adafruit Core graphics library
#include <Adafruit_TFTLCD.h> // Adafruit Hardware-specific library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// Keyestudio 2.8" LCD Shield Hardware
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

// Matrix Screen Adjustments
#define T_SELS  0.12 // Multiplier (12% of screen)
#define T_SELP  2    // Selector padding (Pixels)
#define T_PIXS  6    // Pixel size (Pixels)
#define T_PIXG  1    // Pixel padding (Pixels)
#define T_BUTP  5    // Touch controls padding (Pixels)

// Timing Settings
#define T_TICK  0.25    // Loops of game code per second
#define T_FPS   2       // Frames per second

// Gameplay Settings
#define AGE_MOVE        64
#define AGE_HATCH      128
#define AGE_MATURE     796
#define AGE_DEATH     8192

#define ENABLE_EAT      32
#define HUNGER_WARNING 128
#define HUNGER_SICK    256
#define HUNGER_DEATH   512

#define ENABLE_SLEEP   150
#define ENERGY_WARNING  64
#define FORCE_SLEEP      8

#define ENABLE_CLEAN    32
#define WASTE_SICK     256


// Animation ID
#define IDLE_EGG        0
#define IDLE_BABY       1
#define IDLE_MATURE     2
#define SLEEP_BABY      3
#define SLEEP_MATURE    4
#define OVERLAY_CLEAN   5
#define OVERLAY_EXLAIM  6
#define OVERLAY_ZZZ     7
#define OVERLAY_DEAD    8
#define OVERLAY_EAT     9
#define OVERLAY_STINK  10
#define DISPLAY_HUNGER 11
#define DISPLAY_ENERGY 12
#define DISPLAY_WASTE  13
#define DISPLAY_AGE    14
#define DISPLAY_BACK   15

struct tama_state {
  bool alive;
  bool eat;
  bool sleep;
  bool stink;
  bool warn;
  bool clean;
};

struct tama_pet {
   uint16_t hunger;
   uint16_t energy;
   uint16_t waste;
   int16_t  happiness;
   uint16_t age;
   uint8_t  stage;
   struct   tama_state state;
};

struct tama_display {
   int8_t selector;
   bool   in_stat;
   int8_t sel_stat;
   int8_t offset;
   int8_t animation;
   int8_t aframe;
   int8_t overlay;
   int8_t oframe;
};

struct tama_pet pet;
struct tama_display tdisp;

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen touch = TouchScreen(TOUCH_XP, TOUCH_YP, TOUCH_XM, TOUCH_YM, TOUCH_OHM);

// Debounce Variables
int  btn_cstate[3] = {0, 0, 0}; // Current button state
int  btn_lstate[3] = {0, 0, 0}; // Previous button state
long btn_tstate[3] = {0, 0, 0}; // Time of last state change

// Display Variables
int lcd_w = 0; // LCD Width
int lcd_h = 0; // LCD Height

// System Timing Variables
long lastTick = 0;
long lastFrame = 0;

void setup(void) {
  Serial.begin(115200);
  tft.reset();
  
  Serial.print(F("LCD Driver: "));
  uint16_t identifier = tft.readID();
  switch(identifier) {
    case 0x9325:
      Serial.println(F("ILI9325"));
      break;
    case 0x9328:
      Serial.println(F("ILI9328"));
      break;
    case 0x9341:
      Serial.println(F("ILI9341"));
      break;
    case 0x7575:
      Serial.println(F("HX8347G"));
      break;
    case 0x8357:
      Serial.println(F("HX8357D"));
      break;
    default:
      Serial.print(F("Unknown ("));
      Serial.print(identifier, HEX);
      Serial.println(")");
      while(1) {
        delay(100);
      }
      //Serial.println("Forcing ILI9341 Driver!");
      //identifier = 0x9341;
  }

  lcd_w = tft.width();
  lcd_h = tft.height();
  Serial.print(F("Resolution: "));
  Serial.print(lcd_w);
  Serial.print(F("x"));
  Serial.println(lcd_h);
  tft.begin(identifier);
  
  // Draw in-active region
  drawInactive();

  // Test Display
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 4; x++) {
      pixbuf[y][x] = 0xFF;
    }
  }
  drawPixels();
  delay(500);
  
  clearPixels();
  drawPixels();

  // Init pet
  libtama_init();
}

void drawInactive() {
  tft.fillScreen(tft.color565(160, 178, 129)); // (160, 178, 129)
  tamaButtons();
  tamaSelectorIn();
  tamaSelector(0);
}

void tamaSelectorIn() {
  uint8_t b;
  int h = T_SELP;
  int w = round((lcd_w / 4) / 3.1);
  int x, y;
  for (y = 0; y < 32; y++) {
    for (x = 0; x < 4; x++) {
      b = reverse(pgm_read_byte(&(gfx_feedIcon[y][x])));
      drawTFTraw(b, w + x * 8, h + y, 1);

      b = reverse(pgm_read_byte(&(gfx_flushIcon[y][x])));
      drawTFTraw(b, (w * 2 + (w * 2)) + x * 8, h + y, 1);

      b = reverse(pgm_read_byte(&(gfx_healthIcon[y][x])));
      drawTFTraw(b, (w * 3 + (w * 4)) + x * 8, h + y, 1);

      b = reverse(pgm_read_byte(&(gfx_zzzIcon[y][x])));
      drawTFTraw(b, (w * 4 + (w * 6)) + x * 8, h + y, 1);
    }
  }
}

void tamaSelector(int sel) {
  uint8_t b;
  int h = T_SELP;
  int w = round((lcd_w / 4) / 3.1);
  int x, y;
  for (y = 0; y < 32; y++) {
    for (x = 0; x < 4; x++) {      
      b = reverse(pgm_read_byte(&(gfx_selectorIcon[y][x])));

      drawTFTinv(b, w + x * 8, h + y, 0);
      drawTFTinv(b, (w * 2 + (w * 2)) + x * 8, h + y, 0);
      drawTFTinv(b, (w * 3 + (w * 4)) + x * 8, h + y, 0);
      drawTFTinv(b, (w * 4 + (w * 6)) + x * 8, h + y, 0);
      
      switch(sel) {
        case 4:
          tdisp.selector = 0;
        case 0:
          drawTFTraw(b, w + x * 8, h + y, 0);
          break;
        case 1:
          drawTFTraw(b, (w * 2 + (w * 2)) + x * 8, h + y, 0);
          break;
        case 2:
          drawTFTraw(b, (w * 3 + (w * 4)) + x * 8, h + y, 0);
          break;
        case -1:
          tdisp.selector = 3;
        case 3:
          drawTFTraw(b, (w * 4 + (w * 6)) + x * 8, h + y, 0);
          break;
      }
    }
  }
}

void drawTFTinv(uint8_t pix, int16_t x, int16_t y, bool e) {
  //tft.drawPixel(0, 0, tft.color565(10, 12, 6));
  for (int b = 7; b >= 0; b--) {
    if(pix & (1 << b)) {
      tft.drawPixel(x + (7 - b), y, tft.color565(156, 170, 125));
    } else if(e) {
      tft.drawPixel(x + (7 - b), y, tft.color565(10, 12, 6));
    }
  }
}

void drawTFTraw(uint8_t pix, int16_t x, int16_t y, bool e) {
  //tft.drawPixel(0, 0, tft.color565(10, 12, 6));
  for (int b = 7; b >= 0; b--) {
    if(pix & (1 << b)) {
      tft.drawPixel(x + (7 - b), y, tft.color565(10, 12, 6));
    } else if(e) {
      tft.drawPixel(x + (7 - b), y, tft.color565(156, 170, 125));
    }
  }
}

void tamaButtons() {
  int y = round(lcd_h * T_SELS) + ((T_PIXS + T_PIXG) * 32) + T_BUTP;
  int h = round((lcd_h - y) / 2);
  int w = round((lcd_w / 3) / 2);
  tft.fillCircle(w, y + h - 8, 6, tft.color565(200, 33, 44));
  tft.fillCircle(w * 2 + w, y + h + 2, 6, tft.color565(200, 33, 44));
  tft.fillCircle(w * 3 + (w * 2), y + h - 8, 6, tft.color565(200, 33, 44)); 
  tft.drawCircle(w, y + h - 8, 6, tft.color565(128, 12, 24));
  tft.drawCircle(w * 2 + w, y + h + 2, 6, tft.color565(128, 12, 24));
  tft.drawCircle(w * 3 + (w * 2), y + h - 8, 6, tft.color565(128, 12, 24));
}

void processTouch() {
  bool btn_istate[3]; // Instantanious state
  int i; // Temp Variable
  int ay = round(lcd_h * T_SELS) + ((T_PIXS + T_PIXG) * 32) + T_BUTP; // Calculate active y
  int bsize = round(lcd_w / 3); // button size
  
  TSPoint p = touch.getPoint(); // Get Touch Point
  pinMode(TOUCH_XM, OUTPUT); // Restore Direction (Shared)
  pinMode(TOUCH_YP, OUTPUT); // Restore Direction (Shared)
  //pinMode(TOUCH_XP, OUTPUT); // Restore Direction
  //pinMode(TOUCH_YM, OUTPUT); // Restore Direction

  /*
  // Helpful for calibration
  Serial.print("Raw = "); Serial.print(p.x);
  Serial.print(" x "); Serial.print(p.y);
  Serial.print(" ; Pressure = "); Serial.println(p.z);
  */
  p.x = map(p.x, TS_MINX, TS_MAXX, lcd_w, 0); // scale from 0->1023 to tft.width()
  p.y = map(p.y, TS_MINY, TS_MAXY, lcd_h, 0); // scale from 0->1023 to tft.height()
  btn_istate[0] = false;
  btn_istate[1] = false;
  btn_istate[2] = false;
  if (p.z > TS_MINP && p.z < TS_MAXP) { // Pressure check
    //Serial.print("X = "); Serial.print(p.x);
    //Serial.print("\tY = "); Serial.print(p.y);
    //Serial.print("\tPressure = "); Serial.println(p.z);

    // Button touch detection
    if (p.y > ay) { // Inside active area
      if (p.x < bsize) { // Button 1
        btn_istate[0] = true;
      } else if (p.x > bsize && p.x < bsize * 2) { // Button 2
        btn_istate[1] = true;
      } else if (p.x > bsize * 2 && p.x < bsize * 3) { // Button 3
        btn_istate[2] = true;
      }
    }
  } else {
    randomSeed(p.x); // Update random seed often
  }
  
  for (i = 0; i < 3; i++) { // Process each button
    if (btn_istate[i] != btn_lstate[i]) {  // if our state has changed
      btn_tstate[i] = millis();      // refresh statetime
    } else if ((millis() - btn_tstate[i]) > TS_DEBO) { // Else if waited longer than DEBOUNCE
      btn_cstate[i] = btn_istate[i]; // Store instantanious state as current

      // Process Button Press/Depress
      switch (i) {
        case 0: // Button 1
          if (btn_cstate[i] && btn_tstate[i] != 0) { // Pressed and not held
            btn_tstate[i] = 0; // Held
            if (tdisp.in_stat) {
              tdisp.sel_stat--;
              if(tdisp.sel_stat < 0) {
                tdisp.sel_stat = 3;
              }
              libtama_display(false);
              gfx_render();
            } else {
              tdisp.selector--;
              if(tdisp.selector < 0) {
                tdisp.selector = 3;
              }
              tamaSelector(tdisp.selector);
            }
          }
          break;
        case 1: // Button 2
          if (btn_cstate[i] && btn_tstate[i] != 0) { // Pressed and not held
            btn_tstate[i] = 0; // Held
            if (tdisp.in_stat) {
              tdisp.in_stat = false;
              libtama_display(false);
              gfx_render();
            } else {
              switch(tdisp.selector) {
                case 0: // Eat
                  libtama_eat();
                  break;
                case 1: // Clean
                  libtama_clean();
                  break;
                case 2: // State Page
                  tdisp.overlay = 0;
                  tdisp.offset = 0;
                  tdisp.in_stat = true;
                  libtama_display(false);
                  gfx_render();
                  break;
                case 3: // Sleep
                  libtama_sleep();
                  break;
              }
            }
          }
          break;
        case 2: // Button 3
          if (btn_cstate[i] && btn_tstate[i] != 0) { // Pressed and not held            
            btn_tstate[i] = 0; // Held
            if (tdisp.in_stat) {
              tdisp.sel_stat++;
              if(tdisp.sel_stat > 3) {
                tdisp.sel_stat = 0;
              }
              libtama_display(false);
              gfx_render();
            } else {
              tdisp.selector++;
              if(tdisp.selector > 3) {
                tdisp.selector = 0;
              }
            }
            tamaSelector(tdisp.selector);
          }
          break;
      }
    }
    btn_lstate[i] = btn_istate[i]; // Save instantanious state as previous
  }
}

void libtama_init(){
  // Pet init
  pet.hunger    = 0;
  pet.energy    = 256;
  pet.waste     = 0;
  pet.age       = 0;
  pet.happiness = 0;
  pet.stage     = 0;
  pet.state.eat    = false;
  pet.state.sleep  = false;
  pet.state.clean  = false;
  pet.state.stink  = false;
  pet.state.warn   = false;
  pet.state.alive  = true;

  // Display state init
  tdisp.selector  = 0; // Select first icon
  tdisp.sel_stat  = 0; // First stat page
  tdisp.offset    = 0;
  tdisp.animation = IDLE_EGG; // Idle Egg
  tdisp.aframe    = 0; // First frame
  tdisp.overlay   = 0; // Disable overlay
  tdisp.oframe    = 0; // Overlay frame
}
void libtama_tick() {
  // Stage Check
  if (pet.stage == 0 && pet.age > AGE_HATCH) {
    pet.stage += 1; // Evolve
  } else if (pet.stage == 1 && pet.age > AGE_MATURE) {
    pet.stage += 1; // Evolve
  }
  
  // State Check
  if (pet.state.eat) {
    pet.hunger = 0;        // Reset hunger
  } else if (pet.state.sleep) {
    pet.energy += 8;       // Regain energy
    if (pet.energy >= 256) {
      pet.state.sleep = false; // Wake up
    }
  } else if(pet.state.alive && !pet.state.sleep) {
    // Random event
    switch(random(31)) {
      case 12:
        pet.hunger += 1; // inc hunger
        break;
      case 16:
        pet.energy -= 1; // Dec energy
        break;
      case 18:
        pet.energy += 1; // Inc energy
        break;
      case 20:
        pet.waste += 1; // Inc waste
        break;
      case 7:
        pet.happiness += 1; // Inc happiness
        break;
      case 4:
        pet.happiness -= 1; // Dec happiness
    }
    // Regular Cycle
    pet.hunger += 1;
    pet.waste  += 1;
    pet.energy -= 1;
    pet.age += 2;
    if (pet.waste >= WASTE_SICK){
      pet.happiness -= 1;
    }
    if (pet.energy < FORCE_SLEEP && pet.stage > 0) {
      pet.happiness -= 64;
      pet.state.sleep = true; // Force to sleep
    }
    // Visual states update
    if (!pet.state.sleep && !pet.state.clean && !pet.state.eat) {
      if (pet.waste >= WASTE_SICK || pet.hunger >= HUNGER_SICK) {
        pet.state.stink = true;  // Enable stink state
      } else {
        pet.state.stink = false; // Disable stink state
      }
      if (pet.energy <= ENERGY_WARNING || pet.hunger >= HUNGER_WARNING || pet.waste >= (WASTE_SICK - WASTE_SICK / 3)) {
        pet.state.warn = true;   // Enable warn state
      } else {
        pet.state.warn = false;  // Disable warn state
      }
      if (pet.hunger >= HUNGER_DEATH || pet.age >= AGE_DEATH) {
        pet.state.alive = false; // No longer alive
      }
    }
  }
}

void loop(void) {
  processTouch(); // Process Touch Events
  //libtama_display(false);
  // Process menu

  // Process frame
  if(millis() - lastFrame > (1000 / T_FPS)) { // If its time for tick
    lastFrame = millis(); // Record last frame time
    gfx_render();
    
    processTouch();
    
    // Check if eating
    if(tdisp.oframe == gfx_frames[OVERLAY_EAT] - 1) { // Last frame check
      pet.state.eat = false; // Stop eating
    }
    if(millis() - lastTick > (1000 / T_TICK)) { // If its time for tick
      Serial.print(F("Debug: AGE:"));
      Serial.print(pet.age);
      Serial.print(F(" STAGE:"));
      Serial.print(pet.stage);
      Serial.print(F(" ENERGY:"));
      Serial.print(pet.energy);
      Serial.print(F(" HUNGER:"));
      Serial.print(pet.hunger);
      Serial.print(F(" WASTE:"));
      Serial.print(pet.waste);
      Serial.print(F(" HAPPINESS:"));
      Serial.println(pet.happiness);
      lastTick = millis(); // Record last tick time
      libtama_tick(); // Execute tick
    }
    libtama_display(true);
  }
  delay(10); // delay 10ms 
}

void gfx_render() {
  clearPixels(); // Clear Pixbuf
  // Load graphics
  if (tdisp.in_stat) {
    switch(tdisp.sel_stat) {
      case 0:
        drawDisplay(DISPLAY_HUNGER);
        break;
      case 1:
        drawDisplay(DISPLAY_ENERGY);
        break;
      case 2:
        drawDisplay(DISPLAY_WASTE);
        break;
      case 3:
        drawDisplay(DISPLAY_AGE);
        break;
      default:
        gfx_loadPM(tdisp.animation, tdisp.aframe); // Get graphics into pixbuf
    }
  } else {
    gfx_loadPM(tdisp.animation, tdisp.aframe); // Get graphics into pixbuf
    if(tdisp.overlay > 0) { // Is an overlay enabled?
      gfx_loadPM(tdisp.overlay, tdisp.oframe); // Get graphics into pixbuf
    }
    gfx_offset(tdisp.offset);
  }
  drawPixels(); // Render
}



void libtama_eat() {
  if (pet.hunger >= ENABLE_EAT && pet.state.alive && !pet.state.sleep && !pet.state.clean && pet.stage > 0) {
    tdisp.oframe = 0;
    pet.state.eat = true;
    libtama_display(false);
    gfx_render();
  }
}

void libtama_sleep() {
  if (pet.energy <= ENABLE_SLEEP && pet.state.alive && !pet.state.eat && !pet.state.clean && pet.stage > 0) {
    tdisp.oframe = 0;
    pet.state.sleep = true;
    libtama_display(true);
    gfx_render();
  }
}

void libtama_clean() {
  if (pet.waste >= ENABLE_CLEAN && pet.state.alive && !pet.state.sleep && !pet.state.eat && pet.stage > 0) {
    tdisp.oframe = 0;
    pet.state.clean = true;
    for (int i = 0; i < 32; i++) {
      tdisp.offset = i;
      libtama_display(false);
      gfx_render();
      delay((2000 / 32) / T_FPS);
    }
    tdisp.offset = 0;
    pet.waste = 0;
    pet.state.clean = false;
  }
  libtama_tick(); // Must tick to catch warning
  libtama_display(false);
}

void libtama_display(bool increment){
  // Increment Frames
  if (increment) {
    if (tdisp.aframe < gfx_frames[tdisp.animation] - 1 && pet.state.alive) {
      tdisp.aframe++;
    } else {
      tdisp.aframe = 0;
    }
    if (tdisp.overlay > 0) {
      if (tdisp.oframe < gfx_frames[tdisp.overlay] - 1) {
        tdisp.oframe++;
      } else {
        tdisp.oframe = 0;
      }
    }
  }
  if (!tdisp.in_stat && pet.state.alive && !pet.state.clean) {
    if(!pet.state.sleep && (pet.age > AGE_MOVE)) { // While not sleeping and not fresh
      tdisp.offset = random(-3, 2);
    }
    switch(pet.stage) {
      case 0:
        tdisp.animation = IDLE_EGG;
        tdisp.overlay = 0;
        break;
      case 1:
        tdisp.animation = IDLE_BABY;
        tdisp.overlay = 0;
        if(pet.state.sleep) {
          tdisp.animation = SLEEP_BABY;
          tdisp.overlay = OVERLAY_ZZZ;
        } else if(pet.state.eat) {
          tdisp.overlay = OVERLAY_EAT;
        } else {
          if(pet.state.stink) {
            tdisp.overlay = OVERLAY_STINK;
          } else if(pet.state.warn) {
            tdisp.overlay = OVERLAY_EXLAIM;
          }
        }
        break;
      case 2:
        tdisp.animation = IDLE_MATURE;
        tdisp.overlay = 0;
        if(pet.state.sleep) {
          tdisp.animation = SLEEP_MATURE;
          tdisp.overlay = OVERLAY_ZZZ;
        } else if(pet.state.eat) {
          tdisp.overlay = OVERLAY_EAT;
        } else {
          if(pet.state.stink) {
            tdisp.overlay = OVERLAY_STINK;
          } else if(pet.state.warn) {
            tdisp.overlay = OVERLAY_EXLAIM;
          }
        }
        break;
    }
  } else if(tdisp.in_stat) {
    tdisp.overlay = 0;
    tdisp.animation = DISPLAY_HUNGER + tdisp.sel_stat;
  } else if (!pet.state.alive) {
    if (pet.stage == 1) {
      tdisp.animation = SLEEP_BABY;
      tdisp.overlay = OVERLAY_DEAD;
    } else {
      tdisp.animation = SLEEP_MATURE;
      tdisp.overlay = OVERLAY_DEAD;
    }
  } else if(pet.state.clean) {
    tdisp.overlay = OVERLAY_CLEAN;
  }
}

void gfx_offset(int8_t o) {
  int8_t x, y;
  for (y = 0; y < 32; y++) {
    if(o < 0) { // Shift Left
      for (x = 0; x < 32; x++) {
        if(x - o < 32) {
          setPixel(x, y, getPixel(x - o, y)); // Copy
        } else {
          setPixel(x, y, false); // Null
        }
      }
    } else if(o > 0) { // Shift Right
      for (x = 31; x >= 0; x--) {
        if(x - o >= 0) {
          setPixel(x, y, getPixel(x - o, y)); // Copy
        } else {
          setPixel(x, y, false); // Null
        }
      }
    }
  }
}

void gfx_loadPM(int id, int frame) {
  int x, y;
  uint8_t t;
  for (y = 0; y < 32; y++) {
    for (x = 0; x < 4; x++) {
      switch(id) {
        case IDLE_EGG: // Idle Egg [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_idleEgg[frame][y][x])));
          break;
        case IDLE_BABY: // Idle Baby [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_idleBaby[frame][y][x])));
          break;
        case IDLE_MATURE: // Idle Mature [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_idleMature[frame][y][x])));
          break;
        case SLEEP_BABY: // Sleep Baby [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_sleepBaby[frame][y][x])));
          break;
        case SLEEP_MATURE: // Sleep Mature [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_sleepMature[frame][y][x])));
          break;
        case OVERLAY_CLEAN: // Overlay Clean [1 frame]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_overlayClean[frame][y][x])));
          break;
        case OVERLAY_EXLAIM: // Overlay Exlaim [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_overlayExlaim[frame][y][x])));
          break;
        case OVERLAY_ZZZ: // Overlay Zzz [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_overlayZzz[frame][y][x])));
          break;
        case OVERLAY_DEAD: // Overlay Dead [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_overlayDead[frame][y][x])));
          break;
        case OVERLAY_EAT: // Overlay Eat [6 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_overlayEat[frame][y][x])));
          break;
        case OVERLAY_STINK: // Overlay Stink [2 frames]
          pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_overlayStink[frame][y][x])));
          break;
      }
    }
  }
}


void drawPixels() {
  int cx = round((lcd_w - ((T_PIXS + T_PIXG) * 32)) / 2);
  int cy = round(lcd_h * T_SELS);
  int px, py, ppx;

  for(py = 0; py < 32; py++) {
    for(px = 0; px < 32; px++) {
      if (px < 8){
        ppx = 0;
      } else if (px < 16){
        ppx = 1;
      } else if (px < 24){
        ppx = 2;
      } else {
        ppx = 3;
      }
      if ((pixbuf[py][ppx] & (1 << (7 - (px - ppx * 8)))) != (disp[py][ppx] & (1 << (7 - (px - ppx * 8))))) { // Do we need to update this pixel?
        if(pixbuf[py][ppx] & (1 << (7 - (px - ppx * 8)))) {
          tft.fillRect(cx + (px * (T_PIXS + T_PIXG)), cy + (py * (T_PIXS + T_PIXG)), T_PIXS, T_PIXS, tft.color565(10, 12, 6));
        } else {
          tft.fillRect(cx + (px * (T_PIXS + T_PIXG)), cy + (py * (T_PIXS + T_PIXG)), T_PIXS, T_PIXS, tft.color565(156, 170, 125));
        }
      }
    }
    disp[py][0] = pixbuf[py][0];
    disp[py][1] = pixbuf[py][1];
    disp[py][2] = pixbuf[py][2];
    disp[py][3] = pixbuf[py][3];
  }
}

void setPixel(int x, int y, bool v) {
  int px;
  if (x < 8){
    px = 0;
  } else if (x < 16){
    px = 1;
  } else if (x < 24){
    px = 2;
  } else {
    px = 3;
  }
  if(v) {
    pixbuf[y][px] |= 1 << (7 - (x - px * 8));
  } else {
    pixbuf[y][px] &= ~(1 << (7 - (x - px * 8)));
  }
}

bool getPixel(int x, int y) {
  int px;
  if (x < 8){
    px = 0;
  } else if (x < 16) {
    px = 1;
  } else if (x < 24) {
    px = 2;
  } else {
    px = 3;
  }
  return(pixbuf[y][px] & (1 << (7 - (x - px * 8))));
}

void clearPixels() {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 4; x++) {
      pixbuf[y][x] = 0x00;
    }
  }
}

// https://stackoverflow.com/questions/2602823/
// Reverse right to left binary to left to right
uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void loadGlyph35(char c, int x, int y) {
  int yy, yb, xb, bb, b; // Y track, y byte, x byte, begin bit
  const uint8_t w = 3, h = 5; // 3 Bits wide, 5 bits deep
  switch(c) {
    case 'A':
    case 'a':
      loadXGlyph35(x, y, 3, 5, 0, 0, 0);
      break;
    case 'B':
    case 'b':
      loadXGlyph35(x, y, 3, 5, 3, 0, 0);
      break;
    case 'C':
    case 'c':
      loadXGlyph35(x, y, 3, 5, 6, 0, 0);
      break;
    case 'D':
    case 'd':
      loadXGlyph35(x, y, 3, 5, 1, 1, 0);
      break;
    case 'E':
    case 'e':
      loadXGlyph35(x, y, 3, 5, 4, 1, 0);
      break;
    case 'F':
    case 'f':
      loadXGlyph35(x, y, 3, 5, 7, 1, 0);
      break;
    case 'G':
    case 'g':
      loadXGlyph35(x, y, 3, 5, 2, 2, 0);
      break;
    case 'H':
    case 'h':
      loadXGlyph35(x, y, 3, 5, 5, 2, 0);
      break;
    case 'I':
    case 'i':
      loadXGlyph35(x, y, 3, 5, 0, 3, 0);
      break;
    case 'J':
    case 'j':
      loadXGlyph35(x, y, 3, 5, 3, 3, 0);
      break;
    case 'K':
    case 'k':
      loadXGlyph35(x, y, 3, 5, 0, 0, 1);
      break;
    case 'L':
    case 'l':
      loadXGlyph35(x, y, 3, 5, 3, 0, 1);
      break;
    case 'M':
    case 'm':
      loadXGlyph35(x, y, 3, 5, 6, 0, 1);
      break;
    case 'N':
    case 'n':
      loadXGlyph35(x, y, 3, 5, 1, 1, 1);
      break;
    case 'O':
    case 'o':
      loadXGlyph35(x, y, 3, 5, 4, 1, 1);
      break;
    case 'P':
    case 'p':
      loadXGlyph35(x, y, 3, 5, 7, 1, 1);
      break;
    case 'Q':
    case 'q':
      loadXGlyph35(x, y, 3, 5, 2, 2, 1);
      break;
    case 'R':
    case 'r':
      loadXGlyph35(x, y, 3, 5, 5, 2, 1);
      break;
    case 'S':
    case 's':
      loadXGlyph35(x, y, 3, 5, 0, 3, 1);
      break;
    case 'T':
    case 't':
      loadXGlyph35(x, y, 3, 5, 3, 3, 1);
      break;
    case 'U':
    case 'u':
      loadXGlyph35(x, y, 3, 5, 0, 0, 2);
      break;
    case 'V':
    case 'v':
      loadXGlyph35(x, y, 3, 5, 3, 0, 2);
      break;
    case 'W':
    case 'w':
      loadXGlyph35(x, y, 3, 5, 6, 0, 2);
      break;
    case 'X':
    case 'x':
      loadXGlyph35(x, y, 3, 5, 1, 1, 2);
      break;
    case 'Y':
    case 'y':
      loadXGlyph35(x, y, 3, 5, 4, 1, 2);
      break;
    case 'Z':
    case 'z':
      loadXGlyph35(x, y, 3, 5, 7, 1, 2);
      break;
    case '0':
      loadXGlyph35(x, y, 3, 5, 2, 2, 2);
      break;
    case '1':
      loadXGlyph35(x, y, 3, 5, 5, 2, 2);
      break;
    case '2':
      loadXGlyph35(x, y, 3, 5, 0, 3, 2);
      break;
    case '3':
      loadXGlyph35(x, y, 3, 5, 3, 3, 2);
      break;
    case '4':
      loadXGlyph35(x, y, 3, 5, 0, 0, 3);
      break;
    case '5':
      loadXGlyph35(x, y, 3, 5, 3, 0, 3);
      break;
    case '6':
      loadXGlyph35(x, y, 3, 5, 6, 0, 3);
      break;
    case '7':
      loadXGlyph35(x, y, 3, 5, 1, 1, 3);
      break;
    case '8':
      loadXGlyph35(x, y, 3, 5, 4, 1, 3);
      break;
    case '9':
      loadXGlyph35(x, y, 3, 5, 7, 1, 3);
      break;
    case '.':
      loadXGlyph35(x, y, 3, 5, 2, 2, 3);
      break;
    case ',':
      loadXGlyph35(x, y, 3, 5, 5, 2, 3);
      break;
    default:
    case '?':
      loadXGlyph35(x, y, 3, 5, 0, 3, 3);
      break;
    case '!':
      loadXGlyph35(x, y, 3, 5, 3, 3, 3);
      break;
  }
}

void drawDisplay(int id) {
  int xx, yy, px;
  uint8_t p;
  switch(id) {
    case DISPLAY_HUNGER: // Display Hunger
      for (yy = 0; yy < ifo_Hunger[1]; yy++) {
        for (xx = 0; xx < ifo_Hunger[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[2 + yy][px] |= reverse(pgm_read_byte(&(gfx_Hunger[yy][px])));
        }
      }
      for (yy = 0; yy < ifo_Progress[1]; yy++) {
        for (xx = 0; xx < ifo_Progress[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[11 + yy][px] |= pgm_read_byte(&(gfx_Progress[yy][px]));
        }
      }
      p = map(pet.hunger, 0, HUNGER_DEATH, 0, 27);
      break;    
    case DISPLAY_ENERGY: // Display Energy
      for (yy = 0; yy < ifo_Energy[1]; yy++) {
        for (xx = 0; xx < ifo_Energy[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[2 + yy][px] |= reverse(pgm_read_byte(&(gfx_Energy[yy][px])));
        }
      }
      for (yy = 0; yy < ifo_Progress[1]; yy++) {
        for (xx = 0; xx < ifo_Progress[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[11 + yy][px] |= pgm_read_byte(&(gfx_Progress[yy][px]));
        }
      }
      p = map(pet.energy, FORCE_SLEEP, 256, 0, 27);
      break;
    case DISPLAY_WASTE: // Display Waste
      for (yy = 0; yy < ifo_Age[1]; yy++) {
        for (xx = 0; xx < ifo_Age[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[2 + yy][px] |= reverse(pgm_read_byte(&(gfx_Waste[yy][px])));
        }
      }
      for (yy = 0; yy < ifo_Progress[1]; yy++) {
        for (xx = 0; xx < ifo_Progress[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[11 + yy][px] |= pgm_read_byte(&(gfx_Progress[yy][px]));
        }
      }
      p = map(pet.waste, 0, WASTE_SICK, 0, 27);
      break;
    case DISPLAY_AGE: // Display Age
      for (yy = 0; yy < ifo_Age[1]; yy++) {
        for (xx = 0; xx < ifo_Age[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[2 + yy][px] |= reverse(pgm_read_byte(&(gfx_Age[yy][px])));
        }
      }
      for (yy = 0; yy < ifo_Progress[1]; yy++) {
        for (xx = 0; xx < ifo_Progress[0]; xx++) {
          if (xx < 8){
            px = 0;
          } else if (xx < 16) {
            px = 1;
          } else if (xx < 24) {
            px = 2;
          } else {
            px = 3;
          }
          pixbuf[11 + yy][px] |= pgm_read_byte(&(gfx_Progress[yy][px]));
        }
      }
      p = map(pet.age, 0, AGE_DEATH, 0, 27);
      break;
  }
  for(int i = 0; i < p; i++) {
    setPixel(3 + i, 12, true);
    setPixel(3 + i, 13, true);
    setPixel(3 + i, 14, true);
    setPixel(3 + i, 15, true);
    setPixel(3 + i, 16, true); 
  }
  /*
      loadGlyph35('I', 0, 0);
      //loadGlyph35(' ', 4, 0);
      loadGlyph35('g', 5, 0);
      loadGlyph35('o', 9, 0);
      loadGlyph35('t', 13, 0);
      //Space = x+1
      loadGlyph35('m', 18, 0);
      loadGlyph35('y', 22, 0);
      
      loadGlyph35('1', 0, 6);
      loadGlyph35('0', 4, 6);
      loadGlyph35('0', 8, 6);
      //Space = x+1
      loadGlyph35('b', 13, 6);
      loadGlyph35('y', 17, 6);
      loadGlyph35('t', 21, 6);
      loadGlyph35('e', 25, 6);
      loadGlyph35('!', 29, 6);

      loadGlyph35('f', 0, 12);
      loadGlyph35('o', 4, 12);
      loadGlyph35('n', 8, 12);
      loadGlyph35('t', 12, 12);
      //Space = x+1
      loadGlyph35('d', 17, 12);
      loadGlyph35('o', 21, 12);
      loadGlyph35('n', 25, 12);
      loadGlyph35('e', 29, 12);
      break;
      */
}

void loadXGlyph35(int dx, int dy, int w, int h, int bb, int xb, int yb) {
  int xx, yy, b, xc, bc, yc; // Y track, y byte, x byte, begin bit
  for (yy = 0; yy < h; yy++) {
    bc = bb; // current bit = begin bit
    xc = xb; // x current = original x byte
    yc = yb; // y current = y byte (untested)
    
    for (xx = 0; xx < w; xx++) {
      b = pgm_read_byte(&(font_UE3X5[(yc * h) + yy][xc])); // Grab Bytes
      setPixel(dx + xx, dy + yy, (b & (1 << (7 - bc)))); // Set our pixel
      bc++;
      // Prepare for next loop
      if (bc > 7) { // If we are over begin bit 7
        bc = 0; // set 0
        xc++; // increment x byte
        if (xc > 3) { // If we are over byte 3
          xc = 0; // set 0
          yc++; // increment x byte
        }
      }
    }
  }
}
