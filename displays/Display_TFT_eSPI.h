#ifndef DISPLAY_TFT_eSPI_H
#define DISPLAY_TFT_eSPI_H

#include <Arduino.h>
#include "Display_Base.h"
#include "../src/Arduboy2.h"
#include "../src/Arduboy2Core.h"
#include "Background320x240.h"

#include "SPI.h"
#include "TFT_eSPI.h"

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320
#define SCREEN_OFFSET_X (((SCREEN_WIDTH - WIDTH * 2) / 2) - 5)
#define SCREEN_OFFSET_Y (((SCREEN_HEIGHT - HEIGHT * 2) / 2) - 30)


#if defined(HARTMANNS_PS2Controller)
#include <PS2X_lib.h>
extern PS2X ps2x;

extern TFT_eSPI screen;
#else
TFT_eSPI screen = TFT_eSPI();
#endif

class Display_TFT_eSPI : public Display_Base
{
public:
    Display_TFT_eSPI()
    {
	    forceCompleteDraw = true;
	    zoomActive = true;
	    nextDisplayModeChange = 0;

        // link the pointer of base class to actual array
        sBuffer = uBuffer;

        changeFilterSet(0);
    }

    void init()
    {
        screen.init();
        screen.setRotation(1);

        drawBackground();
    }

    void newFrame()
    {
#if defined(HARTMANNS_PS2Controller)
        if (nextDisplayModeChange < millis())
        {
            // this happens here because I am to lazy to add it to every game i compile...
            if (ps2x.Button(PSB_TRIANGLE))
            {
                toggleFilterSet();

                nextDisplayModeChange = millis() + 200;
            }

            if (ps2x.Button(PSB_SQUARE))
            {
                toggleZoom();

                nextDisplayModeChange = millis() + 200;
            }
        }
#endif				
    }

    void display()
    {
        // this function would just push the whole screen to the display
        //screen.pushImage(94, 35, WIDTH, HEIGHT, oBuffer, false);

        // use one byte for every display pixel, so 256x128 Bytes = 32768 + 8k
        if (filterSet != 0 && zoomActive)
            manipulateDisplayBuffer();

        uint16_t pixelsVertical, pixelsHorizontal;
        if (filterSet != 0 && zoomActive)
        {
            pixelsVertical = HEIGHT * 2;
            pixelsHorizontal = WIDTH * 2;
        }
        else
        {
            pixelsVertical = HEIGHT;
            pixelsHorizontal = WIDTH;
        }

        // outside the for loop because they stay for 8 vertical bits (used in 128x64 mode only)
        uint8_t dataBefore = 0;
        uint8_t dataCurrent = 0;

        // from left to right
        for (uint16_t xPos = 0; xPos < pixelsHorizontal; xPos++)
        {

            uint16_t yStart = 0;
            uint16_t yLength = 1;

            uint8_t colorBefore;
            uint8_t colorBeforeUp;
            uint8_t colorCurrent;
            uint8_t colorCurrentUp;

            // from top to bottom
            for (uint16_t yPos = 0; yPos < pixelsVertical; yPos++)
            {
                // makes no sense for the first line, so there is a continue later
                colorCurrentUp = colorCurrent;
                colorBeforeUp = colorBefore;

                if (filterSet == 0)
                {
                    // put next byte to 8 bit data buffer
                    if (yPos % 8 == 0)
                    {
                        uint16_t byteNumber = xPos + (yPos / 8) * 128;

                        dataCurrent = uBuffer[byteNumber];
                        dataBefore = cBuffer[byteNumber];
                    }

                    // get color bits
                    colorCurrent = dataCurrent & 0x01;
                    colorBefore = dataBefore & 0x01;

                    // shift to next bit
                    dataCurrent = dataCurrent >> 1;
                    dataBefore = dataBefore >> 1;
                }
                else
                {
                    // get data from 8bit buffer
                    colorCurrent = uBuffer[xPos + yPos * pixelsHorizontal];
                    colorBefore = cBuffer[xPos + yPos * pixelsHorizontal];
                }

                // skip first line
                if (yPos == 0)
                    continue;

                // draw the last line
                if (yPos == pixelsVertical - 1 && colorCurrent != colorBefore)
                    displayLineOnILI9341(xPos, pixelsVertical - 1, 1, colorCurrent);

                // check if the pixel has changed since last time
                if (colorCurrentUp == colorBeforeUp && !forceCompleteDraw)
                {
                    //if (false) { // false will force a complete draw on every frame

                    // draw the last matching pixels
                    if (yLength > 1)
                    {

                        // draw the unfinished pixels!!!!!!!!!!!!!!
                        displayLineOnILI9341(xPos, yStart, yLength, colorCurrentUp);

                        // reset Length
                        yLength = 1;
                    }

                    // set the start point of the next vertical pixel line
                    yStart = yPos;
                }
                else
                {
                    // pixel changed since last draw
                    if (colorCurrentUp == colorCurrent && yPos < (pixelsVertical - 1))
                    {
                        // count pixels with the same color
                        yLength++;
                    }
                    else
                    {

                        // draw the last matching pixels
                        displayLineOnILI9341(xPos, yStart, yLength, colorCurrentUp);

                        // set the start point of the next vertical pixel line
                        yStart = yPos;

                        // reset Length
                        yLength = 1;
                    }
                }
            }
        }

        // disable full draw for the next time if it was active
        if (forceCompleteDraw)
        {
            forceCompleteDraw = false;
        }

        uint16_t bufferSizeUsed;
        if (filterSet == 0)
        {
            bufferSizeUsed = (HEIGHT * WIDTH) / 8;
        }
        else
        {
            bufferSizeUsed = sizeof(uBuffer);
        }

        // copys the complete buffer to the change-buffer
        memcpy(cBuffer, uBuffer, bufferSizeUsed);
    }

