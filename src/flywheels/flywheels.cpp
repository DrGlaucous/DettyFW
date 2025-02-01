#include <Arduino.h>
#include <DShotRMT.h>

#include "../constants/constants.h"
#include "../settings/settings.h"
#include "../state/state.h"
#include "flywheels.h"


namespace Flywheels {



    //not touched by the ISR
    bool initialized = false;
    bool dummy_mode = true;
    hw_timer_t* timer = nullptr;

    //set from outside the ISR, must be treated as readonly inside
    uint32_t motor_tx_delay_micros = 0;
    uint32_t target_rpm = 0;
    bool rpm_override = false;
    uint32_t override_throttle = DSHOT_THROTTLE_MIN;

    uint32_t last_pid_update_time = 0;

    //set from inside the ISR, must be treated as readonly outside
    Wheel wheel_r = {};
    Wheel wheel_l = {};

    //test, see how long it takes to process "update_pid"
    uint32_t process_time = 0;

    /// tell wheels to send out a dshot packet
    IRAM_ATTR void tick_from_isr() {
        wheel_r.send_dshot_packet();
        wheel_l.send_dshot_packet();
    }

    void update_pid() {

        //max time of execution here seems to be around 71 microseconds
        //too heavy to be called from ISR (or is it just because we use floats in the PID?)


        //calculate the time spent outside this function
        uint32_t delta_time = micros() - last_pid_update_time;
        last_pid_update_time = delta_time;

        uint32_t start  = micros();
        
        wheel_r.tick_pid(
            target_rpm,
            rpm_override,
            override_throttle
        );
        wheel_l.tick_pid(
            target_rpm,
            rpm_override,
            override_throttle
        );
        
        //halt dshot output timer while we re-calculate the PID values it should use
        timerStop(timer);
        wheel_r.prepare_dshot_packet();
        wheel_l.prepare_dshot_packet();
        timerStart(timer);


        //for heat testing
        process_time = micros() - start;

    }

    void init(const Settings& settings) {
        auto flywheel_settings = settings.get_flywheel_settings_ref();

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

        //determine if the settings are valid
        if(mode == DSHOT_OFF
        || flywheel_settings.motor_l_pin == -1
        || flywheel_settings.motor_r_pin == -1) {
            dummy_mode = true;
            return;
        } else {
            dummy_mode = false;
        }



        //todo: read in these settings from the controller JSON

        PIDController motor_l_controller = PIDController(
            DSHOT_THROTTLE_MIN, //start output
            DSHOT_THROTTLE_MAX, //min output lim
            DSHOT_THROTTLE_MIN, //max output lim
            0.0012, //p
            0.00015, //i
            0.001 //d
        );
        uint32_t l_last_micros_got = 0;

        PIDController motor_r_controller = PIDController(
            DSHOT_THROTTLE_MIN, //start output
            DSHOT_THROTTLE_MAX, //min output lim
            DSHOT_THROTTLE_MIN, //max output lim
            0.0012, //p
            0.00015, //i
            0.001 //d
        );
        uint32_t r_last_micros_got = 0;

        //set these to start wide open as soon as the PID starts up (function limits to max output limit)
        motor_l_controller.set_output_offset(0);
        motor_r_controller.set_output_offset(0);

        wheel_r.init(
            motor_r_controller,
            flywheel_settings.motor_r_pin,
            mode,
            ENABLE_BIDIRECTION,
            flywheel_settings.motor_r_pole_ct
        );

        wheel_l.init(
            motor_l_controller,
            flywheel_settings.motor_l_pin,
            mode,
            ENABLE_BIDIRECTION,
            flywheel_settings.motor_l_pole_ct
        );

        timer = timerBegin(ISR_TIMER_FLYWHEEL, 80, true); //microsecond resolution timer
        timerAttachInterrupt(timer, &tick_from_isr, true);
        timerAlarmWrite(timer, 300, true); // every 0.01 seconds //1000000 = 1
        timerAlarmEnable(timer);

        initialized = true;

    }

    void deinit() {
        //disable ISR
        timerDetachInterrupt(timer);
        timerEnd(timer);      

        //reset flywheel classes and detach RMT LL
        wheel_r = Wheel();
        wheel_l = Wheel();
        
        initialized = false; 
    }

    //tell the PID to maintain this RPM
    void set_target_rpm(uint32_t rpm) {
        rpm_override = false;
        target_rpm = rpm;
    }

    //tell the PID to output this throttle directly
    void set_throttle_override(uint32_t throttle) {
        rpm_override = true;
        if(throttle > DSHOT_THROTTLE_MAX) {
            throttle = DSHOT_THROTTLE_MAX;
        } else if(throttle < DSHOT_THROTTLE_MIN) {
            throttle = DSHOT_THROTTLE_MIN;
        }
        override_throttle = throttle;
    }

