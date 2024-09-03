#include <Arduino.h>
#include <DShotRMT.h>


#include "../settings/settings.h"
#include "../state/state.h"
#include "flywheels.h"


Flywheels::Flywheels(State& state) {

    auto flywheel_settings = state.settings.get_flywheel_settings_ref();

    dshot_mode_t mode;
    switch(flywheel_settings.dshot_mode) {
        case Dshot300:
            mode = DSHOT300;
            break;
        case Dshot600:
            mode = DSHOT600;
            break;
        case Dshot1200:
            mode = DSHOT1200;
            break;
        default:
            mode = DSHOT_OFF;
            break;
    }

    //run in "dummy" mode if any of these conditions are true
    if(mode == DSHOT_OFF
    || flywheel_settings.motor_l_pin == -1
    || flywheel_settings.motor_r_pin == -1) {
        disabled_mode = true;
        return;
    }
    disabled_mode = false;

    motor_l = new DShotRMT(flywheel_settings.motor_l_pin);
    motor_r = new DShotRMT(flywheel_settings.motor_r_pin);



    //todo: motorPoleCount==0 needs to be caught in the dshotRMT function itself, not here
    motor_l->begin(
        mode,
        ENABLE_BIDIRECTION,
        flywheel_settings.motor_l_pole_ct == 0 ? 1 : flywheel_settings.motor_l_pole_ct
    );
    motor_r->begin(
        mode,
        ENABLE_BIDIRECTION,
        flywheel_settings.motor_r_pole_ct == 0 ? 1 : flywheel_settings.motor_r_pole_ct
    );

};

Flywheels::~Flywheels() {
    delete(motor_l);
    delete(motor_r);
}

void Flywheels::tick(State& state) {

    auto delta_t = state.get_delta_micros();
    //todo: configure this value
    //good timeout values:
	//dshot300: 300micros
	//dshot600: 150micros
    if (delta_t > 300) {

        uint32_t l_rpm;
        uint32_t r_rpm;

        dshot_get_packet_exit_mode_t l_response = motor_l->get_dshot_packet(&l_rpm);
        dshot_get_packet_exit_mode_t r_response = motor_r->get_dshot_packet(&r_rpm);

        
    }

}














