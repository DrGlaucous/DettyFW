#include <string>

#include <Arduino.h>
#include <ArduinoJson.h>

#include "settings.h"
#include "../filesystem/filesystem.h"

using std::string;


//arduinoJson failure modes:
//objects and lists return "null" object if incorrectly addressed (any addressing inside this will return null sub-parts)
//numerics return "0"
//strings return nullptr, so watch out


Settings::Settings(const char* directory) {
    this->directory = string(directory);
}
Settings::~Settings() {

}


bool Settings::load() {

    uint8_t *filebuf;
    size_t filelen;
    auto result = filesystem::readFile(directory.c_str(), &filebuf, &filelen);

    //failed to load settings, reset to default
    if(!result) {
        *this = Settings(this->directory.c_str());
        return false;
    }
    JsonDocument settings_json;

    DeserializationError err = deserializeJson(settings_json, *filebuf, filelen);

    //finished decoding, free filebuffer
    free(filebuf);

    //parse out settings
    unpack_json(settings_json);

}
bool Settings::save() {

    //pack settings
    JsonDocument settings_json = pack_json();

    //calculate the size of the json + null terminator so we can allocate a buffer to fit it
    size_t json_size = measureJsonPretty(settings_json) + 1;
    uint8_t* json_buffer = (uint8_t*)malloc(json_size);
    //write to buffer
    serializeJsonPretty(settings_json, json_buffer, json_size);

    filesystem::writeFile(directory.c_str(), json_buffer, json_size);

    //finished writing, free filebuffer
    free(json_buffer);

}

//reads the json at index and returns 
void Settings::unpack_json(JsonDocument settings_json) {

    //unpack other
    {
        //debug
        {
            JsonObject obj = settings_json["debug"];
            debug_settings.enabled = obj["enabled"];
            debug_settings.baud_rate = obj["baud_rate"];
        }
        //oled
        {
            JsonObject obj = settings_json["oled_user_interface"];
            JsonObject pins = obj["pins"];

            //get inputs
            JsonObject pin = pins["preset_a"];
            oled_user_interface_settings.preset_a = unpack_di_settings(pin);
            pin = pins["preset_b"];
            oled_user_interface_settings.preset_b = unpack_di_settings(pin);
            pin = pins["preset_c"];
            oled_user_interface_settings.preset_c = unpack_di_settings(pin);
            pin = pins["encoder_button"];
            oled_user_interface_settings.encoder_button = unpack_di_settings(pin);

            //get output
            pin = pins["buzzer"];
            oled_user_interface_settings.buzzer = unpack_do_settings(pin);

            //other I/O
            pin = pins["i2c"];
            oled_user_interface_settings.i2c_sda_pin = pin["sda"];
            oled_user_interface_settings.i2c_scl_pin = pin["scl"];

            pin = pins["encoder"];
            oled_user_interface_settings.i2c_scl_pin = pin["scl"];
            oled_user_interface_settings.i2c_scl_pin = pin["scl"];

            oled_user_interface_settings.display_width = obj["display_width"];
            oled_user_interface_settings.display_height = obj["display_height"];



        }
        //voltmeter
        {
            JsonObject obj = settings_json["voltmeter"];

            voltmeter_settings.voltmeter_read_pin = obj["voltmeter_read_pin"]["num"];
            voltmeter_settings.adc_value_ref_1 = obj["adc_value_ref_1"];
            voltmeter_settings.voltage_ref_1 = obj["voltage_ref_1"];
            voltmeter_settings.adc_value_ref_2 = obj["adc_value_ref_2"];
            voltmeter_settings.voltage_ref_2 = obj["voltage_ref_2"];

            voltmeter_settings.batt_full_charge = obj["batt_full_charge"];
            voltmeter_settings.batt_empty_charge = obj["batt_empty_charge"];

        }
        //ir_detector
        {
            JsonObject obj = settings_json["ir_detector"];
            JsonObject pins = obj["pins"];

            //I/O
            JsonObject pin = pins["ir_reciever_power"];
            ir_detector_settings.ir_reciever_power = unpack_do_settings(pin);
            pin = pins["ir_emitter_power"];
            ir_detector_settings.ir_emitter_power = unpack_do_settings(pin);
            pin = pins["mag_release"];
            ir_detector_settings.mag_release = unpack_di_settings(pin);

            ir_detector_settings.adc_falling_threshhold = obj["adc_falling_threshhold"];
            ir_detector_settings.adc_rising_threshhold = obj["adc_rising_threshhold"];
            ir_detector_settings.dart_length_mm = obj["dart_length_mm"];

        }
        //pusher
        {
            JsonObject obj = settings_json["pusher"];

            JsonObject pin = obj["pins"]["fet"];
            pusher_settings.fet = unpack_do_settings(pin);

            pusher_settings.min_extend_time_ms = obj["min_extend_time_ms"];
            pusher_settings.min_retract_time_ms = obj["min_retract_time_ms"];
            pusher_settings.max_extend_time_ms = obj["max_extend_time_ms"];

        }
        //flywheels
        {
            JsonObject obj = settings_json["flywheels"];

            flywheel_settings.motor_l_pin = obj["pins"]["motor_l_pin"];
            flywheel_settings.motor_l_pin = obj["pins"]["motor_r_pin"];

            //enumify dshot mode mode
            switch((int)obj["trigger_mode"]) {
                case 1:
                    flywheel_settings.dshot_mode = DshotMode::Dshot300;
                    break;
                case 2:
                    flywheel_settings.dshot_mode = DshotMode::Dshot600;
                    break;
                case 3:
                    flywheel_settings.dshot_mode = DshotMode::Dshot1200;
                    break;
                default:
                    flywheel_settings.dshot_mode = DshotMode::DshotOff;
                    break;
            }
            flywheel_settings.downthrottle_time_ms = obj["downthrottle_time_ms"];
            flywheel_settings.idle_rpm = obj["idle_rpm"];
            flywheel_settings.idle_time_ms = obj["idle_time_ms"];
            flywheel_settings.brushless_motor_kv = obj["brushless_motor_kv"];
            flywheel_settings.average_battery_voltage = obj["average_battery_voltage"];

        }
        //handle
        {
            JsonObject obj = settings_json["handle"];
            
            JsonObject pin = obj["shoot_trigger"];
            handle_settings.shoot_trigger = unpack_di_settings(pin);
            pin = obj["rev_trigger"];
            handle_settings.rev_trigger = unpack_di_settings(pin);

        }

    }
    //unpack presets
    {

        JsonArray arr = settings_json["presets"];
        size_t list_len = arr.size();

        //pre-allocate correct-sized vector (will delete old vector)
        preset_list = vector<PresetSettings>(list_len);

        for(int i = 0; i < list_len; ++i) {

            //get the object from our master list
            JsonObject output = arr[i]; //.as<JsonObject>();
            
            //parse the preset
            PresetSettings preset = {};
            preset.name = string((const char*)output["name"]);

            //enumify trigger mode
            switch((int)output["trigger_mode"]) {
                case 1:
                    preset.trigger_mode = TriggerMode::Press;
                    break;
                case 2:
                    preset.trigger_mode = TriggerMode::Commit;
                    break;
                default:
                    preset.trigger_mode = TriggerMode::Null;
                    break;
            }
            //enumify shoot mode
            switch((int)output["shoot_mode"]) {
                case 1:
                    preset.shoot_mode = ShootMode::FullAuto;
                    break;
                case 2:
                    preset.shoot_mode = ShootMode::SelectFire;
                    break;
                case 3:
                    preset.shoot_mode = ShootMode::Cache;
                    break;
                default:
                    preset.shoot_mode = ShootMode::Null;
                    break;
            }

            preset.burst_count = output["burst_count"];
            preset.cache_delay_ms = output["cache_delay_ms"];
            preset.push_rate_pct = output["push_rate_pct"];
            preset.flywheel_rpm = output["flywheel_rpm"];

            preset_list[i] = preset;
        }


    }
    
}

