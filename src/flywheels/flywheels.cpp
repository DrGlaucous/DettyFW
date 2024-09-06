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
        dummy_mode = true;
        return;
    }
    dummy_mode = false;

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

    //set these to start wide open as soon as the PID starts up (function limits to max output limit)
    motor_l_controller.set_output_offset(9999);
    motor_r_controller.set_output_offset(9999);


};

Flywheels::~Flywheels() {
    //note: these are probably broken
    delete(motor_l);
    delete(motor_r);
}

void Flywheels::tick(State& state) {

    //do not run if incorrectly initialized
    if(dummy_mode)
        return;

    //should I use state or micros() for this?
    auto abs_micros = state.get_abs_micros();

    uint16_t motor_l_raw_trottle = 0;
    uint16_t motor_r_raw_trottle = 0;

    //check every tick for updated rx values and update PID as often as possible
    {
        //uint32_t l_rpm;
        //uint32_t r_rpm;

        //write directly to the state flywheel speed (values are unchanged if no good packet is gotten)
        dshot_get_packet_exit_mode_t l_response = motor_l->get_dshot_packet(&state.flywheel_l_current_rpm);
        dshot_get_packet_exit_mode_t r_response = motor_r->get_dshot_packet(&state.flywheel_r_current_rpm);

        //raw disable (don't use PID, set motor output directly to 0)
        if(!state.flywheel_rpm_active) {
            motor_l_raw_trottle = 0;
            motor_r_raw_trottle = 0;

            //put PID controllers in reset state
            motor_l_controller.reset();
            motor_r_controller.reset();

        } else {
            //update PID if response is good
            if(l_response == DECODE_SUCCESS) {
                motor_l_raw_trottle = motor_l_controller.tick(state.flywheel_target_rpm, state.flywheel_l_current_rpm, abs_micros - l_last_micros_got);
                l_last_micros_got = abs_micros;
            }
            if(r_response == DECODE_SUCCESS) {
                motor_r_raw_trottle = motor_r_controller.tick(state.flywheel_target_rpm, state.flywheel_r_current_rpm, abs_micros - r_last_micros_got);
                r_last_micros_got = abs_micros;
            }
        }


    }

    //only send new packets out on this fixed interval (interval set in constructor)
    //use subtraction here to deal with overflows
    if (abs_micros - motor_tx_delay_micros > last_micros_sent) {
        motor_l->send_dshot_value(motor_l_raw_trottle);
        motor_r->send_dshot_value(motor_r_raw_trottle);
        last_micros_sent = abs_micros;
    }

}














