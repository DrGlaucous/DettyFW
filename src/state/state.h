#pragma once

#include <Arduino.h>

#include "../settings/settings.h"
#include "../filesystem/filesystem.h"

#define SETTINGS_DIRECTORY "/settings/settings.json"


class State {


    public:


    uint32_t flywheel_target_rpm;
    uint32_t flywheel_current_rpm;

    uint32_t push_count;


    Settings settings = Settings(SETTINGS_DIRECTORY);

    //create new state and load in settings
    State() {}

    ~State() {
    }

    void init() {
        filesystem::initialize();
        
        //settings initialized in constructor
        settings.load();
        
    }

    void update_time(uint32_t micros) {
        //no need for overflow protection; it works just the same
        tick_time = micros - last_micros;
        last_micros = micros;
    }

    inline uint32_t get_delta_micros() {
        return tick_time;
    }
    inline uint32_t get_delta_millis() {
        return tick_time / 1000;
    }



    private:

    //timestamp at last update_time
    uint32_t last_micros = 0;

    //time since last cycle
    uint32_t tick_time = 0;



};





