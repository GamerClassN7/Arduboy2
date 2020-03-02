# Fork of the Arduboy2 lib to support different Microcontrollers and Displays

Note1: There are three differnt display lips tested:
- 1. https://github.com/ThingPulse/esp8266-oled-ssd1306 (fast I2C Library for 128x64 pixels SSD1306 Displays)
- 2. https://github.com/Bodmer/TFT_eSPI (fast Library for ESP32 and ESP8266 for many 16bit color TFT-Displays)
- 3. https://github.com/adafruit/Adafruit_ILI9341 (Adafruit has probably the most common display librarys)
    you have to choose one of those in Arduboy2.h with a #define

Note2: ESP32 and ESP8266 eeprom class has no update() function, it needs to be created by you. See file: Arduboy2/eepromNote.txt

Note3: For button inputs I am using the PS2X_lib but you can use whatever you want for reading your buttons, see Buttons.ino Example 

Note4: I added created also a ArduboyTones fork for ESP32 and ESP8266 audio support.  
        
![BreadboardESP8266](BreadboardESP8266.JPG)
