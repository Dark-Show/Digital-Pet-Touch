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

#define KEYESTUDIO28LCD  // Keyestudio 2.8" LCD Shield (ILI9328)
//#define ADAFRUIT28LCD // Adafruit 2.8" Arduino LCD Shield

#include "pet_gfx.h";    // After graphics are defined
#include "digitalpet.h";

void setup(void) {
  Serial.begin(115200);
#ifdef _TFTLCD_ILI932X_H_
  tft.reset();
  
  uint16_t identifier = tft.readID();
  switch(identifier) {
    case 0x9325: // ILI9325
    case 0x9328: // ILI9328
      break;
    /*
    case 0x9341: // ILI9341
      break;
    case 0x7575: // HX8347G
      break;
    case 0x8357: // HX8357D
      break;
    */
    default:
      Serial.print(F("LCD Unknown ("));
      Serial.print(identifier, HEX);
      Serial.println(")");
      
      //while(1) {
      //  delay(100);
      //}
  }
#endif

  lcd_w = tft.width();
  lcd_h = tft.height();
  tft.begin();
  
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

  processTouch(true); // Get some random
  libpet_init(); // Init pet
}

void drawInactive() {
  tft.fillScreen(tft.color565(160, 178, 129)); // (160, 178, 129)
  petButtons();
  petSelectorIn();
  petSelector(0);
}

void petSelectorIn() {
  uint8_t b;
  int h = T_SELP;
  int w = round((lcd_w / 5) / 3.1);
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

      b = pgm_read_byte(&(gfx_ExploreIcon[y][x]));
      drawTFTraw(b, (w * 5 + (w * 8)) + x * 8, h + y, 1);
    }
  }
}

void petSelector(int sel) {
  uint8_t b;
  int h = T_SELP;
  int w = round((lcd_w / 5) / 3.1);
  int x, y;
  for (y = 0; y < 32; y++) {
    for (x = 0; x < 4; x++) {      
      b = reverse(pgm_read_byte(&(gfx_selectorIcon[y][x])));

      drawTFTinv(b, w + x * 8, h + y, 0);
      drawTFTinv(b, (w * 2 + (w * 2)) + x * 8, h + y, 0);
      drawTFTinv(b, (w * 3 + (w * 4)) + x * 8, h + y, 0);
      drawTFTinv(b, (w * 4 + (w * 6)) + x * 8, h + y, 0);
      drawTFTinv(b, (w * 5 + (w * 8)) + x * 8, h + y, 0);
      
      switch(sel) {
        case 0:
          drawTFTraw(b, w + x * 8, h + y, 0);
          break;
        case 1:
          drawTFTraw(b, (w * 2 + (w * 2)) + x * 8, h + y, 0);
          break;
        case 2:
          drawTFTraw(b, (w * 3 + (w * 4)) + x * 8, h + y, 0);
          break;
        case 3:
          drawTFTraw(b, (w * 4 + (w * 6)) + x * 8, h + y, 0);
          break;
        case 4:
          drawTFTraw(b, (w * 5 + (w * 8)) + x * 8, h + y, 0);
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

void petButtons() {
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

void processTouch(bool sr) {
  bool btn_istate[3]; // Instantanious state
  int i; // Temp Variable
  int ay = round(lcd_h * T_SELS) + ((T_PIXS + T_PIXG) * 32) + T_BUTP; // Calculate active y
  int bsize = round(lcd_w / 3); // button size
#if defined(_ADAFRUIT_TOUCHSCREEN_H_) || defined(_ADAFRUIT_STMPE610H_)
#ifdef _ADAFRUIT_STMPE610H_
  TS_Point p = touch.getPoint(); // Get Touch Point
#elif defined(_ADAFRUIT_TOUCHSCREEN_H_)
  TSPoint p = touch.getPoint(); // Get Touch Point

  pinMode(TOUCH_XM, OUTPUT); // Restore Direction (Shared)
  pinMode(TOUCH_YP, OUTPUT); // Restore Direction (Shared)
  //pinMode(TOUCH_XP, OUTPUT); // Restore Direction
  //pinMode(TOUCH_YM, OUTPUT); // Restore Direction
#endif

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
  }
  if(sr) {
    randomSeed(p.x); // Update random seed often
  }
#endif
  
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
                tdisp.sel_stat = 2;
              }
              libpet_display(false);
              gfx_render();
            } else {
              tdisp.selector--;
              if(tdisp.selector < 0) {
                tdisp.selector = 4;
              }
              petSelector(tdisp.selector);
            }
          }
          break;
        case 1: // Button 2
          if (btn_cstate[i] && btn_tstate[i] != 0) { // Pressed and not held
            btn_tstate[i] = 0; // Held
            if (tdisp.in_stat) {
              tdisp.in_stat = false;
              libpet_display(false);
              gfx_render();
            } else {
              switch(tdisp.selector) {
                case 0: // Eat
                  libpet_eat();
                  break;
                case 1: // Clean
                  libpet_clean();
                  break;
                case 2: // Stats Page
                  if(!pet.state.explore) { // FIXME: allow stats page inside explore game
                    tdisp.overlay = 0;
                    tdisp.offset = 0;
                    tdisp.in_stat = true;
                    libpet_display(false);
                    gfx_render();
                  }
                  break;
                case 3: // Sleep
                  libpet_sleep();
                  break;
                case 4: // RPG
                  libpet_explore();
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
              if(tdisp.sel_stat > 2) {
                tdisp.sel_stat = 0;
              }
              libpet_display(false);
              gfx_render();
            } else {
              tdisp.selector++;
              if(tdisp.selector > 4) {
                tdisp.selector = 0;
              }
            }
            petSelector(tdisp.selector);
          }
          break;
      }
    }
    btn_lstate[i] = btn_istate[i]; // Save instantanious state as previous
  }
}

