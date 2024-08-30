#pragma once


#include <Arduino.h>

#include "main.h"
#include "configuration.h"


//there are 2 main types of firing operations: full auto and select fire
//there are multiple attributes that can be added to the select fire modes, such as:
//cache fire (firing window)
//commit fire (trigger must be held until flywheels are at speed)



class connectomeHandler
{
    public:

        connectomeHandler();
        void update();



    private:

        void triggerTypeHandler(bool release = false);



        unsigned int press_start = 0; //when the trigger is first pressed millis()
        int cache_burst_total = 0; //how many rounds to fire in cache mode

        bool cache_in_progress = false; //is a cache fire session in progress?
        bool push_in_progress = false; //set to true when we send the pusher command

        //(this is a botch)
        bool cycle_completed = false; //set if the push cycle was complete and the user needs to let off the trigger to start a new one

        bool connectome_trigger = false; //what the connectome uses as a secondary trigger
        bool connectome_trigger_changed = false; //same as .pressed()



};
