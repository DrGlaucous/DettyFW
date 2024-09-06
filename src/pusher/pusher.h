#pragma once

#include <Arduino.h>


#include "../settings/settings.h"
#include "../state/state.h"


class Pusher {

    public:

    Pusher();
    ~Pusher();

    void tick(State& state) {

    }



    private:

    uint32_t burst_count = 0; //how many times we need to push yet

    //for the session, how long (ms) the solenoid should be in these states
    uint32_t curr_extend_time = 0;
    uint32_t curr_retract_time = 0;


    int32_t countdown_millis = 0; //used to count down the delay for each push in/out cycle (may be negative!)
    bool solenoid_state = false;//the state of the solenoid (true == ON)

    uint32_t last_millis = 0; //for changing only 1x per tick


};