JsonDocument Settings::pack_json() {

    JsonDocument settings;

    //pack other
    {
        //debug
        {
            JsonObject obj = settings["debug"];
            obj["enabled"] = debug_settings.enabled;
            obj["baud_rate"] = debug_settings.baud_rate;

        }
        //oled
        {
            JsonObject obj = settings["oled_user_interface"];
            JsonObject pins = obj["pins"];

            pins["i2c"]["sda"] = oled_user_interface_settings.i2c_sda_pin;
            pins["i2c"]["scl"] = oled_user_interface_settings.i2c_scl_pin;
            
            pins["preset_a"] = pack_di_settings(oled_user_interface_settings.preset_a);
            pins["preset_b"] = pack_di_settings(oled_user_interface_settings.preset_b);
            pins["preset_c"] = pack_di_settings(oled_user_interface_settings.preset_c);
            pins["encoder_button"] = pack_di_settings(oled_user_interface_settings.encoder_button);

            pins["encoder"]["num_a"] = oled_user_interface_settings.encoder_a_pin;
            pins["encoder"]["num_b"] = oled_user_interface_settings.encoder_b_pin;
            pins["buzzer"] = pack_do_settings(oled_user_interface_settings.buzzer);

            obj["display_width"] = oled_user_interface_settings.display_width;
            obj["display_height"] = oled_user_interface_settings.display_height;

        }
        //voltmeter
        {
            JsonObject obj = settings["voltmeter"];

            obj["voltmeter_read_pin"]["num"] = voltmeter_settings.voltmeter_read_pin;
            obj["adc_value_ref_1"] = voltmeter_settings.adc_value_ref_1;
            obj["voltage_ref_1"] = voltmeter_settings.voltage_ref_1;
            obj["adc_value_ref_2"] = voltmeter_settings.adc_value_ref_2;
            obj["voltage_ref_2"] = voltmeter_settings.voltage_ref_2;

            obj["batt_full_charge"] = voltmeter_settings.batt_full_charge;
            obj["batt_empty_charge"] = voltmeter_settings.batt_empty_charge;

        }
        //ir_detector
        {
            JsonObject obj = settings["ir_detector"];
            JsonObject pins = obj["pins"];

            pins["ir_reciever_power"] = pack_do_settings(ir_detector_settings.ir_reciever_power);
            pins["ir_emitter_power"] = pack_do_settings(ir_detector_settings.ir_emitter_power);
            pins["ir_reciever_read_pin"]["num"] = ir_detector_settings.ir_reciever_read_pin;
            pins["mag_release"] = pack_di_settings(ir_detector_settings.mag_release);
            
            obj["adc_falling_threshhold"] = ir_detector_settings.adc_falling_threshhold;
            obj["adc_rising_threshhold"] = ir_detector_settings.adc_rising_threshhold;
            obj["dart_length_mm"] = ir_detector_settings.dart_length_mm;

        }
        //pusher
        {
            JsonObject obj = settings["pusher"];
            obj["pins"]["fet"] = pack_do_settings(pusher_settings.fet);
            obj["min_extend_time_ms"] = pusher_settings.min_extend_time_ms;
            obj["min_retract_time_ms"] = pusher_settings.min_retract_time_ms;
            obj["max_extend_time_ms"] = pusher_settings.max_extend_time_ms;

        }
        //flywheels
        {
            JsonObject obj = settings["flywheels"];

            obj["pins"]["motor_l_pin"] = flywheel_settings.motor_l_pin;
            obj["pins"]["motor_r_pin"] = flywheel_settings.motor_r_pin;
            obj["dshot_mode"] = flywheel_settings.dshot_mode;
            obj["downthrottle_time_ms"] = flywheel_settings.downthrottle_time_ms;
            obj["idle_rpm"] = flywheel_settings.idle_rpm;
            obj["idle_time_ms"] = flywheel_settings.idle_time_ms;
            obj["brushless_motor_kv"] = flywheel_settings.brushless_motor_kv;
            obj["average_battery_voltage"] = flywheel_settings.average_battery_voltage;

        }
        //handle
        {
            JsonObject obj = settings["handle"];

            obj["shoot_trigger"] = pack_di_settings(handle_settings.shoot_trigger);
            obj["rev_trigger"] = pack_di_settings(handle_settings.rev_trigger);


        }

    }
    //pack presets
    {
        JsonObject obj = settings["variables"];

        JsonArray preset_array = obj["presets"];

        for(int i = 0; i < preset_list.size(); ++i) {

            //make document
            JsonDocument preset_settings;
            preset_settings["name"] = preset_list[i].name;

            //de-enumify mode typedefs
            preset_settings["trigger_mode"] = (size_t)preset_list[i].trigger_mode;
            preset_settings["shoot_mode"] = (size_t)preset_list[i].shoot_mode;
            preset_settings["burst_count"] = preset_list[i].burst_count;
            preset_settings["cache_delay_ms"] = preset_list[i].cache_delay_ms;
            preset_settings["push_rate_pct"] = preset_list[i].push_rate_pct;
            preset_settings["flywheel_rpm"] = preset_list[i].flywheel_rpm;

            preset_array.add(preset_settings);

        }

    }


    return settings;

}



//private helpers:

DigitalInputSettings Settings::unpack_di_settings(JsonObject pin_settings) {
    DigitalInputSettings settings;
    settings.pin = pin_settings["num"];
    settings.normally_closed = pin_settings["nc"];
    settings.pullup = pin_settings["pullup"];
    settings.debounce_time = pin_settings["debounce_time"];
    return settings;
}

DigitalOutputSettings Settings::unpack_do_settings(JsonObject pin_settings) {
    DigitalOutputSettings settings;
    settings.pin = pin_settings["num"];
    settings.on_high = pin_settings["on_high"];
    return settings;
}

JsonDocument pack_di_settings(DigitalInputSettings pin_settings) {
    JsonDocument settings;

    settings["num"] = pin_settings.pin;
    settings["nc"] = pin_settings.normally_closed;
    settings["pullup"] = pin_settings.pullup;
    settings["debounce_time"] = pin_settings.debounce_time;

    return settings;
}

JsonDocument pack_do_settings(DigitalOutputSettings pin_settings) {
    JsonDocument settings;

    settings["num"] = pin_settings.pin;
    settings["on_high"] = pin_settings.on_high;

    return settings;
}



