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

// Timing Settings
#define T_TICK  16 // Loops of game code per second
#define T_FPS   2  // Frames per second

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

// 16 (low chance of level 2 reach
// 32 (low chance of level 6 reach
#define EXPLORE_HIDE    32


// Matrix Screen Adjustments
#define T_SELS  0.12 // Multiplier (12% of screen)
#define T_SELP  2    // Selector padding (Pixels)
#define T_PIXS  6    // Pixel size (Pixels)
#define T_PIXG  1    // Pixel padding (Pixels)
#define T_BUTP  5    // Touch controls padding (Pixels)

// Animation IDs
#define IDLE_EGG        0
#define IDLE_BABY       1
#define IDLE_MATURE     2
#define SLEEP_BABY      3
#define SLEEP_MATURE    4
// Overlay IDs
#define OVERLAY_CLEAN   5
#define OVERLAY_EXLAIM  6
#define OVERLAY_ZZZ     7
#define OVERLAY_DEAD    8
#define OVERLAY_EAT     9
#define OVERLAY_STINK  10
// Display IDs
#define DISPLAY_HUNGER 0
#define DISPLAY_ENERGY 1
#define DISPLAY_WASTE  2
#define DISPLAY_AGE    3
#define DISPLAY_RPG    4

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

struct tama_state {
  bool alive;
  bool eat;
  bool sleep;
  bool stink;
  bool warn;
  bool clean;
};

struct tama_rpg {
   int8_t attack;
   int8_t defense;
   int8_t magic;
   int8_t luck;
   uint8_t experience;
   uint8_t level;
   uint32_t coins;
};

