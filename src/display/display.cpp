
#include <Arduino.h>
#include <U8g2lib.h>

#include "../settings/settings.h"
#include "../state/state.h"
#include "../controller/controller.h"

#include "display.h"


//setup i2c display, see https://github.com/olikraus/u8g2/wiki/u8g2setupcpp for a list of proper constructors
//currently, this sets up a generic 128x64 display on the hardware i2c BUS
//#define INIT_CLASS U8G2_SSD1306_128X64_NONAME_1_HW_I2C
//u8g2(U8G2_R0, U8X8_PIN_NONE , I2C_SCL_PIN, I2C_SDA_PIN)
//INIT_CLASS u8g2(U8G2_R0, U8X8_PIN_NONE , I2C_SCL_PIN, I2C_SDA_PIN);



Display::Display(Settings& settings) {

    auto oled = settings.get_oled_interface_settings_ref();
    width = oled.display_width == 0 ? 1 : oled.display_width;
    height = oled.display_height == 0 ? 1 : oled.display_height;
    
    width = 128;
    height = 64;

    int16_t i2c_scl = 22;
    int16_t i2c_sda = 19;

    //todo: add other u8g2 displays here
    int display_mode = 0;
    switch(display_mode) {
        default: {
            display = new U8G2_SSD1306_128X64_NONAME_1_HW_I2C(U8G2_R0, U8X8_PIN_NONE , i2c_scl, i2c_sda);
            break;
        }
        case 1: {
            display = new U8G2_SSD1306_64X48_ER_1_HW_I2C(U8G2_R0, U8X8_PIN_NONE , i2c_scl, i2c_sda);
            break;
        }
    }


    display->begin();
    

}
Display::~Display() {
    delete display;
}

void Display::tick(const DisplayData& data) {

    //auto preset = state.settings.get_current_preset_mut();

    
    //flywheel_rpm = preset->flywheel_rpm;

    display->firstPage();
    do {
        draw_stuff(data);
    } while (display->nextPage());

}

void Display::draw_stuff(const DisplayData& data) {

    display->setFont(u8g2_font_ncenB14_tr);

    sprintf(numeric_str, "TGT: %1d", data.target_speed);
    display->drawStr(
        origin_left + 0,
        origin_top + 16,
        numeric_str);

    sprintf(numeric_str, "L: %1d", data.wheel_speed_l);
    display->drawStr(
        origin_left + 0,
        origin_top + 32,
        numeric_str);    
    sprintf(numeric_str, "R: %1d", data.wheel_speed_r);
    display->drawStr(
        origin_left + 0,
        origin_top + 48,
        numeric_str);

    sprintf(numeric_str, "x+- | x%1d", data.multiplier);
    display->drawStr(
        origin_left + 0,
        origin_top + 64,
        numeric_str);
    //display->drawBox(0,0,64,64);
}




















