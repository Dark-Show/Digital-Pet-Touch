# Digital Pet Touch
Based on Python Emulator by ryesalvador: https://gist.github.com/ryesalvador/e88cb2b4bbe0694d175ef2d7338abd07

* Enhancements
  * Simple RPG + Economy Subsystem
    * Mature pets must pay to eat
  * Exploration Mini-Game
    * 1024 Pixel "Map", 256 "Steps"
    * Screen Transitions
      * Random Static
      * Shift left/right
    * Locatable Level Entrances
      * Reset Steps to Zero
      * Chance of entering at maximum reached depth
    * Locatable Coins
      * Max allowed return increases on level
    * Random Locations
      * Temple (Luck)
      * Gym (Att / Def)
    * Monster Battles
      * Difficulty: Easy
        * Snake
        * Rat
      * Difficulty: Medium
        * Hawk
        * Wolf
      * Difficulty: Hard
        * Witch
        * Wizard
      * Difficulty: Very Hard
        * Dragon
    * Simple Experience System

# Tested Displays
 * [Keyestudio 2.8" LCD Shield](https://www.keyestudio.com/products/keyestudio-28-inch-240320-tft-lcd-shield-for-arduino-uno-r3)
 * [ILI9486 3.5" Touch Shield](https://www.amazon.ca/gp/product/B07NWH47PV/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1)
 * [Adafruit 2.8" TFT Touch Shield](https://www.adafruit.com/product/1651)

# Supported Display Libraries
 * MCUFriend_kbv (ILI9486) [Good] [Link](https://github.com/prenticedavid/MCUFRIEND_kbv)
 * TFTLCD-Chopped (ILI932X) [Good] [Link](https://github.com/Dark-Show/TFTLCD-Chopped)
 * Adafruit-ILI9341 [Working] [Link](https://github.com/adafruit/Adafruit_ILI9341)
 * TFTLCD-Chopped (ILI9341) [Untested] [Link](https://github.com/Dark-Show/TFTLCD-Chopped)
 * TFTLCD-Chopped (HX8347G) [Untested] [Link](https://github.com/Dark-Show/TFTLCD-Chopped)
 * TFTLCD-Chopped (HX8357D) [Untested] [Link](https://github.com/Dark-Show/TFTLCD-Chopped)
 
# Supported Touchscreen Libraries
 * Adafruit TouchScreen [Good] [Link](https://github.com/adafruit/Adafruit_TouchScreen)
 * Adafruit STMPE610 [Beta] [Link](https://github.com/adafruit/Adafruit_STMPE610)
 
# YouTube Demo
[![Digital Pet Demo](https://img.youtube.com/vi/8PmTnB564RI/0.jpg)](http://www.youtube.com/watch?v=8PmTnB564RI)

# Installation
 * Install libraries for your display (Supported libraries listed above)
 * Test display with example code to ensure it functions properly
 * Change selected display at the top of the digitalpet.ino file
 * Upload to Arduino