void libpet_init(){
  // Pet init
  pet.hunger    = 0;
  pet.energy    = 256;
  pet.waste     = 0;
  pet.age       = 0;
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

  // RPG stats
  pet.rpg.coins      = COIN_DEFAULT;
  pet.rpg.attack     = random(0, 4);
  pet.rpg.defense    = random(0, 4);
  pet.rpg.luck       = random(0, 8);
  pet.rpg.experience = 0;
  pet.rpg.level      = 0;
}
void libpet_tick() {
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
      case 4:
      case 7:
        pet.rpg.coins += 1; // small inc coin
        break;
      case 8:
        pet.rpg.coins += 5; // medium inc coin
        break;
      case 9:
        pet.rpg.coins += 10; // large inc coin
        break;
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
    }
    // Regular Cycle
    if (pet.stage > 0) {
      pet.hunger += 1;
      pet.waste  += 1;
    }
    pet.energy -= 1;
    pet.age += 2;
    if (pet.energy < FORCE_SLEEP && pet.stage > 0) {
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
  processTouch(true); // Process Touch Events
  //libpet_display(false);
  // Process menu
  if(millis() - lastTick > (1000 / T_TICK)) { // If its time for tick
    /*
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
    */
    lastTick = millis(); // Record last tick time
    libpet_tick(); // Execute tick
  }
  // Process frame
  if(millis() - lastFrame > (1000 / T_FPS)) { // If its time for tick
    lastFrame = millis(); // Record last frame time
    gfx_render();
    
    processTouch(true);
    
    // Check if eating
    if(tdisp.oframe == gfx_frames[OVERLAY_EAT] - 1) { // Last frame check
      pet.hunger = 0;
      pet.state.eat = false; // Stop eating
      libpet_tick(); // Execute tick
    }
    libpet_display(true);
  }
  delay(10); // delay 10ms 
}

void libpet_eat() {
  if (pet.hunger >= ENABLE_EAT && pet.state.alive && !pet.state.sleep && !pet.state.clean && !pet.state.eat && pet.stage > 0 && !pet.state.explore) {
    if (pet.stage >= 2) { // if adult
      if (pet.rpg.coins < EAT_ADU_COST) { // Cant afford to eat
        // Show No Coin animation
        return;
      }
      pet.rpg.coins -= EAT_ADU_COST; // subtract cost of meal
    }
    tdisp.oframe = 0;
    pet.state.eat = true;
    libpet_display(false);
    gfx_render();
  }
}

void libpet_sleep() {
  if (pet.energy <= ENABLE_SLEEP && pet.state.alive && !pet.state.eat && !pet.state.clean && pet.stage > 0 && !pet.state.explore) {
    tdisp.oframe = 0;
    pet.state.sleep = true;
    libpet_display(true);
    gfx_render();
  }
}

void libpet_clean() {
  if (pet.waste >= ENABLE_CLEAN && !pet.state.clean && pet.state.alive && !pet.state.sleep && !pet.state.eat && pet.stage > 0 && !pet.state.explore) {
    pet.state.clean = true;
    tdisp.oframe = 0;
    //tdisp.overlay = OVERLAY_CLEAN;
    tdisp.offset = 0;
    libpet_display(false);
    gfx_render();
    doShiftTransition(1);
    pet.waste = 0;
    pet.state.clean = false;
    libpet_tick(); // Must tick to catch warning
  }
  libpet_display(false);
}

