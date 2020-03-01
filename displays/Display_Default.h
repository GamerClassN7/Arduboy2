#ifndef DISPLAY_Default_H
#define DISPLAY_Default_H

#include <Arduino.h>
#include "Display_Base.h"
#include "../src/Arduboy2Core.h"

class Display_Default : public Display_Base
{
public:
    Display_Default(){};

    void init()
    {
    }

    void drawPixel(int16_t x, int16_t y, uint8_t color)
    {
        uint16_t row_offset;
        uint8_t bit;

        asm volatile(
            // bit = 1 << (y & 7)
            "ldi  %[bit], 1                    \n" //bit = 1;
            "sbrc %[y], 1                      \n" //if (y & _BV(1)) bit = 4;
            "ldi  %[bit], 4                    \n"
            "sbrc %[y], 0                      \n" //if (y & _BV(0)) bit = bit << 1;
            "lsl  %[bit]                       \n"
            "sbrc %[y], 2                      \n" //if (y & _BV(2)) bit = (bit << 4) | (bit >> 4);
            "swap %[bit]                       \n"
            //row_offset = y / 8 * WIDTH + x;
            "andi %A[y], 0xf8                  \n" //row_offset = (y & 0xF8) * WIDTH / 8
            "mul  %[width_offset], %A[y]       \n"
            "movw %[row_offset], r0            \n"
            "clr  __zero_reg__                 \n"
            "add  %A[row_offset], %[x]         \n" //row_offset += x
#if WIDTH != 128
            "adc  %B[row_offset], __zero_reg__ \n" // only non 128 width can overflow
#endif
            : [row_offset] "=&x"(row_offset), // upper register (ANDI)
              [bit] "=&d"(bit),               // upper register (LDI)
              [y] "+d"(y)                     // upper register (ANDI), must be writable
            : [width_offset] "r"((uint8_t)(WIDTH / 8)),
              [x] "r"((uint8_t)x)
            :);
        uint8_t data = sBuffer[row_offset] | bit;
        if (!(color & _BV(0)))
            data ^= bit;
        sBuffer[row_offset] = data;
    }

    void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color)
    {
        int16_t end = y + h;
        for (int16_t a = max(0, y); a < min(end, HEIGHT); a++)
        {
            drawPixel(x, a, color);
        }
    }

    void fillScreen(uint8_t color)
    {
        // C version:
        //
        // if (color != BLACK)
        // {
        //   color = 0xFF; // all pixels on
        // }
        // for (int16_t i = 0; i < WIDTH * HEIGTH / 8; i++)
        // {
        //    sBuffer[i] = color;
        // }

        // This asm version is hard coded for 1024 bytes. It doesn't use the defined
        // WIDTH and HEIGHT values. It will have to be modified for a different
        // screen buffer size.
        // It also assumes color value for BLACK is 0.

        // local variable for screen buffer pointer,
        // which can be declared a read-write operand

        uint8_t *bPtr = sBuffer;

        asm volatile(
            // if value is zero, skip assigning to 0xff
            "cpse %[color], __zero_reg__\n"
            "ldi %[color], 0xFF\n"
            // counter = 0
            "clr __tmp_reg__\n"
            "loopto:\n"
            // (4x) push zero into screen buffer,
            // then increment buffer position
            "st Z+, %[color]\n"
            "st Z+, %[color]\n"
            "st Z+, %[color]\n"
            "st Z+, %[color]\n"
            // increase counter
            "inc __tmp_reg__\n"
            // repeat for 256 loops
            // (until counter rolls over back to 0)
            "brne loopto\n"
            : [color] "+d"(color),
              "+z"(bPtr)
            :
            :);
    }

    void newFrame()
    {
    }

    void display()
    {
        paintScreen(sBuffer);
    }

    void clear() 
    {
        memset(sBuffer, 0x00, 1024);
    }

    uint8_t sBuffer[(HEIGHT * WIDTH) / 8];
};

#endif