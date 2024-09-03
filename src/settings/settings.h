#pragma once

#include <vector>
#include <string>

#include <Arduino.h>
#include <ArduinoJson.h>

using std::string;
using std::vector;

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

    void unpack(JsonObject settings) {
        pin = settings["num"];
        normally_closed = settings["nc"];
        pullup = settings["pullup"];
        debounce_time = settings["debounce_time"];
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

    void unpack(JsonObject settings) {
        pin = settings["num"];
        on_high = settings["on_high"];
    }

    JsonDocument pack() {
        JsonDocument settings;
        settings["num"] = pin;
        settings["on_high"] = on_high;
        return settings;
    }

};

////////////////////////////////////////////////

class DebugSettings {

    public:

    bool enabled = false;
    size_t baud_rate = 115200;

    void unpack(JsonObject settings) {
        enabled = settings["enabled"];
        baud_rate = settings["baud_rate"];
    }

    JsonDocument pack() {
        JsonDocument obj;
        obj["enabled"] = enabled;
        obj["baud_rate"] = baud_rate;
        return obj;
    }

};

class OledUserInterfaceSettings {
    public:

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

    void unpack(JsonObject settings) {

        JsonObject pins = settings["pins"];

        //get inputs
        JsonObject pin = pins["preset_a"];
        preset_a.unpack(pin);
        pin = pins["preset_b"];
        preset_b.unpack(pin);
        pin = pins["preset_c"];
        preset_c.unpack(pin);
        pin = pins["encoder_button"];
        encoder_button.unpack(pin);


        //get output
        pin = pins["buzzer"];
        buzzer.unpack(pin);

        //other I/O
        pin = pins["i2c"];
        i2c_sda_pin = pin["sda"];
        i2c_scl_pin = pin["scl"];

        pin = pins["encoder"];
        encoder_a_pin = pin["num_a"];
        encoder_b_pin = pin["num_b"];

        preset_a_index = settings["preset_a_index"];
        preset_b_index = settings["preset_b_index"];
        preset_c_index = settings["preset_c_index"];

        display_width = settings["display_width"];
        display_height = settings["display_height"];
    }

    JsonDocument pack() {
        JsonDocument obj;
        JsonDocument pins;

        pins["i2c"]["sda"] = i2c_sda_pin;
        pins["i2c"]["scl"] = i2c_scl_pin;
        
        pins["preset_a"] = preset_a.pack();
        pins["preset_b"] = preset_b.pack();
        pins["preset_c"] = preset_c.pack();
        pins["encoder_button"] = encoder_button.pack();

        pins["encoder"]["num_a"] = encoder_a_pin;
        pins["encoder"]["num_b"] = encoder_b_pin;
        pins["buzzer"] = buzzer.pack();

        obj["preset_a_index"] = preset_a_index;
        obj["preset_b_index"] = preset_b_index;
        obj["preset_c_index"] = preset_c_index;

        obj["display_width"] = display_width;
        obj["display_height"] = display_height;

        obj["pins"] = pins;
        return obj;
    }

};

class VoltmeterSettings {

    public:

    int16_t voltmeter_read_pin = -1;
    uint16_t adc_value_ref_1 = 0;
    float voltage_ref_1 = 0.0;
    uint16_t adc_value_ref_2 = 1;
    float voltage_ref_2 = 0.0;
    float batt_full_charge = 0.0;
    float batt_empty_charge = 0.0;

    void unpack(JsonObject settings) {
        voltmeter_read_pin = settings["voltmeter_read_pin"]["num"];
        adc_value_ref_1 = settings["adc_value_ref_1"];
        voltage_ref_1 = settings["voltage_ref_1"];
        adc_value_ref_2 = settings["adc_value_ref_2"];
        voltage_ref_2 = settings["voltage_ref_2"];

        batt_full_charge = settings["batt_full_charge"];
        batt_empty_charge = settings["batt_empty_charge"];
    }

    JsonDocument pack() {
        JsonDocument obj;

        obj["voltmeter_read_pin"]["num"] = voltmeter_read_pin;
        obj["adc_value_ref_1"] = adc_value_ref_1;
        obj["voltage_ref_1"] = voltage_ref_1;
        obj["adc_value_ref_2"] = adc_value_ref_2;
        obj["voltage_ref_2"] = voltage_ref_2;

        obj["batt_full_charge"] = batt_full_charge;
        obj["batt_empty_charge"] = batt_empty_charge;

        return obj;
    }

};

class IRDetectorSettings {

    public:

    DigitalInputSettings mag_release;
    DigitalOutputSettings ir_reciever_power;
    DigitalOutputSettings ir_emitter_power;
    int16_t ir_reciever_read_pin = -1;
    uint16_t adc_falling_threshhold = 0;
    uint16_t adc_rising_threshhold = 0;
    float dart_length_mm = 0.0;

    void unpack(JsonObject settings) {
        JsonObject pins = settings["pins"];

        //I/O
        JsonObject pin = pins["ir_reciever_power"];
        ir_reciever_power.unpack(pin); // = unpack_do_settings(pin);
        pin = pins["ir_emitter_power"];
        ir_emitter_power.unpack(pin); // = unpack_do_settings(pin);
        pin = pins["ir_reciever_read_pin"];
        ir_reciever_read_pin = pin["num"];
        pin = pins["mag_release"];
        mag_release.unpack(pin); // = unpack_di_settings(pin);

        adc_falling_threshhold = settings["adc_falling_threshhold"];
        adc_rising_threshhold = settings["adc_rising_threshhold"];
        dart_length_mm = settings["dart_length_mm"];
    }

