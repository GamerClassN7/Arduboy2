#ifndef ARDUBOY2_BASE_H
#define ARDUBOY2_BASE_H

#include <Arduino.h>
#include "../src/Arduboy2Core.h"

#define DISPLAY_BYTES ((WIDTH * HEIGHT) / 8)

class Display_Base
{
public:
    Display_Base(){};

    virtual void init();

    virtual void display(); 
    
    void drawPixel(int16_t x, int16_t y, uint8_t color) {
        uint8_t row = (uint8_t)y / 8;
        if (color)
        {
            sBuffer[(row * WIDTH) + (uint8_t)x] |= _BV((uint8_t)y % 8);
        }
        else
        {
            sBuffer[(row * WIDTH) + (uint8_t)x] &= ~_BV((uint8_t)y % 8);
        }
    }

    void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color)
    {
        int16_t length = h;

        if (x < 0 || x >= WIDTH)
            return;

        if (y < 0)
        {
            length += y;
            y = 0;
        }

        if ((y + length) > HEIGHT)
        {
            length = (HEIGHT - y);
        }

        if (length <= 0)
            return;

        uint8_t yOffset = y & 7;
        uint8_t drawBit;
        uint8_t *bufferPtr = sBuffer;

        bufferPtr += (y >> 3) * WIDTH;
        bufferPtr += x;

        if (yOffset)
        {
            yOffset = 8 - yOffset;
            drawBit = ~(0xFF >> (yOffset));

            if (length < yOffset)
            {
                drawBit &= (0xFF >> (yOffset - length));
            }

            switch (color)
            {
            case WHITE:
                *bufferPtr |= drawBit;
                break;
            case BLACK:
                *bufferPtr &= ~drawBit;
                break;
            case INVERSE:
                *bufferPtr ^= drawBit;
                break;
            }

            if (length < yOffset)
                return;

            length -= yOffset;
            bufferPtr += WIDTH;
        }

        if (length >= 8)
        {
            switch (color)
            {
            case WHITE:
            case BLACK:
                drawBit = (color == WHITE) ? 0xFF : 0x00;
                do
                {
                    *bufferPtr = drawBit;
                    bufferPtr += WIDTH;
                    length -= 8;
                } while (length >= 8);
                break;
            case INVERSE:
                do
                {
                    *bufferPtr = ~(*bufferPtr);
                    bufferPtr += WIDTH;
                    length -= 8;
                } while (length >= 8);
                break;
            }
        }

        if (length > 0)
        {
            drawBit = (1 << (length & 7)) - 1;
            switch (color)
            {
            case WHITE:
                *bufferPtr |= drawBit;
                break;
            case BLACK:
                *bufferPtr &= ~drawBit;
                break;
            case INVERSE:
                *bufferPtr ^= drawBit;
                break;
            }
        }
    }

    void fillScreen(uint8_t color)
    {
        if (color == BLACK)
        {
            memset(sBuffer, 0x00, DISPLAY_BYTES);
        }
        else
        {
            memset(sBuffer, 0xff, DISPLAY_BYTES);
        }
    }

    void clear() 
    {   
        // clear the first 1024 of the display buffer
        memset(sBuffer, 0x00, DISPLAY_BYTES);       
    }
    
    void newFrame() 
    {

    }

    void paintScreen(const uint8_t *image)
    {

    }

    void redraw() 
    {

    }

    uint8_t *sBuffer;
};

#endif