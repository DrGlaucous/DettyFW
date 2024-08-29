#pragma once

#include <DShotRMT.h>
#include <Arduino.h>
#include "configuration.h"


typedef enum shoot_mode_e
{
    SHOOT_MODE_NULL,

    SHOOT_MODE_FULL_AUTO,
    SHOOT_MODE_SELECT_FIRE,
    SHOOT_MODE_CACHE,

    SHOOT_MODE_MAX,

}shoot_mode_t;
typedef enum trigger_mode_e
{
    TRIGGER_MODE_NULL,

    TRIGGER_MODE_PRESS,
    TRIGGER_MODE_COMMIT,

    TRIGGER_MODE_MAX,
}trigger_mode_t;

enum presets_e
{
    PRESET_A = 0,
    PRESET_B = 1,
    PRESET_C = 2,
};

//the settings for a single configuration (selectable with the preset buttons)
//unique to each preset
typedef struct live_settings_s
{

    int pusher_rate = PUSHER_SPEED_MIN; //from PUSHER_SPEED_MIN to PUSHER_SPEED_MAX
    int flywheel_speed = FLYWHEEL_SPEED_MIN; //from FLYWHEEL_SPEED_MIN to FLYWHEEL_SPEED_MAX

    //these need to be common types because of the menu
    int shoot_mode = SHOOT_MODE_FULL_AUTO; //shoot_mode_t
    int burst_count = 1;
    int cache_delay = 300; //milliseconds
    int trigger_mode = TRIGGER_MODE_COMMIT; //trigger_mode_t


} live_settings_t;

//settings common to all presets
typedef struct all_settings_s
{

    int esc_output_min = ESC_OUTPUT_MIN; //see configuration.h
    int esc_output_max = ESC_OUTPUT_MAX;

    int esc_rpm_min = ESC_RPM_MIN; //ditto
    int esc_rpm_max = ESC_RPM_MAX;

//TODO: the rest of the pusher settings for other backends
#if PUSHER_TYPE == PUSHER_TYPE_SOLENOID
    int so_min_ext_time = SO_MIN_EXT_TIME;
    int so_min_ret_time = SO_MIN_RET_TIME;
    int so_max_ext_time = SO_MAX_EXT_TIME;
    int so_max_ret_time = (SO_MAX_EXT_TIME / SO_MIN_EXT_TIME) * SO_MIN_RET_TIME;
#endif

    //wifi: (TODO)

    int selected_preset = 0;
    int preset_hold_time = 1000;

    //settings that do change often
    live_settings_t preset_settings[3];

} all_settings_t;


// This section is currently unused.
// I wanted to make the whole thing dynamic, simmilar to how reprapFirmware works,
// but some of the libraries don't allow that to happen.
// these main settings (ones that have to do with pin assignments or hard-baked numbers)
// will need recompilation and reupload to change
// blaster preset settings on the other hand will use a .json as originally intended
/*
//main setting types
typedef enum esc_type_e
{
    ESC_TYPE_NULL,
    ESC_TYPE_BRUSHED,
    ESC_TYPE_PWM,
    ESC_TYPE_DSHOT,
    ESC_TYPE_MAX,

}esc_type_t;
typedef enum pusher_type_e
{
    PUSHER_TYPE_NULL,
    PUSHER_TYPE_BRUSHED,
    PUSHER_TYPE_STEPPER,
    PUSHER_TYPE_SOLENOID,
    PUSHER_TYPE_MAX,
}pusher_type_t;

//all settings in the blaster (as set by configuration.h)
typedef struct all_settings_s
{
    //////////////////////////
    //Flywheel configuration
    //////////////////////////
    esc_type_t esc_type;

    unsigned int esc_rev_delay;

    unsigned int FLYWHEEL_SPEED_MIN;
    unsigned int FLYWHEEL_SPEED_MAX;

    unsigned int esc_output_min;
    unsigned int esc_output_max;

    unsigned int esc_rpm_min;
    unsigned int esc_rpm_max;

    //only used if dshot is enabled
    dshot_mode_t esc_dshot_mode;
    bool esc_dshot_bidir;
    unsigned short esc_dshot_motor_poles;

    //pinouts
    unsigned char esc_1_pin;
    unsigned char esc_2_pin;
    unsigned char esc_3_pin;
    unsigned char esc_4_pin;

    //////////////////////////
    //Pusher configuration
    //////////////////////////
    pusher_type_t pusher_type;

    //vars for brushed pusher type
    unsigned char h_a_pin;
    unsigned char h_b_pin;

    //vars for stepper pusher type
    unsigned char s_step_pin;
    unsigned char s_enable_pin;
    unsigned char s_direction_pin;

    //vars for solenoid pusher type
    unsigned char so_fet_pin;
    unsigned int so_min_ext_time;
    unsigned int so_min_ret_time;
    unsigned int so_max_ext_time;


    //////////////////////////
    //inputs
    //////////////////////////
    unsigned int debounce_time;
    
    unsigned char trigger_pin;
    bool trigger_nc;
    unsigned char mag_trig_pin;
    bool mag_trig_nc;
    unsigned char rev_trig_pin;
    bool rev_trig_nc;

    unsigned char preset_a_pin;
    bool preset_a_nc;
    unsigned char preset_b_pin;
    bool preset_b_nc;
    unsigned char preset_c_pin;
    bool preset_c_nc;

    unsigned char encoder_a_pin;
    unsigned char encoder_b_pin;
    unsigned char encoder_button_pin;
    bool encoder_button_nc;
    //ammoc counter
    unsigned char ir_rec_power_pin;
    unsigned char ir_rec_feedback_pin;
    unsigned char ir_emitter_pin;
    //voltage divider
    unsigned char voltmeter_pin;
    //voltage divider resist values
    unsigned int r1_val;
    unsigned int r2_val;
    unsigned int adc_value_ref;
    float adc_voltage_ref;
    float battery_full_voltage;
    float battery_empty_voltage;

    //////////////////////////
    //display
    //////////////////////////
    unsigned char i2c_sda_pin;
    unsigned char i2c_scl_pin;
    unsigned short display_width;
    unsigned short display_height;

    //the diplay driver is u8g2. 
    //this cannot be changed live. it must be set during compile-time.

    unsigned int baud_rate;


    live_settings_t preset_settings[3];

} all_settings_t;

*/