    JsonDocument pack() {
        JsonDocument obj;
        JsonDocument pins;

        pins["ir_reciever_power"] = ir_reciever_power.pack();
        pins["ir_emitter_power"] = ir_emitter_power.pack();
        pins["ir_reciever_read_pin"]["num"] = ir_reciever_read_pin;
        pins["mag_release"] = mag_release.pack();
        
        obj["adc_falling_threshhold"] = adc_falling_threshhold;
        obj["adc_rising_threshhold"] = adc_rising_threshhold;
        obj["dart_length_mm"] = dart_length_mm;

        obj["pins"] = pins;
        return obj;
    }

};

class PusherSettings {

    public:

    DigitalOutputSettings fet;
    size_t min_extend_time_ms = 0;
    size_t min_retract_time_ms = 0;
    size_t max_extend_time_ms = 0;

    void unpack(JsonObject settings) {
        JsonObject pin = settings["pins"]["fet"];
        fet.unpack(pin); // = unpack_do_settings(pin);

        min_extend_time_ms = settings["min_extend_time_ms"];
        min_retract_time_ms = settings["min_retract_time_ms"];
        max_extend_time_ms = settings["max_extend_time_ms"];
    }

    JsonDocument pack() {
        JsonDocument obj;

        obj["pins"]["fet"] = fet.pack();
        obj["min_extend_time_ms"] = min_extend_time_ms;
        obj["min_retract_time_ms"] = min_retract_time_ms;
        obj["max_extend_time_ms"] = max_extend_time_ms;
        
        return obj;
    }

};

class FlywheelSettings {

    public:

    int16_t motor_l_pin = -1;
    int16_t motor_r_pin = -1;
    DshotMode dshot_mode = DshotMode::DshotOff;
    size_t downthrottle_time_ms = 0;
    size_t idle_rpm = 0;
    size_t idle_time_ms = 0;
    size_t brushless_motor_kv = 0;
    float average_battery_voltage = 0.0;

    void unpack(JsonObject settings) {
        motor_l_pin = settings["pins"]["motor_l_pin"];
        motor_r_pin = settings["pins"]["motor_r_pin"];

        //enumify dshot mode mode
        dshot_mode = (DshotMode)settings["trigger_mode"];

        downthrottle_time_ms = settings["downthrottle_time_ms"];
        idle_rpm = settings["idle_rpm"];
        idle_time_ms = settings["idle_time_ms"];
        brushless_motor_kv = settings["brushless_motor_kv"];
        average_battery_voltage = settings["average_battery_voltage"];
    }

    JsonDocument pack() {
        JsonDocument obj;

        obj["pins"]["motor_l_pin"] = motor_l_pin;
        obj["pins"]["motor_r_pin"] = motor_r_pin;
        obj["dshot_mode"] = dshot_mode;
        obj["downthrottle_time_ms"] = downthrottle_time_ms;
        obj["idle_rpm"] = idle_rpm;
        obj["idle_time_ms"] = idle_time_ms;
        obj["brushless_motor_kv"] = brushless_motor_kv;
        obj["average_battery_voltage"] = average_battery_voltage;

        return obj;
    }

};

class HandleSettings {

    public:

    DigitalInputSettings shoot_trigger;
    DigitalInputSettings rev_trigger;

    void unpack(JsonObject settings) {
        JsonObject pin = settings["shoot_trigger"];
        shoot_trigger.unpack(pin); // = unpack_di_settings(pin);
        pin = settings["rev_trigger"];
        rev_trigger.unpack(pin); // = unpack_di_settings(pin);
    }

    JsonDocument pack() {
        JsonDocument obj;

        obj["shoot_trigger"] = shoot_trigger.pack();//pack_di_settings(handle_settings.shoot_trigger);
        obj["rev_trigger"] = rev_trigger.pack();//pack_di_settings(handle_settings.rev_trigger);

        return obj;
    }

};

class PresetSettings {

    public:

    string name = "";
    TriggerMode trigger_mode = TriggerMode::TriggerModeNull;
    ShootMode shoot_mode = ShootMode::ShootModeNull;
    int burst_count = 0;
    int cache_delay_ms = 0;
    int push_rate_pct = 0;
    int flywheel_rpm = 0;

    //the index in the json that this preset is (this value is NOT stored in the json!)
    int index = 0;

    void unpack(JsonObject settings) {
        //parse the preset
        name = string((const char*)settings["name"]);

        //enumify trigger mode
        trigger_mode = (TriggerMode)settings["trigger_mode"];
        //enumify shoot mode
        shoot_mode = (ShootMode)settings["shoot_mode"];

        burst_count = settings["burst_count"];
        cache_delay_ms = settings["cache_delay_ms"];
        push_rate_pct = settings["push_rate_pct"];
        flywheel_rpm = settings["flywheel_rpm"];

    }

    JsonDocument pack() {
        JsonDocument preset_settings;
        preset_settings["name"] = name.c_str();

        //de-enumify mode typedefs
        preset_settings["trigger_mode"] = (size_t)trigger_mode;
        preset_settings["shoot_mode"] = (size_t)shoot_mode;
        preset_settings["burst_count"] = burst_count;
        preset_settings["cache_delay_ms"] = cache_delay_ms;
        preset_settings["push_rate_pct"] = push_rate_pct;
        preset_settings["flywheel_rpm"] = flywheel_rpm;

        return preset_settings;
    }

};


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
    void unpack_json(JsonObject settings_json);

    //takes the current settings and creates a json document from it
    JsonDocument pack_json();


};










