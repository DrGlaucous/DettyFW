#pragma once

#include <Arduino.h>
#include <DShotRMT.h>


#include "../settings/settings.h"


class Flywheels {

    public:


    Flywheels();
    ~Flywheels();


    void init();

    //all inputs are passed in through the "state"
    void tick();





};