    uint32_t get_wheel_l_rpm() {
        return wheel_l.get_rpm();
    }
    uint32_t get_wheel_r_rpm() {
        return wheel_r.get_rpm();
    }

}


/*
//load in settings from State settings
Flywheels::Flywheels(Settings& settings) {

    auto flywheel_settings = settings.get_flywheel_settings_ref();

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

    //set up PID Controllers
    {
        //todo: read in these settings from the controller JSON

        motor_l_controller = PIDController(
            DSHOT_THROTTLE_MIN, //start output
            DSHOT_THROTTLE_MAX, //min output lim
            DSHOT_THROTTLE_MIN, //max output lim
            0.0012, //p
            0.000, //i
            0.006 //d
	    );
	    uint32_t l_last_micros_got = 0;

        motor_r_controller = PIDController(
            DSHOT_THROTTLE_MIN, //start output
            DSHOT_THROTTLE_MAX, //min output lim
            DSHOT_THROTTLE_MIN, //max output lim
            0.0012, //p
            0.000, //i
            0.006 //d
        );
        uint32_t r_last_micros_got = 0;

        //set these to start wide open as soon as the PID starts up (function limits to max output limit)
        motor_l_controller.set_output_offset(9999);
        motor_r_controller.set_output_offset(9999);

    }

    //set up motor drivers
    {
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
    }


    //set up tick ISR
    {
        timer = timerBegin(0, 80, true); //microsecond resolution timer
        timerAttachInterrupt(timer, &(this->tick), true);
        timerAlarmWrite(timer, 200, true); // every 0.01 seconds
        timerAlarmEnable(timer);
    }

};

Flywheels::~Flywheels() {
    //note: these are probably broken
    delete(motor_l);
    delete(motor_r);
}

void Flywheels::tick() {

    //do not run if incorrectly initialized
    if(dummy_mode)
        return;

    //should I use state or micros() for this?
    auto abs_micros = micros();
    
    //check every tick for updated rx values and update PID as often as possible
    {
        uint32_t l_rpm;
        uint32_t r_rpm;

        //write directly to the state flywheel speed (values are unchanged if no good packet is gotten)
        dshot_get_packet_exit_mode_t l_response = motor_l->get_dshot_packet(&l_rpm);
        dshot_get_packet_exit_mode_t r_response = motor_r->get_dshot_packet(&r_rpm);

        flywheel_l_current_rpm = l_rpm != 0 ? l_rpm : flywheel_l_current_rpm;
        flywheel_r_current_rpm = r_rpm != 0 ? r_rpm : flywheel_r_current_rpm;

        //note: get_dshot_packet gets the latest reponse ONLY, regardless of success or not (we need to do success handeling in a transmit ISR or something)

        //raw disable (don't use PID, set motor output directly to 0)
        if(!flywheel_rpm_active) {

            //decelerate
            if(motor_l_raw_trottle > 100) {
                motor_l_raw_trottle -= 2;
            } else {
                motor_l_raw_trottle = 0;
            }
            if(motor_r_raw_trottle > 100) {
                motor_r_raw_trottle -= 2;
            } else {
                motor_r_raw_trottle = 0;
            }

            //put PID controllers in reset state
            motor_l_controller.reset();
            motor_r_controller.reset();

        } else {
            //update PID if response is good
            if(l_response == DECODE_SUCCESS) {
                motor_l_raw_trottle = motor_l_controller.tick(flywheel_target_rpm, flywheel_l_current_rpm, abs_micros - l_last_micros_got);
                l_last_micros_got = abs_micros;
            }
            if(r_response == DECODE_SUCCESS) {
                motor_r_raw_trottle = motor_r_controller.tick(flywheel_target_rpm, flywheel_r_current_rpm, abs_micros - r_last_micros_got);
                r_last_micros_got = abs_micros;
            }
        }

        //TEST STUFF
        // Serial.printf("run\n");
        // delay(100);
        // int no = motor_l_controller->test(state.flywheel_target_rpm,0,0);
        // int no = motor_l_controller->tick(state.flywheel_target_rpm, 0, 0);
        // no += state.flywheel_target_rpm;
        // Serial.printf("run %d, %d, %d\n", state.flywheel_target_rpm, state.flywheel_l_current_rpm, abs_micros - l_last_micros_got);
    
    }
    
    //motor_l_raw_trottle = state.flywheel_target_rpm;
    //motor_r_raw_trottle = state.flywheel_target_rpm;

    //only send new packets out on this fixed interval (interval set in constructor)
    //use subtraction here to deal with overflows
    //if (abs_micros - motor_tx_delay_micros > last_micros_sent) {

    //calling from ISR, at fixed rate
    if(true) {
        motor_l->send_dshot_value(motor_l_raw_trottle);
        motor_r->send_dshot_value(motor_r_raw_trottle);
        last_micros_sent = abs_micros;
    }

}
*/