    void drawBackground()
    {
        if (zoomActive)
        {
            screen.pushImage(0, 0, 320, 240, backgroundZoomActive);
        }
        else
        {
            screen.fillRect(0, 0, 320, 240, BLACK);
            screen.pushImage(52, 0, 217, 240, backgroundZoomInactive);
        }
    }

    void toggleZoom()
    {

        zoomActive = !zoomActive;

        // reprint the arduboy console
        drawBackground();

        // set to set 0 because there can not be filters with no zoom
        changeFilterSet(0);
    }

    void toggleFilterSet()
    {

        // there are no filters without zoom
        if (zoomActive == false)
            return;

        if (filterSet == 5)
        {
            filterSet = 0;
        }
        else
        {
            filterSet++;
        }

        changeFilterSet(filterSet);
    }

    void changeFilterSet(uint8_t set)
    {

        uint16_t colorWhite = 0;
        uint16_t colorLight = 0;
        uint16_t colorDark = 0;
        uint16_t colorBlack = 0;

        // set class variable
        filterSet = set;

        if (set == 0)
        {
            colorWhite = TFT_WHITE;
            colorLight = TFT_WHITE;
            colorDark = TFT_BLACK;
            colorBlack = TFT_BLACK;
        }
        else if (set == 1)
        {
            colorWhite = TFT_WHITE;
            colorLight = TFT_LIGHTGREY;
            colorDark = TFT_DARKGREY;
            colorBlack = TFT_BLACK;
        }
        else if (set == 2)
        {
            colorWhite = TFT_WHITE;
            colorLight = screen.color565(235, 221, 156); // hellgelb
            colorDark = screen.color565(185, 122, 87);   // braun
            colorBlack = screen.color565(112, 56, 56);   // dunkelbraun
        }
        else if (set == 3)
        {
            colorWhite = TFT_WHITE;
            colorLight = screen.color565(255, 160, 160);
            colorDark = screen.color565(160, 0, 0);
            colorBlack = TFT_BLACK;
        }
        else if (set == 4)
        {
            colorWhite = TFT_WHITE;
            colorLight = screen.color565(200, 191, 231); // lavendel
            colorDark = screen.color565(0, 90, 180);
            colorBlack = TFT_BLACK;
        }
        else if (set == 5)
        {
            colorWhite = TFT_WHITE;
            colorLight = TFT_YELLOW;
            colorDark = TFT_MAGENTA;
            colorBlack = TFT_BLACK;
        }

        set16BitFilterColors(colorWhite, colorLight, colorDark, colorBlack);

        drawFilterSet();

        forceCompleteDraw = true;
    }

    void drawFilterSet()
    {
        // there are no filters without zoom
        if (zoomActive == false)
            return;

        for (uint8_t i = 0; i < 4; i++)
        {
            // draw little rects with the current color mix
            screen.fillRect(118 + i * 20, 204, 20, 4, filterColorsArray[i]);
        }
    }

    void set16BitFilterColors(uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4)
    {

        filterColorsArray[0] = c1;
        filterColorsArray[1] = c2;
        filterColorsArray[2] = c3;
        filterColorsArray[3] = c4;
    }

    void displayLineOnILI9341(uint16_t xPos, uint16_t yStart, uint16_t yLength, uint8_t color8Bit)
    {

        if (zoomActive == false)
        {
            // draw screen line 1 pixel width
            screen.drawFastVLine(94 + xPos, 35 + yStart, yLength, color1to16Bit[color8Bit]);
        }
        else
        {
            if (filterSet == 0)
            {
                // draw fast in black and white 2x2 rects
                screen.fillRect(SCREEN_OFFSET_X + xPos * 2, SCREEN_OFFSET_Y + yStart * 2, 2, yLength * 2, color1to16Bit[color8Bit]);
            }
            else
            {
                // this could be also draw vertical line in 4 colors
                screen.drawFastVLine(SCREEN_OFFSET_X + xPos, SCREEN_OFFSET_Y + yStart, yLength, filterColorsArray[color8Bit]);
            }
        }
    }