void libpet_display(bool increment){
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
    tdisp.animation = DISPLAY_STAT1 + tdisp.sel_stat;
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

void doOffset(int8_t o) {
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

void drawPixels() {
  int cx = round((lcd_w - ((T_PIXS + T_PIXG) * 32)) / 2);
  int cy = round(lcd_h * T_SELS);
  int px, py, ppx;

  for(py = 0; py < 32; py++) {
    for(px = 0; px < 32; px++) {
      ppx = calculateXByte(px);
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
  int px = calculateXByte(x);
  if(v) {
    pixbuf[y][px] |= 1 << (7 - (x - px * 8));
  } else {
    pixbuf[y][px] &= ~(1 << (7 - (x - px * 8)));
  }
}

bool getPixel(int x, int y) {
  int px = calculateXByte(x);
  return(pixbuf[y][px] & (1 << (7 - (x - px * 8))));
}

void clearPixels() {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 4; x++) {
      pixbuf[y][x] = 0x00;
    }
  }
}

void fillPixels() {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 4; x++) {
      pixbuf[y][x] = 0xFF;
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

void gfx_render() {
  clearPixels(); // Clear Pixbuf
  // Load graphics
  if (tdisp.in_stat) {
    drawDisplay(tdisp.sel_stat); // Draw status display screens
  } else {
    drawAnimation(tdisp.animation, tdisp.aframe); // Draw animation frame
    if(tdisp.overlay > 0) { // Is an overlay enabled?
      if(tdisp.overlay == OVERLAY_STINK)
        doOffset(random(-1, 1));
      drawOverlay(tdisp.overlay, tdisp.oframe); // Get graphics into pixbuf
    }
    if(tdisp.overlay != OVERLAY_CLEAN)
      doOffset(tdisp.offset);
  }
  //drawOverlay(OVERLAY_STINK, 0);
  drawPixels(); // Render
}

void drawZigzag (int x, int y, int o, int h) {
  for (int yy = y; yy < y + h; yy++) {
    if (o) {
        setPixel(x, yy, 1);
        setPixel(x + 1, yy, 0);
        o = 0;
    } else {
        setPixel(x, yy, 0);
        setPixel(x + 1, yy, 1);
        o = 1;
    }
  }
}

void drawOverlay(int id, int frame) {
  int x, y;

  if (id == OVERLAY_STINK) {
    drawZigzag( 2,  5 - frame, 0, 6); // Left
    drawZigzag( 5,  7 - frame, 1, 6); // Left
    drawZigzag(24,  7 - frame, 0, 6); // Right
    drawZigzag(27,  4 - frame, 0, 6); // Right
    drawZigzag(28, 11 - frame, 1, 5); // Right
  } else if (id == OVERLAY_EAT) {
    // Draw apple
    rectPixels(20, 9, 7, 6, 1, 1);
    setPixel(20, 14, 0); // rounding
    setPixel(21, 11, 0); // shine dot
    setPixel(23,  9, 0); // indent
    // stem
    setPixel(25, 8, 1);
    setPixel(26, 7, 1);
    linePixels(22, 15, 25, 15, 1); // bottom
    linePixels(27, 10, 27, 12, 1); // right side

    // Eat the apple
    switch(frame) {
      case 4:
        rectPixels(20, 9, 8, 7, 0, 1);
        setPixel(24, 9, 1);
        break;
      case 3:
        rectPixels(20, 9, 4, 5, 0, 1);
        setPixel(21, 14, 0);
        setPixel(22, 14, 0);
        setPixel(24, 11, 0);
        setPixel(24, 12, 0);
        break;
      case 2:
        rectPixels(20, 9, 2, 5, 0, 1);
        rectPixels(21, 10, 2, 3, 0, 1);
        break;
      case 1:
        linePixels(20, 10, 20, 12, 0);
      default:
        break;
    }
  } else if (id == OVERLAY_CLEAN) {
    linePixels(1, 0, 1, 31, 1);
  } else if (id == OVERLAY_EXLAIM) {
    if (frame == 0) {
      // Draw top of mark
      linePixels(4, 2, 4, 8, 1);
      linePixels(5, 1, 5, 9, 1);
      linePixels(6, 2, 6, 8, 1);

      // Replace with 35font plus?
      linePixels(5, 11, 5, 13, 1);
      linePixels(4, 12, 6, 12, 1);
    }
  } else if (id == OVERLAY_DEAD) {
    // Skull
    rectPixels(22 - frame, 2, 5, 6, 1, 1); // center
    linePixels(21 - frame, 3, 21 - frame, 6, 1); // L side
    linePixels(27 - frame, 3, 27 - frame, 6, 1); // R side
            
    // Eye Holes
    setPixel(23 - frame, 4, 0);
    setPixel(25 - frame, 4, 0);  
                 
    // Teeth
    setPixel(22 - frame, 8, 1);
    setPixel(24 - frame, 8, 1);
    setPixel(26 - frame, 8, 1); 
  } else {
    for (y = 0; y < 32; y++) {
      for (x = 0; x < 4; x++) {
        switch(id) {
          case OVERLAY_ZZZ: // Overlay Zzz [2 frames]
            pixbuf[y][x] |= reverse(pgm_read_byte(&(gfx_overlayZzz[frame][y][x])));
            break;
        }
      }
    }
  }
}

void drawAnimation(int id, int frame) {
  int xx, yy, px;
  switch(id) {
    case IDLE_EGG:
      for (yy = 0; yy < 12; yy++) {
        for (xx = 0; xx < 32; xx++) {
          px = calculateXByte(xx);
          if (frame == 0) {
            pixbuf[19 + yy][px] |= reverse(pgm_read_byte(&(gfx_idleEgg[0][yy][px])));
          } else {
            pixbuf[19 + yy][px] |= reverse(pgm_read_byte(&(gfx_idleEgg[1][yy][px])));
          }
        }
      }
      break;
    case IDLE_BABY:
      for (yy = 0; yy < 7; yy++) {
        for (xx = 0; xx < 32; xx++) {
          px = calculateXByte(xx);
          if(frame == 0) {
            pixbuf[24 + yy][px] |= reverse(pgm_read_byte(&(gfx_idleBaby[0][yy][px])));
          } else {
            pixbuf[24 + yy][px] |= reverse(pgm_read_byte(&(gfx_idleBaby[1][yy][px])));
          }
        }
      }
      break;
    case IDLE_MATURE:
      for (yy = 0; yy < 10; yy++) {
        for (xx = 0; xx < 32; xx++) {
          px = calculateXByte(xx);
          if(frame == 0) {
            pixbuf[9 + yy][px] |= reverse(pgm_read_byte(&(gfx_idleMature[0][yy][px])));
          } else {
            pixbuf[9 + yy][px] |= reverse(pgm_read_byte(&(gfx_idleMature[1][yy][px])));
          }
        }
      }
      break;
    case SLEEP_BABY:
      for (yy = 0; yy < 3; yy++) {
        for (xx = 0; xx < 32; xx++) {
          px = calculateXByte(xx);
          if(frame == 0) {
            pixbuf[28 + yy][px] |= reverse(pgm_read_byte(&(gfx_sleepBaby[0][yy][px])));
          } else {
            pixbuf[28 + yy][px] |= reverse(pgm_read_byte(&(gfx_sleepBaby[1][yy][px])));
          }
        }
      }
      break;
    case SLEEP_MATURE:
      for (yy = 0; yy < 4; yy++) {
        for (xx = 0; xx < 32; xx++) {
          px = calculateXByte(xx);
          if(frame == 0) {
            pixbuf[27 + yy][px] |= reverse(pgm_read_byte(&(gfx_sleepMature[0][yy][px])));
          } else {
            pixbuf[27 + yy][px] |= reverse(pgm_read_byte(&(gfx_sleepMature[1][yy][px])));
          }
        }
      }
      break;
  }
}

void drawDisplay(int id) {
  int xx, yy, px;
  uint8_t p;
  char snum[5];
  uint8_t c = 0, i = 0;
  
  switch(id) {
    case DISPLAY_STAT1: // Display Hunger
      loadGlyph35('h',  4, 2);
      loadGlyph35('u',  8, 2);
      loadGlyph35('n', 12, 2);
      loadGlyph35('g', 16, 2);
      loadGlyph35('e', 20, 2);
      loadGlyph35('r', 24, 2);
      drawProgress (pet.hunger, 0, HUNGER_DEATH, 9);
      loadGlyph35('w',  6, 18);
      loadGlyph35('a', 10, 18);
      loadGlyph35('s', 14, 18);
      loadGlyph35('t', 18, 18);
      loadGlyph35('e', 22, 18);
      drawProgress (pet.waste, 0, WASTE_SICK, 25);
      break;
    case DISPLAY_STAT2: // Display Energy
      loadGlyph35('e',  4, 2);
      loadGlyph35('n',  8, 2);
      loadGlyph35('e', 12, 2);
      loadGlyph35('r', 16, 2);
      loadGlyph35('g', 20, 2);
      loadGlyph35('y', 24, 2);
      drawProgress (pet.energy, FORCE_SLEEP, 256, 9);
      loadGlyph35('a', 10, 18);
      loadGlyph35('g', 14, 18);
      loadGlyph35('e', 18, 18);
      // Adjust for each life phase
      if (pet.age < AGE_MOVE) {
        drawProgress (pet.age, 0, AGE_MOVE, 25);
      } else if (pet.age < AGE_HATCH) {
        drawProgress (pet.age, AGE_MOVE, AGE_HATCH, 25);
      } else if (pet.age < AGE_MATURE) {
        drawProgress (pet.age, AGE_HATCH, AGE_MATURE, 25);
      } else {
        drawProgress (pet.age, AGE_MATURE, AGE_DEATH, 25);
      }
      break;
    case DISPLAY_RPG: // Display RPG
      loadGlyph35('R', 10, 2);
      loadGlyph35('P', 14, 2);
      loadGlyph35('G', 18, 2);

      // Coins
      loadGlyph35('5', 2,  8);
      linePixels(3, 7, 3, 13, 1);
      drawNumber(pet.rpg.coins, 13, 8); // Numeric Reading

      // Luck
      loadGlyph35('L', 2,  15);
      loadGlyph35('U', 6,  15);
      loadGlyph35('C', 10, 15);
      loadGlyph35('K', 14, 15);
      drawNumber(pet.rpg.luck, 13, 15); // Numeric Reading

      // Attack
      loadGlyph35('A', 2,  21);
      loadGlyph35('T', 6,  21);
      loadGlyph35('T', 10, 21);
      drawNumber(pet.rpg.attack, 13, 21); // Numeric Reading

      // Defense
      loadGlyph35('D', 2,  27);
      loadGlyph35('E', 6,  27);
      loadGlyph35('F', 10, 27);
      drawNumber(pet.rpg.defense, 13, 27); // Numeric Reading
      break;
  }
}

void drawNumber(int num, int x, int y) {
  char snum[5];
  int c, i;
  itoa(num, snum, 10); // int to base 10 string
  
  // count digits
  for (c = 0; c < 5; c++) {
    if(snum[c] == 0x00)
      break;
  }
  for (i = 0; i < c; i++) {
    loadGlyph35(snum[i], ((5 - c) * 4) + (i * 4) + x, y); // Draw them aligned right
  }
}

void drawProgress (int value, int vmin, int vmax, int y) {
    int p;
    linePixels( 3, y,     28, y,     1); // top
    linePixels( 3, y + 4, 28, y + 4, 1); // bottom
    linePixels( 2, y + 1 , 2, y + 3, 1); // left
    linePixels(29, y + 1, 29, y + 3, 1); // right
    p = map(value, vmin, vmax, 0, 27);
    if (p > 27)
      p = 27;
    rectPixels(3, y + 1, p,  3, 1, 1); // Fill in progress bar
}

void rectPixels(int8_t x, int8_t y, int8_t w, int16_t h, bool value, bool fill) {
  if (fill) { // Draw one horizonal line per pixel height.
    for(uint8_t i = 0; i < h; i++) {
      linePixels(x, y + i, x + w - 1, y + i, value);
    }    
  } else { // Draw outline of square
    linePixels(x, y, x + w - 1, y, value); // top
    linePixels(x, y + h - 1, x + w - 1, y + h - 1, value); // bottom
    linePixels(x, y, x, y + h -1, value); // left
    linePixels(x + w - 1, y, x + w - 1, y + h - 1, value); // right
  }
}

// Adapted from adafruit GFX library, which is based on Bresenham's line algorithm
void linePixels(int8_t x0, int8_t y0, int8_t x1, int16_t y1, bool value) {
  int8_t dx, dy, err, ystep;
  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    // Swap x0 and y0
    err = x0;
    x0 = y0;
    y0 = err;

    // Swap x1 and y1
    err = x1;
    x1 = y1;
    y1 = err;
  }

  if (x0 > x1) {
    // Swap x0 and x1
    err = x0;
    x0 = x1;
    x1 = err;
    
    // Swap y0 and y1
    err = y0;
    y0 = y1;
    y1 = err;
  }
  
  dx = x1 - x0;
  dy = abs(y1 - y0);

  err = dx / 2;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      setPixel(y0, x0, value);
    } else {
      setPixel(x0, y0, value);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
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

int calculateXByte(int l) {
  int c = 0;
  while(l > 7) {
    c++;
    l -= 8;
  }
  return(c);
}

void libpet_explore() {
  int8_t l = 0, t, jm, x, y, bx, by, r[EXPLORE_HIDE + pet.rpg.luck][2];
  uint8_t bpb[32][4], bpx; // Back-up pixel buffer and vars
  bool restore = false; // restore backup pixel buffer
  bool hide = true, fill = true, gend = false;
  long tt;
  int i, j, fitems = 0;

  // Is pet able?
  if (!pet.state.alive || pet.state.sleep || pet.state.eat || pet.stage == 0 || pet.state.explore) {
    return;
  }

  // Does user have enough coin
  if (pet.rpg.coins < EXPLORE_COST) {
    // No Coin Animation
    return;
  }

  pet.state.explore = true;
  pet.rpg.coins -= EXPLORE_COST;
  
  doRandTransition(1, 8, true); // frameskip 8 seems nice
  fillPixels(); // Fill real pixel buffer
  // Set starting co-ordinates
  x = random(0, 32);
  y = random(0, 32);

  drawPixels();

  for (i = 0; i < 256 && !gend; i++) { // 256 steps
    if (hide) {
      hide = false;
      // Hide stuff
      //Serial.println("Hide");
      for (j = 0; j < EXPLORE_HIDE + pet.rpg.luck; j++) { // Increase with luck
        r[j][0] = random(0, 32);
        r[j][1] = random(0, 32);
      }
    }
    setPixel(x, y, 0); // Set Real Pixel Buffer

    // Set backup pixel buffer
    bpx = calculateXByte(x);
    bpb[y][bpx] &= ~(1 << (7 - (x - bpx * 8))); // Set backup

    //Serial.println("Explore");
    // Explore
    do {
      t = random(-8, 9);    
      // Not sure if we need these extra random lines
      switch(t) {
        case -8:
        case -6:
        case -4:
        case -2:
          x--;
          if (x < 0)
            x = 0;
          break;
        case  0:
        case  2:
        case  4:
        case  6:
          y++;
          if (y > 31)
            y = 31;
          break;
        case -7:
        case -5:
        case -3:
        case -1:
          y--;
          if (y < 0)
            y = 0;
          break;
        case 1:
        case 3:
        case 5:
        case 7:
          x++;
          if (x > 31)
            x = 31;
          break;
        default:
          break;
      }
    } while (!getPixel(x, y));

    // Serial.println("Find");

    // Check for find
    jm = EXPLORE_HIDE + pet.rpg.luck; // Luck can change mid lood
    for (j = 0; j < jm; j++) {
      if (x == r[j][0] && y == r[j][1]) { // found something
        // What did we find?
        switch(random(0, 65)) {
          case 15: // Battle
            if(!gotBattle()) {
              gend = true;
            }
            break;
          case 14: // Location
          case  7:
            // Serial.println("Location");
            gotLocation();
            break;
          case 60: // deep level entrance (our highest level)
          case 50:
          case 40:
          case 30:
            // Serial.println("Deep");
            i = 0; // reset steps
            if (explorer_high == l) {
              explorer_high++;
            }
            gotLevel(explorer_high);
            l = explorer_high;
            fill = true;
            hide = true;
            break;
          case 20: // next level entrance
          case 10:
          case  0:
            // Serial.println("Level");
            i = 0; // reset steps
            gotLevel(++l);
            if(explorer_high < l)
              explorer_high = l;
            fill = true;
            hide = true;
            break;
          case 64: // big coins
            // Serial.println("c");
            gotCoins(random(100, (20 * l) + 200)); // Level bonus
            restore = true;
            break;
          case 32: // medium coins
          case 16:
            // Serial.println("c");
            gotCoins(random(20, (10 * l) + 100)); // Level bonus
            restore = true;
            break;
          default: // small coins
            gotCoins(random(1, (10 * l) + 10)); // Level bonus
            restore = true; // restore pixel buffer
        }
      }
    }
    if (fill) { // Fill screen and backup
      fill = false;
      
      if (!tdisp.in_stat) {
        fillPixels();
        setPixel(x, y, 0);
        drawPixels();
      }
      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 4; x++) {
          bpb[y][x] = 0xFF;
        }
      }
    }
    if(restore) {
      restore = false;
      // backup x, y
      bx = x;
      by = y;
      for (y = 0; y < 32; y++) {
        for(x = 0; x < 4; x++) {
          pixbuf[y][x] = bpb[y][x];
        }
      }
      x = bx;
      y = by;
    }

    drawPixels();
    tt = millis();
    while (millis() - tt < 1000 / (T_FPS * 4)) { // burn time watching touch
      processTouch(false);
      delay(10);
    }
  }
  doRandTransition(0, 8, false); // Exit transition
  pet.state.explore = false;
}

void gotCoins(int count) {
  char snum[5];
  int x, i;

  // Serial.print("Coins: ");
  // Serial.println(count);
  pet.rpg.coins += count;
  doShiftTransition(random(0, 2));
  //doRandTransition(0, 64, true); // fast Fadeout with fill
  
  loadGlyph35('C',  6, 2);
  loadGlyph35('o', 10, 2);
  loadGlyph35('i', 14, 2);
  loadGlyph35('n', 18, 2);
  loadGlyph35('s', 22, 2);
      
  loadGlyph35('f',  6, 8);
  loadGlyph35('o', 10, 8);
  loadGlyph35('u', 14, 8);
  loadGlyph35('n', 18, 8);
  loadGlyph35('d', 22, 8);
  drawNumber(count, 6, 18);
  drawPixels();
  
  long tt = millis();
  while (millis() - tt < 5000 / T_FPS) { // burn time watching touch
    processTouch(false);
    delay(10);
  }
  doRandTransition(1, 64, false); // Fast fade-in without fill
}

void gotLevel(int level) {
  char snum[5];
  int x, i;
  //Serial.print("Entrance ");
  //Serial.println(level);
  doRandTransition(0, 64, true); // fast Fadeout with fill
  
  loadGlyph35('e', 0, 2);
  loadGlyph35('n', 4, 2);
  loadGlyph35('t', 8, 2);
  loadGlyph35('r', 12, 2);
  loadGlyph35('a', 16, 2);
  loadGlyph35('n', 20, 2);
  loadGlyph35('c', 24, 2);
  loadGlyph35('e', 28, 2);

  loadGlyph35('F',  6, 8);
  loadGlyph35('o', 10, 8);
  loadGlyph35('u', 14, 8);
  loadGlyph35('n', 18, 8);
  loadGlyph35('d', 22, 8);
  drawNumber(level, 6, 18);
  drawPixels();
  long tt = millis();
  while (millis() - tt < 5000 / T_FPS) { // burn time watching touch
    processTouch(false);
    delay(10);
  }
  getExperience(random(1, 50));
  doRandTransition(1, 64, false); // Fast fade-in without fill
}

void gotLocation() {
  char snum[5];
  int x, i = 0;
  doRandTransition(0, 64, true); // Fast fade-out with fill
  
  x = random(0, 9);
  switch(x) {
    case 0: // Temple (LUCK)
    case 3: //
      loadGlyph35('t',  4, 2);
      loadGlyph35('e',  8, 2);
      loadGlyph35('m', 12, 2);
      loadGlyph35('p', 16, 2);
      loadGlyph35('l', 20, 2);
      loadGlyph35('e', 24, 2);
      i = random(1, 3);
      
      loadGlyph35('l',  0, 18);
      loadGlyph35('u',  4, 18);
      loadGlyph35('c',  8, 18);
      loadGlyph35('k', 12, 18);
      getExperience(random(1, 100));
      pet.rpg.luck += i;
      
      break;
    case 1: // GYM (Attack)
    case 2: // GYM (Defense)
      loadGlyph35('g', 12, 0);
      loadGlyph35('y', 16, 0);
      loadGlyph35('m', 20, 0);
      i = random(0, 2);
      getExperience(random(1, 100));
      if (x == 0) {
        pet.rpg.attack += i;
        loadGlyph35('a', 0, 18);
        loadGlyph35('t', 4, 18);
        loadGlyph35('t', 8, 18);
        i = pet.rpg.attack;
      } else {
        pet.rpg.defense += i;
        loadGlyph35('d', 0, 18);
        loadGlyph35('e', 4, 18);
        loadGlyph35('f', 8, 18);
        i = pet.rpg.defense;
      }
      break;
    default: // Nothing
      loadGlyph35('r',  6, 2);
      loadGlyph35('u', 10, 2);
      loadGlyph35('i', 14, 2);
      loadGlyph35('n', 18, 2);
      loadGlyph35('s', 22, 2);
  }
  loadGlyph35('f',  6, 8);
  loadGlyph35('o', 10, 8);
  loadGlyph35('u', 14, 8);
  loadGlyph35('n', 18, 8);
  loadGlyph35('d', 22, 8);

  if (x < 4) { // If we are not Ruins
    drawNumber(i, 6, 18);
  }
  drawPixels();
  long tt = millis();
  while (millis() - tt < 5000 / T_FPS) { // burn time watching touch
    processTouch(false);
    delay(10);
  }
  doRandTransition(1, 64, false); // Fast fade-in without fill
}

int gotBattle() {
  long tt;
  int hp, hpb, ehp, ehpb, att, def, x, turn = random(0, 2);
  doRandTransition(0, 64, true); // Fast fade-out with fill
  hp = 100; hpb = 100;
  x = random(0, 9);
  switch(x) {
    case 0: // DRAGON (HARD)
      loadGlyph35('d',  4, 2);
      loadGlyph35('r',  8, 2);
      loadGlyph35('a', 12, 2);
      loadGlyph35('g', 16, 2);
      loadGlyph35('o', 20, 2);
      loadGlyph35('n', 24, 2);
      ehp = random(50, 200);
      att = random(1, 10);
      def = random(1, 8);
      break;
    default: // Snake (EASY)
      loadGlyph35('s',  6, 2);
      loadGlyph35('n', 10, 2);
      loadGlyph35('a', 14, 2);
      loadGlyph35('k', 18, 2);
      loadGlyph35('e', 22, 2);
      ehp = random(10, 50);
      att = random(1, 4);
      def = random(1, 3);
  }
  ehpb = ehp;
  loadGlyph35('a', 1, 8);
  loadGlyph35('t', 5, 8);
  loadGlyph35('t', 9, 8);
  loadGlyph35('a', 13, 8);
  loadGlyph35('c', 17, 8);
  loadGlyph35('k', 21, 8);
  loadGlyph35('e', 25, 8);
  loadGlyph35('d', 29, 8);
  drawPixels();

  // delay
  tt = millis();
  while (millis() - tt < 2000 / T_FPS) { // burn time watching touch
    processTouch(false);
    delay(10);
  }

  

  while (ehp > 0 && hp > 0) {
    
    rectPixels(1, 8, 31, 5, 0, 1);
    drawProgress(ehp, 0, ehpb, 8);  // Enemy
    rectPixels(2, 25, 30, 5, 0, 1);
    drawProgress(hp,  0,  hpb, 25); // Us
    drawPixels();
    
    if (turn) { // Player turn
      turn = 0;
      rectPixels(2, 25, 30, 5, 0, 1); // Clear
      drawProgress(hp,  0,  hpb, 23); // Us
      drawPixels();
      rectPixels(2, 23, 30, 5, 0, 1); // Clear
      delay(250);
      drawProgress(hp,  0,  hpb, 25); // Us
    } else {
      turn = 1;
      rectPixels(1, 8, 31, 5, 0, 1); // Clear
      drawProgress(ehp, 0, ehpb, 10);  // Enemy
      drawPixels();
      rectPixels(2, 10, 30, 5, 0, 1);
      delay(250);
      drawProgress(ehp, 0, ehpb, 8);  // Enemy
    }
    drawPixels();
    delay(250);
    rectPixels(0, 16, 32, 5, 0, 1); // Clear
    switch(random(0, 8)) {
      case 0: // miss
      case 4:
        loadGlyph35('m',  8, 16);
        loadGlyph35('i', 12, 16);
        loadGlyph35('s', 16, 16);
        loadGlyph35('s', 20, 16);
        break;
      case 1: // hit
      case 2:
      case 3:
        loadGlyph35('h', 10, 16);
        loadGlyph35('i', 14, 16);
        loadGlyph35('t', 18, 16);
        if (turn) { // user
          ehp -= abs((pet.rpg.attack + 1) - (def / 2));
        } else { // enemy
          hp -= 4*abs(att - (pet.rpg.defense / 2));
        }
        break;
      default: // weak hit
        loadGlyph35('w', 1, 16);
        loadGlyph35('e', 5, 16);
        loadGlyph35('a', 9, 16);
        loadGlyph35('k', 13, 16);
        loadGlyph35('h', 21, 16);
        loadGlyph35('i', 25, 16);
        loadGlyph35('t', 29, 16);
        if (turn) { // user
          ehp -= abs(((pet.rpg.attack + 1) / 2) - (def / 2));
        } else { // enemy
          hp -= abs((att / 2) - (pet.rpg.defense / 2));
        }
        break;
    }
    drawPixels();
    tt = millis();
    while (millis() - tt < 2000 / T_FPS) { // burn time watching touch
      processTouch(false);
      delay(10);
    }
  }

  rectPixels(1, 8, 31, 5, 0, 1);
  drawProgress(ehp, 0, ehpb, 8);  // Enemy
  rectPixels(2, 25, 30, 5, 0, 1);
  drawProgress(hp,  0,  hpb, 25); // Us
  drawPixels();
  delay(250);
  
  doRandTransition(1, 64, false); // Fast fade-in without fill

  // Who won?
  if(hp > 0) { // User
    if(x == 0) {
      gotCoins(random(100, 100 * att));
      getExperience(random(25, 50 * att));
    } else {
      gotCoins(random(1, 25));
      getExperience(random(25, 50));
    }
    return 1;
  } else { // Enemy
    getExperience(random(1, 10));
    return 0 ;
  }
}

void getExperience(int a){
  pet.rpg.experience += a;
  while (pet.rpg.experience >= 500) {
    pet.rpg.experience -= 500;
    if(random(0, 2)){ // attack
      pet.rpg.attack++;
    } else {
      pet.rpg.defense++;
    }
    //Serial.println(pet.rpg.experience);
  }
}

void doShiftTransition(bool lr) {
  uint8_t i;
  long tt;
  for (i = 0; i < 32 && !tdisp.in_stat; i++) {
    if (lr) {
      doOffset(1);
    } else {
      doOffset(-1);
    }
    drawPixels();
    tt = millis();
    while (millis() - tt < (1000 / 32) / T_FPS) { // burn time watching touch
      processTouch(false);
      delay(10);
    }
  }
}

void doRandTransition(bool v, uint8_t fs, bool fill) {
  int x, y, z, r = 0;
  int todo = 32 * 32; // total pixels
  if (fill) {
    if (v) {
      clearPixels();
    } else {
      fillPixels();
    }
  } else {
    todo = 0; // Reset todo to zero
    for (y = 0; y < 32; y++) {
      for (x = 0; x < 32; x++) {
        if(getPixel(x, y) != v) { // If we have a set pixel
          todo++; // Count it
        }
      }
    }
  }
  drawPixels();
  while (todo - 1 > fs && r < 120) { // end check & retry cuttoff
    z = 0;
    while (z < fs && r < 120) { // frame skip & retry cuttoff
      x = random(0, 32);
      y = random(0, 32);
      if (getPixel(x, y) != v) {
        setPixel(x, y, v);
        todo--;
        z++;
        r = 0;
      } else {
        r++;
      }
    }
    processTouch(false);
    drawPixels();
    delay(1);
  }
  // Catch remaining blocks (fast)
  if (v) {
    fillPixels();
  } else {
    clearPixels();
  }
  drawPixels();
}
