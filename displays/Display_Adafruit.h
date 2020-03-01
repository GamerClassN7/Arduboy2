#ifndef DISPLAY_ADAFRUIT_H
#define DISPLAY_ADAFRUIT_H

#include <Arduino.h>
#include "Display_Base.h"
#include "../src/Arduboy2Core.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320

// calculate the offset to center the screen
#define SCREEN_OFFSET_X ((SCREEN_WIDTH - WIDTH) / 2)
#define SCREEN_OFFSET_Y ((SCREEN_HEIGHT - HEIGHT) / 2)

// For the Adafruit shield, these are the default.
#define TFT_DC 4
#define TFT_CS 5

Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC);

class Display_Adafruit : public Display_Base
{
public:
    Display_Adafruit()
    {
        // link the pointer of base class to actual array
        sBuffer = uBuffer;
    };

    void init()
    {
        screen.begin();
        screen.fillScreen(ILI9341_BLACK);
        screen.setRotation(1);
    }

    void drawVerticalLine(int16_t x, int16_t y, int16_t len, uint8_t color)
    {
        screen.drawFastVLine(SCREEN_OFFSET_X + x, SCREEN_OFFSET_Y + y, len, color1to16Bit[color]);
    }

    void display()
    {
        //screen.fillRect(SCREEN_OFFSET_X, SCREEN_OFFSET_Y, WIDTH, HEIGHT, ILI9341_CYAN);

        uint8_t dataCurrent = 0;

        // from left to right
        for (uint16_t xPos = 0; xPos < WIDTH; xPos++)
        {
            uint16_t yStart = 0;
            uint16_t yLength = 1;

            uint8_t colorCurrent;
            uint8_t colorCurrentUp;

            // from top to bottom
            for (uint16_t yPos = 0; yPos < HEIGHT; yPos++)
            {
                // makes no sense for the first line, so there is a continue later
                colorCurrentUp = colorCurrent;

                // put next byte to 8 bit data buffer
                if (yPos % 8 == 0)
                {
                    uint16_t byteNumber = xPos + (yPos / 8) * 128;

                    dataCurrent = uBuffer[byteNumber];
                }

                // get color bits
                colorCurrent = dataCurrent & 0x01;

                // shift to next bit
                dataCurrent = dataCurrent >> 1;

                // skip first line
                if (yPos == 0)
                    continue;

                // draw the last line
                if (yPos == HEIGHT - 1)
                    drawVerticalLine(xPos, HEIGHT - 1, 1, colorCurrent);

                if (colorCurrentUp == colorCurrent && yPos < (HEIGHT - 1))
                {
                    // count pixels with the same color
                    yLength++;
                }
                else
                {
                    // draw the last matching pixels
                    drawVerticalLine(xPos, yStart, yLength, colorCurrentUp);

                    // set the start point of the next vertical pixel line
                    yStart = yPos;

                    // reset Length
                    yLength = 1;
                }
            }
        }
    }

    const uint16_t color1to16Bit[2] = {ILI9341_BLACK, ILI9341_WHITE};

    uint8_t uBuffer[(HEIGHT * WIDTH) / 8];
};

#endif