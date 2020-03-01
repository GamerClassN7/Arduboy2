#ifndef ARDUBOY2_CONFIG_H
#define ARDUBOY2_CONFIG_H

#ifdef DISPLAY_Default
#include "../displays/Display_Default.h"
DISPLAY_Default arduboyDisplay;
#endif

#ifdef DISPLAY_Adafruit
#include "../displays/Display_Adafruit.h"
Display_Adafruit arduboyDisplay;
#endif

#ifdef DISPLAY_TFT_eSPI
#include "../displays/Display_TFT_eSPI.h"
Display_TFT_eSPI arduboyDisplay;
#endif

#ifdef DISPLAY_SSD1306
#include "../displays/Display_SSD1306.h"
Display_SSD1306 arduboyDisplay;
#endif

#endif