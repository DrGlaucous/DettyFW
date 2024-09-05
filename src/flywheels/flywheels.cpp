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
            motor_tx_delay_micros = 300;
            mode = DSHOT300;
            break;
        case Dshot600:
            motor_tx_delay_micros = 200;
            mode = DSHOT600;
            break;
        case Dshot1200:
            motor_tx_delay_micros = 100;
            mode = DSHOT1200;
            break;
        default:
            motor_tx_delay_micros = 0;
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

    uint16_t motor_l_raw_trottle = 0;
    uint16_t motor_r_raw_trottle = 0;

    //check every tick for updated rx values and update PID as often as possible
    {
        uint32_t l_rpm;
        uint32_t r_rpm;

        dshot_get_packet_exit_mode_t l_response = motor_l->get_dshot_packet(&l_rpm);
        dshot_get_packet_exit_mode_t r_response = motor_r->get_dshot_packet(&r_rpm);

        //update PID if response is good
        if(l_response == DECODE_SUCCESS) {
            motor_l_raw_trottle = motor_l_controller.tick(state.flywheel_target_rpm, l_rpm, delta_t);
            state.flywheel_l_current_rpm = l_rpm; //report back values to state
        }
        if(r_response == DECODE_SUCCESS) {
            motor_l_raw_trottle = motor_l_controller.tick(state.flywheel_target_rpm, l_rpm, delta_t);
            state.flywheel_r_current_rpm = r_rpm;
        }
    }

    //only send new packets out on this fixed interval (set in constructor)
    if (delta_t > motor_tx_delay_micros) {
        motor_l->send_dshot_value(motor_l_raw_trottle);
        motor_r->send_dshot_value(motor_r_raw_trottle);
    }

}














