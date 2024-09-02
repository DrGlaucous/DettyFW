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

    DeserializationError err = deserializeJson(settings_json, filebuf, filelen);

    if(err.code()) {
        auto code = err.code();
        size_t apple = code + 6;
        size_t banana = apple + 2;
        Serial.printf("parse error: %d\n", code);
    }

    serializeJsonPretty(settings_json, Serial);


    //finished decoding, free filebuffer
    free(filebuf);

    //parse out settings
    unpack_json(settings_json);

    PresetSettings a = preset_list[0];
    PresetSettings b = preset_list[1];

    Serial.printf("\nsize: %d\n", preset_list.size());
    Serial.printf("name: %s\n", a.name.c_str());


    //todo: make this meaningful
    return false;

}
bool Settings::save() {

    //pack settings
    JsonDocument settings_json = pack_json();

    //calculate the size of the json + null terminator so we can allocate a buffer to fit it
    size_t json_size = measureJsonPretty(settings_json) + 1;
    uint8_t* json_buffer = (uint8_t*)malloc(json_size);
    //write to buffer
    serializeJsonPretty(settings_json, json_buffer, json_size);

    //filesystem::writeFile(directory.c_str(), json_buffer, json_size);

    //finished writing, free filebuffer
    free(json_buffer);

    //todo: make this meaningful
    return false;

}

//reads the json at index and returns 
void Settings::unpack_json(JsonDocument& settings_json) {

    //unpack other
    {
        JsonObject constants = settings_json["constants"];
        //debug
        debug_settings.unpack(constants["debug"]);
        //oled
        oled_user_interface_settings.unpack(constants["oled_user_interface"]);
        //voltmeter
        voltmeter_settings.unpack(constants["voltmeter"]);
        //ir_detector
        ir_detector_settings.unpack(constants["ir_detector"]);
        //pusher
        pusher_settings.unpack(constants["pusher"]);
        //flywheels
        flywheel_settings.unpack(constants["flywheels"]);
        //handle
        handle_settings.unpack(constants["handle"]);

    }
    
    
    //unpack presets
    {

        JsonArray arr = settings_json["variables"]["presets"];
        size_t list_len = arr.size();

        //pre-allocate correct-sized vector (will delete old vector)
        preset_list = vector<PresetSettings>(list_len);

        for(int i = 0; i < list_len; ++i) {

            //get the object from our master list
            //JsonObject output = arr[i]; //.as<JsonObject>();
            preset_list[i].unpack(arr[i]);// = preset;
        }


    }
 
}

JsonDocument Settings::pack_json() {

    JsonDocument settings;

    //pack other
    {
        JsonDocument constants;// = settings["constants"];

        //debug
        constants["debug"] = debug_settings.pack();
        //oled
        constants["oled_user_interface"] = oled_user_interface_settings.pack();
        //voltmeter
        constants["voltmeter"] = voltmeter_settings.pack();
        //ir_detector
        constants["ir_detector"] = ir_detector_settings.pack();
        //pusher
        constants["pusher"] = pusher_settings.pack();
        //flywheels
        constants["flywheels"] = flywheel_settings.pack();
        //handle
        constants["handle"] = handle_settings.pack();

        settings["constants"] = constants;
    }
    //pack presets
    {
        //JsonObject obj = settings["variables"];
        JsonDocument obj;

        JsonArray preset_array = obj["presets"].to<JsonArray>();

        for(int i = 0; i < preset_list.size(); ++i) {
            preset_array.add(preset_list[i].pack());
        }

        settings["variables"] = obj;
    }

    Serial.printf("\nRe-package:\n");


    // JsonArray data = settings["arp"].to<JsonArray>();
    // data.add(48.75608);
    // data.add(2.302038);

    serializeJsonPretty(settings, Serial);

    return settings;

}



//private helpers:
/*
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

JsonDocument Settings::pack_di_settings(DigitalInputSettings pin_settings) {
    JsonDocument settings;

    settings["num"] = pin_settings.pin;
    settings["nc"] = pin_settings.normally_closed;
    settings["pullup"] = pin_settings.pullup;
    settings["debounce_time"] = pin_settings.debounce_time;

    return settings;
}

JsonDocument Settings::pack_do_settings(DigitalOutputSettings pin_settings) {
    JsonDocument settings;

    settings["num"] = pin_settings.pin;
    settings["on_high"] = pin_settings.on_high;

    return settings;
}

*/

