/*

  _       _   _            ____
 | |     (_) | |  _   _   / ___|   ___
 | |     | | | | | | | | | |  _   / _ \
 | |___  | | | | | |_| | | |_| | | (_) |
 |_____| |_| |_|  \__, |  \____|  \___/
                  |___/

website:https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library
Written by Lewis he //https://github.com/lewisxhe
*/

#include "TTGO.h"

TTGOClass *TTGOClass::_ttgo = nullptr;

EventGroupHandle_t TTGOClass::_tpEvent = nullptr;


#include "drive/i2c/i2c_bus.cpp"
#include "drive/axp/axp20x.cpp"
#include "drive/rtc/pcf8563.cpp"
#include "drive/bma423/bma.cpp"
#include "drive/fx50xx/focaltech.cpp"

#include "libraries/TFT_eSPI/TFT_eSPI.cpp"
