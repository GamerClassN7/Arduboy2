#ifndef DISPLAY_SSD1306_H
#define DISPLAY_SSD1306_H

#include <Arduino.h>
#include "Display_Base.h"
#include "../src/Arduboy2Core.h"

#include <brzo_i2c.h>
#include "SSD1306Brzo.h"

// create display display
#ifdef ESP8266
#define OLED_I2C_SDA D2
#define OLED_I2C_SCL D1
#else 
// as far a I now the brzo_i2c lib only works with the esp8266!
#define OLED_I2C_SDA 2
#define OLED_I2C_SCL 1
#endif 
#define OLED_I2C_ADRESS    0x3C

#if defined(HARTMANNS_PS2Controller)
extern SSD1306Brzo screen;
#else
SSD1306Brzo screen(OLED_I2C_ADRESS, OLED_I2C_SDA, OLED_I2C_SCL);
#endif

#define I2C_SDA D2
#define I2C_SCL D1

class Display_SSD1306 : public Display_Base
{
public:
    Display_SSD1306()
    {
    };

    void init()
    {  
        Serial.println("init()");

        screen.init();	

        // link the buffer to this static sBuffer thingi
        sBuffer = screen.buffer;

        Serial.println("init() done");
    }

    void clear() 
    {     
        screen.clear();
    }

    void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color)
    {
        // this is called in fillRect, so it needs to be the fast version
        screen.setColor(OLEDDISPLAY_COLOR(color));
        screen.drawVerticalLine(x, y, h);
    }

    void fillScreen(uint8_t color)
    {
        if (color == BLACK)
        {
            // ich denke, dass ist quatsch.......
            screen.clear();
        }
        else
        {
            memset(sBuffer, 0xff, 1024);
        }
    }

    void display()
    {
        screen.display();
    }
};

#endif