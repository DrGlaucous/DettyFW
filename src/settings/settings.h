#pragma once

#include <vector>
#include <string>

#include <Arduino.h>
#include <ArduinoJson.h>

using std::string;
using std::vector;


#define SETTINGS_DIRECTORY "/settings/settings.json"

//convention:
/*
variables use snake_case names
classes, enums, and structs use CamelCase names
use of enums must always use the qualifier: enum::qualifier
*/

typedef enum ShootMode {
    ShootModeNull = 0,
    ShootModeFullAuto = 1,
    ShootModeSelectFire = 2,
    ShootModeCache = 3,
    ShootModeMax,

}ShootMode;

typedef enum TriggerMode {
    TriggerModeNull = 0,
    TriggerModePress = 1,
    TriggerModeCommit = 2,
    TriggerModeMax,
}TriggerMode;

// Not the same as the definition in DSHOT_RMT_NEO!
typedef enum DshotMode
{
    DshotOff,
    Dshot150,
    Dshot300,
    Dshot600,
    Dshot1200,
} DshotMode;

////////////////////////////////////////////////

class DigitalInputSettings {

    public:

    int16_t pin = -1;
    bool pullup;
    bool normally_closed = false;
    size_t debounce_time = 0;

    void unpack(JsonObject pin_settings) {
        pin = pin_settings["num"];
        normally_closed = pin_settings["nc"];
        pullup = pin_settings["pullup"];
        debounce_time = pin_settings["debounce_time"];
    }

    JsonDocument pack() {
        JsonDocument settings;
        settings["num"] = pin;
        settings["nc"] = normally_closed;
        settings["pullup"] = pullup;
        settings["debounce_time"] = debounce_time;
        return settings;
    }

};



class DigitalOutputSettings {
    
    public:

    int16_t pin = -1;
    bool on_high = false;

    void unpack(JsonObject pin_settings) {
        pin = pin_settings["num"];
        on_high = pin_settings["on_high"];
    }

    JsonDocument pack() {
        JsonDocument settings;
        settings["num"] = pin;
        settings["on_high"] = on_high;
        return settings;
    }

};

////////////////////////////////////////////////

typedef struct DebugSettings {
    bool enabled = false;
    size_t baud_rate = 115200;
} DebugSettings;

typedef struct OledUserInterfaceSettings {
    DigitalInputSettings preset_a;
    DigitalInputSettings preset_b;
    DigitalInputSettings preset_c;
    DigitalInputSettings encoder_button;
    DigitalOutputSettings buzzer;
    int16_t encoder_a_pin = -1;
    int16_t encoder_b_pin = -1;
    int16_t i2c_sda_pin = -1;
    int16_t i2c_scl_pin = -1;
    size_t display_width = 0;
    size_t display_height = 0;
    int16_t preset_a_index = -1;
    int16_t preset_b_index = -1;
    int16_t preset_c_index = -1;

} OledUserInterfaceSettings;

typedef struct VoltmeterSettings {
    int16_t voltmeter_read_pin = -1;
    uint16_t adc_value_ref_1 = 0;
    float voltage_ref_1 = 0.0;
    uint16_t adc_value_ref_2 = 1;
    float voltage_ref_2 = 0.0;
    float batt_full_charge = 0.0;
    float batt_empty_charge = 0.0;

} VoltmeterSettings;

typedef struct IRDetectorSettings {
    DigitalInputSettings mag_release;
    DigitalOutputSettings ir_reciever_power;
    DigitalOutputSettings ir_emitter_power;
    int16_t ir_reciever_read_pin = -1;
    uint16_t adc_falling_threshhold = 0;
    uint16_t adc_rising_threshhold = 0;
    float dart_length_mm = 0.0;

} IRDetectorSettings;

typedef struct PusherSettings {
    DigitalOutputSettings fet;
    size_t min_extend_time_ms = 0;
    size_t min_retract_time_ms = 0;
    size_t max_extend_time_ms = 0;
} PusherSettings;

typedef struct FlywheelSettings {
    int16_t motor_l_pin = -1;
    int16_t motor_r_pin = -1;
    DshotMode dshot_mode = DshotMode::DshotOff;
    size_t downthrottle_time_ms = 0;
    size_t idle_rpm = 0;
    size_t idle_time_ms = 0;
    size_t brushless_motor_kv = 0;
    float average_battery_voltage = 0.0;

} FLywheelSettings;

typedef struct HandleSettings {
    DigitalInputSettings shoot_trigger;
    DigitalInputSettings rev_trigger;
} HandleSettings;

typedef struct PresetSettings {

    string name = "";
    TriggerMode trigger_mode = TriggerMode::TriggerModeNull;
    ShootMode shoot_mode = ShootMode::ShootModeNull;
    int burst_count = 0;
    int cache_delay_ms = 0;
    int push_rate_pct = 0;
    int flywheel_rpm = 0;

    //the index in the json that this preset is (this value is NOT stored in the json!)
    int index = 0;

} PresetSettings;


class Settings {


    public:

    Settings(const char* directory);
    ~Settings();

    //deserialize json and populate struct
    bool load();

    //serialize and save json to directory
    bool save();

    //sets preset a,b,or c to refer to config index "index" in the array
    bool set_preset_index(char preset, size_t index);

    //set the "current index" to index
    bool set_current_preset(size_t index);

    //get refrence to the "current" preset
    PresetSettings& get_current_preset();

    private:

    //where in the filesystem the settings are loaded from
    string directory;
    //set to "true" if any settings other than the stuff in "variables" has changed
    bool needs_power_cycle = false;


    //settings from JSON file
    DebugSettings debug_settings = {};
    OledUserInterfaceSettings oled_user_interface_settings = {};
    VoltmeterSettings voltmeter_settings = {};
    IRDetectorSettings ir_detector_settings = {};
    PusherSettings pusher_settings = {};
    FlywheelSettings flywheel_settings = {};
    HandleSettings handle_settings = {};
    vector<PresetSettings> preset_list = {};




    //takes the loaded settings json and populates the above field with it
    void unpack_json(JsonDocument& settings_json);

    //takes the current settings and creates a json document from it
    JsonDocument pack_json();


    //takes the "pin" object and converts it into a digital i/o pin
    DigitalInputSettings unpack_di_settings(JsonObject pin_settings);
    DigitalOutputSettings unpack_do_settings(JsonObject pin_settings);

    //takes the digital i/o pin and packs it into a json object
    JsonDocument pack_di_settings(DigitalInputSettings pin_settings);
    JsonDocument pack_do_settings(DigitalOutputSettings pin_settings);




};