    void manipulateDisplayBuffer()
    {
        // create a 128x64 byte buffer for fast access to every pixel without bitshifting
        uint8_t currentDataByte = 0;
        for (uint16_t xPos = 0; xPos < WIDTH; xPos++)
        {
            for (uint16_t yPos = 0; yPos < HEIGHT; yPos++)
            {
                // get 1 bit color and save 8 bit color byte

                // put next byte to 8 bit data buffer
                if (yPos % 8 == 0)
                {
                    uint16_t byteNumber = xPos + (yPos / 8) * 128;

                    currentDataByte = uBuffer[byteNumber];
                }

                oBuffer[xPos + yPos * WIDTH] = currentDataByte & 0x01;

                // shift to next bit
                currentDataByte = currentDataByte >> 1;
            }
        }

        enum
        {
            COLOR_WHITE,
            COLOR_LIGHT,
            COLOR_DARK,
            COLOR_BLACK
        };

        // fill 256x128 byte with manipulated values
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                int loc_orginal = x + y * WIDTH;

                uint8_t color_original, color_opposite, color_changed;
                if (oBuffer[loc_orginal] == WHITE)
                {
                    color_original = COLOR_WHITE;
                    color_opposite = BLACK;
                    color_changed = COLOR_LIGHT;
                }
                else
                {
                    color_original = COLOR_BLACK;
                    color_opposite = WHITE;
                    color_changed = COLOR_DARK;
                }

                int loc_filter = x * 2 + y * 2 * WIDTH * 2;

                // oben links, also neues pixel unten rechts wird verändert
                if (
                    ((loc_orginal + 1) % WIDTH != 0) &&
                    (loc_orginal < (HEIGHT - 1) * WIDTH) &&
                    //oBuffer[loc_orginal] == color_original &&
                    oBuffer[loc_orginal + 1] == color_opposite &&
                    oBuffer[loc_orginal + WIDTH] == color_opposite

                )
                {
                    uBuffer[loc_filter + WIDTH * 2 + 1] = color_changed;
                }
                else
                {
                    uBuffer[loc_filter + WIDTH * 2 + 1] = color_original;
                }

                // oben rechts
                if (
                    (loc_orginal % WIDTH != 0) &&
                    (loc_orginal < (HEIGHT - 1) * WIDTH) &&
                    //oBuffer[loc_orginal] == COLOR_WHITE &&
                    oBuffer[loc_orginal + WIDTH] == color_opposite &&
                    oBuffer[loc_orginal - 1] == color_opposite)
                {
                    uBuffer[loc_filter + WIDTH * 2] = color_changed;
                }
                else
                {
                    uBuffer[loc_filter + WIDTH * 2] = color_original;
                }

                // unten links
                if (
                    ((loc_orginal + 1) % WIDTH != 0) &&
                    (loc_orginal > WIDTH) &&
                    //riginal.pixels[loc_orginal] == COLOR_WHITE &&
                    oBuffer[loc_orginal - WIDTH] == color_opposite &&
                    oBuffer[loc_orginal + 1] == color_opposite)
                {
                    uBuffer[loc_filter + 1] = color_changed;
                }
                else
                {
                    uBuffer[loc_filter + 1] = color_original;
                }

                // unten rechts
                if (
                    (loc_orginal % WIDTH != 0) &&
                    (loc_orginal > WIDTH) &&
                    //oBuffer[loc_orginal] == COLOR_BLACK &&
                    oBuffer[loc_orginal - WIDTH] == color_opposite &&
                    oBuffer[loc_orginal - 1] == color_opposite)
                {
                    uBuffer[loc_filter] = color_changed;
                }
                else
                {
                    uBuffer[loc_filter] = color_original;
                }
            }
        }
    }

    /*
    void displayLineOnILI9341(uint16_t xPos, uint16_t yStart, uint16_t yLength, uint8_t color8Bit);
    void manipulateDisplayBuffer();
    void drawFilterSet();
    void drawBackground();
    void toggleZoom();
    void toggleFilterSet();
    void changeFilterSet(uint8_t set);
    void set16BitFilterColors(uint16_t lightColor, uint16_t lightGrey, uint16_t darkGrey, uint16_t darkColor);
    */

    uint32_t nextDisplayModeChange;

    bool zoomActive;

    const uint16_t color1to16Bit[2] = {TFT_BLACK, TFT_WHITE};

    uint8_t oBuffer[HEIGHT * WIDTH];
    uint8_t uBuffer[(HEIGHT * 2 * WIDTH * 2)];
    uint8_t cBuffer[(HEIGHT * 2 * WIDTH * 2)];

    // hold the four colors of the current filter
    uint16_t filterColorsArray[4];
    uint8_t filterSet;

    bool forceCompleteDraw;
};

#endif