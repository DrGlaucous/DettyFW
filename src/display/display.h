#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "../settings/settings.h"
#include "../state/state.h"

/*

priority ranking
flywheel transmission (via t. ISR, always active)
chronometer (via t. ISR, active when trigger held)
encoder (via ISR, always active)
user buttons 

should I keep the menu drawing + handling routine in the same task?

menu R/W presets ONLY (load copy and )
connectome R/W state


*/

//temp. struct for pre-packaging display information
typedef struct DisplayData {
    uint32_t target_speed = 0;
    uint32_t wheel_speed_l = 0;
    uint32_t wheel_speed_r = 0;
    uint32_t multiplier = 1;
} DisplayData;


class Display {

    public:

    Display(Settings& settings);
    ~Display();

    void tick(const DisplayData& data);

    private:

    void draw_stuff(const DisplayData& data);


    U8G2* display;

    char numeric_str[64] = {};

    //size of the display, used for drawing items onto it
    uint16_t origin_left = 0;
    uint16_t origin_top = 0;
    uint16_t width = 1;
    uint16_t height = 1;

    int16_t selected_digit = 0;
    int32_t flywheel_rpm = 0;

};