struct tama_pet {
   uint16_t hunger;
   uint16_t energy;
   uint16_t waste;
   int16_t  happiness;
   uint16_t age;
   uint8_t  stage;
   struct   tama_state state;
   struct   tama_rpg rpg;
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

uint8_t disp[32][4];   // Active pixels
uint8_t pixbuf[32][4]; // Pixel buffer
uint8_t explorer_high = 0;

int gfx_frames[16] = {2, 2, 2, 2, 2, 1, 2, 2, 2, 5, 2, 1, 1, 1, 1};

// Bits(right to left)
const uint8_t gfx_overlayZzz[2][32][4] PROGMEM = {{{0x00, 0x00, 0x00, 0x00}, // Frame 1
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                                   {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                                   {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                                   {0x00, 0x00, 0x00, 0x01}, // 0x1000000
                                                   {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x01}, // 0x1000000
                                                   {0x00, 0x00, 0x80, 0x00}, // 0x800000
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x70, 0x00}, // 0x700000
                                                   {0x00, 0x00, 0x20, 0x00}, // 0x200000
                                                   {0x00, 0x00, 0x70, 0x00}, // 0x700000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x08, 0x00}, // 0x80000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00}},
                                   
                                                  {{0x00, 0x00, 0x00, 0x00}, // Frame 2
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                                   {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                                   {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                                   {0x00, 0x00, 0x00, 0x01}, // 0x1000000
                                                   {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x01}, // 0x1000000
                                                   {0x00, 0x00, 0x80, 0x00}, // 0x800000
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x70, 0x00}, // 0x700000
                                                   {0x00, 0x00, 0x20, 0x00}, // 0x200000
                                                   {0x00, 0x00, 0x70, 0x00}, // 0x700000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x08, 0x00}, // 0x80000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00}}};

const uint8_t gfx_overlayEat[5][32][4] PROGMEM = {{{0x00, 0x00, 0x00, 0x00}, // Frame 1
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                                   {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                                   {0x00, 0x00, 0x70, 0x07}, // 0x7700000
                                                   {0x00, 0x00, 0xf0, 0x0f}, // 0xff00000
                                                   {0x00, 0x00, 0xd0, 0x0f}, // 0xfd00000
                                                   {0x00, 0x00, 0xf0, 0x0f}, // 0xff00000
                                                   {0x00, 0x00, 0xf0, 0x07}, // 0x7f00000
                                                   {0x00, 0x00, 0xe0, 0x07}, // 0x7e00000
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00}},
                                  
                                                  {{0x00, 0x00, 0x00, 0x00}, // Frame 2
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                                   {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                                   {0x00, 0x00, 0x70, 0x07}, // 0x7700000
                                                   {0x00, 0x00, 0xe0, 0x0f}, // 0xfe00000
                                                   {0x00, 0x00, 0xc0, 0x0f}, // 0xfc00000
                                                   {0x00, 0x00, 0xe0, 0x0f}, // 0xfe00000
                                                   {0x00, 0x00, 0xf0, 0x07}, // 0x7f00000
                                                   {0x00, 0x00, 0xe0, 0x07}, // 0x7e00000
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00}},
                                     
                                                  {{0x00, 0x00, 0x00, 0x00}, // Frame 3
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                                   {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                                   {0x00, 0x00, 0x40, 0x07}, // 0x7400000
                                                   {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                                   {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                                   {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                                   {0x00, 0x00, 0xc0, 0x07}, // 0x7c00000
                                                   {0x00, 0x00, 0xe0, 0x07}, // 0x7e00000
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00}},
                                    
                                                  {{0x00, 0x00, 0x00, 0x00}, // Frame 4
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                                   {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                                   {0x00, 0x00, 0x00, 0x07}, // 0x7000000
                                                   {0x00, 0x00, 0x00, 0x0f}, // 0xf000000
                                                   {0x00, 0x00, 0x00, 0x0e}, // 0xe000000
                                                   {0x00, 0x00, 0x00, 0x0e}, // 0xe000000
                                                   {0x00, 0x00, 0x00, 0x07}, // 0x7000000
                                                   {0x00, 0x00, 0x80, 0x07}, // 0x7800000
                                                   {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00}},
                                   
                                                  {{0x00, 0x00, 0x00, 0x00}, // Frame 5
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                                   {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                                   {0x00, 0x00, 0x00, 0x01}, // 0x1000000
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00}}};

const uint8_t gfx_overlayStink[2][32][4] PROGMEM = {{{0x00, 0x00, 0x00, 0x00}, // Frame 1
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x10}, // 0x10000000
                                                     {0x08, 0x00, 0x00, 0x08}, // 0x8000008
                                                     {0x04, 0x00, 0x00, 0x10}, // 0x10000004
                                                     {0x28, 0x00, 0x00, 0x0a}, // 0xa000028
                                                     {0x44, 0x00, 0x00, 0x11}, // 0x11000044
                                                     {0x28, 0x00, 0x00, 0x0a}, // 0xa000028
                                                     {0x44, 0x00, 0x00, 0x01}, // 0x1000044
                                                     {0x20, 0x00, 0x00, 0x12}, // 0x12000020
                                                     {0x40, 0x00, 0x00, 0x21}, // 0x21000040
                                                     {0x00, 0x00, 0x00, 0x10}, // 0x10000000
                                                     {0x00, 0x00, 0x00, 0x20}, // 0x20000000
                                                     {0x00, 0x00, 0x00, 0x10}, // 0x10000000
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00}},
                                   
                                                    {{0x00, 0x00, 0x00, 0x00}, // Frame 2
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00}, 
                                                     {0x00, 0x00, 0x00, 0x10}, // 0x10000000
                                                     {0x08, 0x00, 0x00, 0x08}, // 0x8000008
                                                     {0x04, 0x00, 0x00, 0x10}, // 0x10000004
                                                     {0x28, 0x00, 0x00, 0x0a}, // 0xa000028
                                                     {0x44, 0x00, 0x00, 0x11}, // 0x11000044
                                                     {0x28, 0x00, 0x00, 0x0a}, // 0xa000028
                                                     {0x44, 0x00, 0x00, 0x01}, // 0x1000044
                                                     {0x20, 0x00, 0x00, 0x12}, // 0x12000020
                                                     {0x40, 0x00, 0x00, 0x21}, // 0x21000040
                                                     {0x00, 0x00, 0x00, 0x10}, // 0x10000000
                                                     {0x00, 0x00, 0x00, 0x20}, // 0x20000000
                                                     {0x00, 0x00, 0x00, 0x10}, // 0x10000000
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00},
                                                     {0x00, 0x00, 0x00, 0x00}}};

const uint8_t gfx_selectorIcon[32][4] PROGMEM = {{0x0f, 0x00, 0x00, 0x78}, // 0x7800000f
                                                 {0x03, 0x00, 0x00, 0x60}, // 0x60000003
                                                 {0x01, 0x00, 0x00, 0x40}, // 0x40000001
                                                 {0x01, 0x00, 0x00, 0x40}, // 0x40000001
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00}, 
                                                 {0x00, 0x00, 0x00, 0x00}, 
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x01, 0x00, 0x00, 0x40},  // 0x40000001
                                                 {0x01, 0x00, 0x00, 0x40},  // 0x40000001
                                                 {0x03, 0x00, 0x00, 0x60},  // 0x60000003
                                                 {0x0f, 0x00, 0x00, 0x78}}; // 0x7800000f

const uint8_t gfx_feedIcon[32][4] PROGMEM = {{0x00, 0x00, 0x00, 0x00},
                                             {0x00, 0x00, 0x00, 0x00},
                                             {0x00, 0x00, 0x00, 0x00},
                                             {0x00, 0x00, 0x00, 0x00},
                                             {0x00, 0x00, 0x00, 0x00},
                                             {0xa0, 0x05, 0x78, 0x00}, // 0x7805a0
                                             {0xa0, 0x05, 0x7c, 0x00}, // 0x7c05a0
                                             {0xa0, 0x05, 0x7c, 0x00}, // 0x7c05a0
                                             {0xa0, 0x05, 0x7c, 0x00}, // 0x7c05a0
                                             {0xa0, 0x05, 0x7c, 0x00}, // 0x7c05a0
                                             {0xa0, 0x05, 0x7c, 0x00}, // 0x7c05a0
                                             {0xa0, 0x05, 0x7c, 0x00}, // 0x7c05a0
                                             {0xe0, 0x07, 0x7c, 0x00}, // 0x7c07e0
                                             {0xe0, 0x07, 0x7c, 0x00}, // 0x7c07e0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0xc0, 0x03, 0x78, 0x00}, // 0x7803c0
                                             {0x80, 0x01, 0x30, 0x00}, // 0x300180
                                             {0x00, 0x00, 0x00, 0x00},
                                             {0x00, 0x00, 0x00, 0x00},
                                             {0x00, 0x00, 0x00, 0x00}};

const uint8_t gfx_flushIcon[32][4] PROGMEM = {{0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                              {0x00, 0x00, 0x00, 0x05}, // 0x5000000
                                              {0x00, 0x00, 0x00, 0x05}, // 0x5000000
                                              {0x00, 0x00, 0x80, 0x04}, // 0x4800000
                                              {0x00, 0x00, 0x80, 0x04}, // 0x4800000
                                              {0x00, 0x00, 0x40, 0x04}, // 0x4400000
                                              {0x00, 0x00, 0x40, 0x04}, // 0x4400000
                                              {0x00, 0x00, 0x40, 0x04}, // 0x4400000
                                              {0x00, 0x00, 0x20, 0x02}, // 0x2200000
                                              {0x00, 0x00, 0x20, 0x02}, // 0x2200000
                                              {0x00, 0x00, 0x20, 0x01}, // 0x1200000
                                              {0x00, 0xff, 0xff, 0x00}, // 0xffff00
                                              {0x80, 0x02, 0x20, 0x01}, // 0x1200280
                                              {0x00, 0xfd, 0xf1, 0x01}, // 0x11ffd00
                                              {0x80, 0x00, 0x00, 0x01}, // 0x1000080
                                              {0x80, 0x00, 0x00, 0x01}, // 0x1000080
                                              {0x80, 0x00, 0x00, 0x01}, // 0x1000080
                                              {0x80, 0x00, 0x00, 0x01}, // 0x1000080
                                              {0x80, 0x00, 0x00, 0x01}, // 0x1000080
                                              {0x80, 0x00, 0x00, 0x01}, // 0x1000080
                                              {0x40, 0x00, 0x00, 0x01}, // 0x1000040
                                              {0x80, 0xff, 0xff, 0x00}, // 0xffff80
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00},
                                              {0x00, 0x00, 0x00, 0x00}};

const uint8_t gfx_healthIcon[32][4] PROGMEM = {{0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00}, // 0x3ffffc0
                                               {0x00, 0x00, 0x00, 0x00}, // 0xc000030
                                               {0x88, 0x24, 0x91, 0x10}, // 0x10912488
                                               {0x88, 0x24, 0x91, 0x10}, // 0x10912488
                                               {0x08, 0x29, 0x49, 0x10}, // 0x10492908
                                               {0x10, 0x00, 0x00, 0x08}, // 0x8000010
                                               {0x10, 0x00, 0x00, 0x08}, // 0x8000010
                                               {0x10, 0x04, 0x00, 0x08}, // 0x8000410
                                               {0x20, 0x08, 0x00, 0x04}, // 0x4000820
                                               {0x20, 0x10, 0x00, 0x04}, // 0x4001020
                                               {0x20, 0x20, 0x00, 0x04}, // 0x4002020
                                               {0x40, 0xc0, 0x01, 0x02}, // 0x201c040
                                               {0x40, 0xc0, 0x01, 0x02}, // 0x201c040
                                               {0x80, 0xff, 0xff, 0x01}, // 0x1ffff80
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00},
                                               {0x00, 0x00, 0x00, 0x00}};

const uint8_t gfx_zzzIcon[32][4] PROGMEM = {{0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                            {0x00, 0x00, 0x00, 0x04}, // 0x4000000
                                            {0x00, 0x00, 0x00, 0x02}, // 0x2000000
                                            {0x00, 0x00, 0x00, 0x01}, // 0x1000000
                                            {0x00, 0x00, 0x80, 0x0f}, // 0xf800000
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                            {0x00, 0x00, 0x00, 0x01}, // 0x1000000
                                            {0x00, 0x00, 0x80, 0x00}, // 0x800000
                                            {0x00, 0x00, 0xc0, 0x03}, // 0x3c00000
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x70, 0x00}, // 0x700000
                                            {0x00, 0x00, 0x20, 0x00}, // 0x200000
                                            {0x00, 0x00, 0x70, 0x00}, // 0x700000
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x08, 0x00}, // 0x80000
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00},
                                            {0x00, 0x00, 0x00, 0x00}};

const uint8_t gfx_ExploreIcon[32][4] PROGMEM = {{ 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 }};

const uint8_t gfx_idleEgg[2][12][4] PROGMEM  = {{{0x00, 0xe0, 0x07, 0x00}, // 0x7e000
                                                 {0x00, 0x70, 0x08, 0x00}, // 0x87000
                                                 {0x00, 0x38, 0x10, 0x00}, // 0x103800
                                                 {0x00, 0x0c, 0x30, 0x00}, // 0x300c00
                                                 {0x00, 0x04, 0x70, 0x00}, // 0x700400
                                                 {0x00, 0x82, 0x41, 0x00}, // 0x418200
                                                 {0x00, 0x82, 0x41, 0x00}, // 0x418200
                                                 {0x00, 0x02, 0x40, 0x00}, // 0x400200
                                                 {0x00, 0x06, 0x70, 0x00}, // 0x700600
                                                 {0x00, 0x0c, 0x3c, 0x00}, // 0x3c0c00
                                                 {0x00, 0x08, 0x1e, 0x00}, // 0x1e0800
                                                 {0x00, 0xfc, 0x3f, 0x00}}, // 0x3ffc00
                                 
                                                {{0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0xe0, 0x07, 0x00}, // 0x7e000
                                                 {0x00, 0x70, 0x08, 0x00}, // 0x87000
                                                 {0x00, 0x38, 0x10, 0x00}, // 0x103800
                                                 {0x00, 0x0c, 0x30, 0x00}, // 0x300c00
                                                 {0x00, 0x04, 0x70, 0x00}, // 0x700400
                                                 {0x00, 0x02, 0x40, 0x00}, // 0x400200
                                                 {0x00, 0x82, 0x41, 0x00}, // 0x418200
                                                 {0x00, 0x82, 0x41, 0x00}, // 0x418200
                                                 {0x00, 0x06, 0x70, 0x00}, // 0x700600
                                                 {0x00, 0x0c, 0x3c, 0x00}, // 0x3c0c00
                                                 {0x00, 0xff, 0xff, 0x00}}}; // 0xffff00

const uint8_t gfx_idleBaby[2][7][4] PROGMEM = {{{0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x00, 0x00, 0x00},
                                                 {0x00, 0x80, 0x07, 0x00}, // 0x78000
                                                 {0x00, 0x40, 0x0b, 0x00}, // 0xb4000
                                                 {0x00, 0xe0, 0x1f, 0x00}},// 0x1fe000
                            
                                                {{0x00, 0x80, 0x07, 0x00}, // 0x78000
                                                 {0x00, 0xc0, 0x0c, 0x00}, // 0xcc000
                                                 {0x00, 0x40, 0x08, 0x00}, // 0x84000
                                                 {0x00, 0x40, 0x0b, 0x00}, // 0xb4000
                                                 {0x00, 0x40, 0x08, 0x00}, // 0x84000
                                                 {0x00, 0x80, 0x07, 0x00}, // 0x78000
                                                 {0x00, 0x00, 0x00, 0x00}}};

const uint8_t gfx_idleMature[2][10][4] PROGMEM = {{{0x00, 0xfc, 0x00, 0x00}, // 0xfc00
                                                   {0x00, 0x02, 0x01, 0x00}, // 0x10200
                                                   {0x00, 0x49, 0x02, 0x00}, // 0x24900
                                                   {0x00, 0x01, 0x02, 0x00}, // 0x20100
                                                   {0x00, 0x31, 0x02, 0x00}, // 0x23100
                                                   {0x00, 0x01, 0x02, 0x00}, // 0x20100
                                                   {0x00, 0x01, 0x02, 0x00}, // 0x20100
                                                   {0x00, 0x02, 0x01, 0x00}, // 0x10200
                                                   {0x00, 0xfc, 0x00, 0x00}, // 0xfc00
                                                   {0x00, 0x00, 0x00, 0x00}},
                            
                                                   {{0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0x00, 0x00, 0x00},
                                                   {0x00, 0xfc, 0x00, 0x00}, // 0xfc00
                                                   {0x00, 0x02, 0x01, 0x00}, // 0x10200
                                                   {0x00, 0x85, 0x02, 0x00}, // 0x28500
                                                   {0x00, 0x31, 0x02, 0x00}, // 0x23100
                                                   {0x00, 0x31, 0x02, 0x00}, // 0x23100
                                                   {0x00, 0x01, 0x02, 0x00}, // 0x20100
                                                   {0x00, 0x02, 0x01, 0x00}, // 0x10200
                                                   {0x00, 0xfc, 0x00, 0x00}}}; // 0xfc00
                                                   

const uint8_t gfx_sleepBaby[2][3][4] PROGMEM = {{{0x00, 0x80, 0x07, 0x00}, // 0x78000
                                                  {0x00, 0xc0, 0x0f, 0x00}, // 0xfc000
                                                  {0x00, 0xe0, 0x1f, 0x00}},// 0x1fe000
                            
                                                 {{0x00, 0x00, 0x00, 0x00},
                                                  {0x00, 0xe0, 0x1f, 0x00},   // 0x1fe000
                                                  {0x00, 0xf0, 0x3f, 0x00}}}; // 0x3ff000

const uint8_t gfx_sleepMature[2][32][4] PROGMEM = {{{0x00, 0x00, 0x00, 0x00},
                                                    {0x00, 0xfc, 0x03, 0x00}, // 0x3fc00
                                                    {0x00, 0x02, 0x04, 0x00}, // 0x40200
                                                    {0x00, 0x01, 0x08, 0x00}},// 0x80100
                            
                                                   {{0x00, 0xf8, 0x01, 0x00},   // 0x1f800
                                                    {0x00, 0x04, 0x02, 0x00},   // 0x20400
                                                    {0x00, 0x02, 0x04, 0x00},   // 0x40200
                                                    {0x00, 0x02, 0x04, 0x00}}}; // 0x40200
const uint8_t ifo_Hunger[2] = {32, 5};
const uint8_t gfx_Hunger[5][4] PROGMEM = {{0xa4, 0x94, 0xcc, 0x3b}, // 0x3bcc94a4
                                          {0xa4, 0xb4, 0x52, 0x48}, // 0x4852b4a4
                                          {0xbc, 0xd4, 0xc2, 0x39}, // 0x39c2d4bc
                                          {0xa4, 0x94, 0x5a, 0x48}, // 0x485a94a4
                                          {0x24, 0x93, 0xdc, 0x4b}}; // 0x4bdc9324
const uint8_t ifo_Energy[2] = {32, 5};
const uint8_t gfx_Energy[5][4] PROGMEM = {{0xbc, 0xf4, 0x8e, 0x49}, // 0x498ef4bc
                                          {0x84, 0x15, 0x52, 0x4a}, // 0x4a521584
                                          {0x8c, 0x36, 0x4e, 0x70}, // 0x704e368c
                                          {0x84, 0x14, 0x52, 0x43}, // 0x43521484
                                          {0xbc, 0xf4, 0x92, 0x3b}}; // 0x3b92f4bc
const uint8_t ifo_Age[2] = {32, 5};
const uint8_t gfx_Age[5][4] PROGMEM = {{0x38, 0xce, 0x07, 0x00}, // 0x7ce38
                                        {0x44, 0x51, 0x00, 0x00}, // 0x5144
                                        {0x7c, 0xc1, 0x01, 0x00}, // 0x1c17c
                                        {0x44, 0x59, 0x00, 0x00}, // 0x5944
                                        {0x44, 0xde, 0x07, 0x00}}; // 0x7de44

const uint8_t ifo_Waste[2] = {32, 5};
const uint8_t gfx_Waste[5][4] PROGMEM = {{0x44, 0x8e, 0xf3, 0x7d}, // 0x7df38e44
                                         {0x44, 0x51, 0x40, 0x04}, // 0x4405144
                                         {0x44, 0x9f, 0x43, 0x1c}, // 0x1c439f44
                                         {0x54, 0x11, 0x44, 0x04}, // 0x4441154
                                         {0x28, 0x91, 0x43, 0x7c}}; // 0x7c439128
// Bits(left to right)
// Upper Case English Mono Font 100bytes
const uint8_t font_UE3X5[25][4] PROGMEM = {{0b11111011, 0b11101111, 0b11111101, 0b11111100}, // ABCDEFGHIJ (3x5)
                                           {0b10110110, 0b01011001, 0b00100101, 0b01000100}, // ABCDEFGHIJ (3x5)
                                           {0b11111110, 0b01011111, 0b10110111, 0b01000100}, // ABCDEFGHIJ (3x5)
                                           {0b10110110, 0b01011001, 0b00101101, 0b01010100}, // ABCDEFGHIJ (3x5)
                                           {0b10111111, 0b11111111, 0b00111101, 0b11111100}, // ABCDEFGHIJ (3x5)
                                                
                                           {0b10110010, 0b11010101, 0b11010110, 0b01111100}, // KLMNOPQRST (3x5)
                                           {0b10110011, 0b11111011, 0b01101101, 0b10001000}, // KLMNOPQRST (3x5)
                                           {0b11010010, 0b11111011, 0b11101110, 0b11101000}, // KLMNOPQRST (3x5)
                                           {0b10110010, 0b11111011, 0b00111101, 0b00101000}, // KLMNOPQRST (3x5)
                                           {0b10111110, 0b11010101, 0b00011101, 0b11001000}, // KLMNOPQRST (3x5)
                                                
                                           {0b10110110, 0b11011011, 0b11111110, 0b11111100}, // UVWXYZ0123 (3x5)
                                           {0b10110110, 0b11011010, 0b01101010, 0b00100100}, // UVWXYZ0123 (3x5)
                                           {0b10110110, 0b10101110, 0b10101010, 0b11101100}, // UVWXYZ0123 (3x5)
                                           {0b10101111, 0b11010101, 0b00101010, 0b10000100}, // UVWXYZ0123 (3x5)
                                           {0b01100110, 0b11010101, 0b11111111, 0b11111100}, // UVWXYZ0123 (3x5)
                                           
                                           {0b10111111, 0b11111111, 0b11000000, 0b11001000}, // 456789.,?! (3x5)
                                           {0b10110010, 0b00011011, 0b01000000, 0b00101000}, // 456789.,?! (3x5)
                                           {0b11111111, 0b10111111, 0b11000000, 0b01001000}, // 456789.,?! (3x5)
                                           {0b00100110, 0b10011010, 0b01010010, 0b00000000}, // 456789.,?! (3x5)
                                           {0b00111111, 0b10011110, 0b01000010, 0b01001000}, // 456789.,?! (3x5)
                                                
                                           {0b11000000, 0b00001010, 0b10101010, 0b00000000}, // &_+-#&%^:; (3x5)
                                           {0b00100001, 0b00001111, 0b11001101, 0b01001000}, // &_+-#&%^:; (3x5)
                                           {0b11100011, 0b11111011, 0b00010000, 0b00000000}, // &_+-#&%^:; (3x5)
                                           {0b10100001, 0b00001111, 0b11100000, 0b01001000}, // &_+-#&%^:; (3x5)
                                           {0b11011100, 0b00001010, 0b10101000, 0b00001000}};// &_+-#&%^:; (3x5)
                                                
// We dont need the full framebuffer in memory, start reducing graphic footprint.
// We also dont need to store words, make a few fonts and print functions.
const uint8_t ifo_Back[2] = {32, 5};
const uint8_t gfx_Back[5][4] PROGMEM = {{0b00001000, 0b11100011, 0b00011001, 0b00100000}, // 0x498c710
                                        {0b00011000, 0b10010100, 0b10100101, 0b01000000}, // 0x2a52918
                                        {0b00111110, 0b11100111, 0b10100001, 0b10000000}, // 0x185e77c
                                        {0b00011000, 0b10010100, 0b10100101, 0b01000000}, // 0x2a52918
                                        {0b00001000, 0b11100100, 0b10011001, 0b00100000}}; // 0x4992710
