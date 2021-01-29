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
//#define T_TICK  16      // Debug
#define T_TICK  1       // Fast Demo
//#define T_TICK  0.025   // Release
#define T_FPS   2   // Frames per second

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

#define EXPLORE_HIDE    16 // (+luck) Explore fun factor // Medium Return
//#define EXPLORE_HIDE     8 // (+luck) Explore fun factor // Low Return
#define EXPLORE_COST    50 // Explore cost
#define EAT_ADU_COST     5 // Adult eating cost
#define COIN_DEFAULT   100 // Default coins for new pets

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
#define DISPLAY_STAT1 0
#define DISPLAY_STAT2 1
#define DISPLAY_RPG   2


// Matrix Screen Adjustments
#define T_SELS  0.12 // Multiplier (12% of screen)
#define T_SELP  2    // Selector padding (Pixels)
#define T_PIXS  6    // Pixel size (Pixels)
#define T_PIXG  1    // Pixel padding (Pixels)
#define T_BUTP  5    // Touch controls padding (Pixels)

#include "digitalpet_drivers.h"; // After graphics are defined

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
  bool explore;
};

struct tama_rpg {
   int8_t attack;
   int8_t defense;
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

// Bits(left to right)
const uint8_t gfx_ExploreIcon[32][4] PROGMEM = {{ 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
                                                { 0b00000001, 0b11100000, 0b00000000, 0b00000000 },
                                                { 0b00000011, 0b00011000, 0b00000000, 0b00000000 },
                                                { 0b00000011, 0b00010000, 0b00000000, 0b00000000 },
                                                { 0b00000011, 0b11100001, 0b11100000, 0b00000000 },
                                                { 0b00000011, 0b00110011, 0b00110000, 0b00000000 },
                                                { 0b00000011, 0b00011011, 0b00110000, 0b00000000 },
                                                { 0b00000000, 0b00000011, 0b11100000, 0b00000000 },
                                                { 0b00000000, 0b00000011, 0b00000111, 0b11000000 },
                                                { 0b00000000, 0b00000011, 0b00001100, 0b01000000 },
                                                { 0b00000000, 0b00000000, 0b00001100, 0b00000000 },
                                                { 0b00000000, 0b00000000, 0b00001100, 0b11100000 },
                                                { 0b00000000, 0b00000000, 0b00001100, 0b01000000 },
                                                { 0b00000000, 0b00000000, 0b00001111, 0b11000000 },
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
