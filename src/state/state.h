#pragma once

#include <Arduino.h>

#include "../settings/settings.h"
#include "../filesystem/filesystem.h"

#define SETTINGS_DIRECTORY "/settings/settings.json"


class State {


    public:

    //flywheel control variables
    uint32_t flywheel_target_rpm;
    bool flywheel_rpm_active; //true if we want to spin up, false if we're spinning down
    //telem values
    uint32_t flywheel_l_current_rpm;
    uint32_t flywheel_r_current_rpm;

    //solenoid control variables
    uint32_t push_count;
    uint8_t push_rate_pct;


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

    inline uint32_t get_abs_micros() {
        return last_micros;
    }
    inline uint32_t get_abs_millis() {
        return last_micros / 1000;
    }



    private:

    //timestamp at last update_time
    uint32_t last_micros = 0;

    //time since last cycle
    uint32_t tick_time = 0;



};





