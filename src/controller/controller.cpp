#include <vector>
#include <string>
#include <Bounce2.h>
#include <Encoder.h>

#include "../constants/constants.h"
#include "../settings/settings.h"
#include "controller.h"


void Controller::init(Settings& settings) {

    //already initialized, don't try again
    if(rotary_encoder) {
        return;
    }

    auto handle = settings.get_handle_settings_ref();
    auto oled = settings.get_oled_interface_settings_ref();
    
    if(handle.shoot_trigger.pin != -1) {
        trigger.attach(handle.shoot_trigger.pin, handle.shoot_trigger.pullup? INPUT_PULLUP : INPUT_PULLDOWN);
        trigger.interval(handle.shoot_trigger.debounce_time);
    }

    if(oled.preset_a.pin != -1) {
        preset_a.attach(oled.preset_a.pin, oled.preset_a.pullup? INPUT_PULLUP : INPUT_PULLDOWN);
        preset_a.interval(oled.preset_a.debounce_time);
    }
    if(oled.preset_b.pin != -1) {
        preset_b.attach(oled.preset_b.pin, oled.preset_b.pullup? INPUT_PULLUP : INPUT_PULLDOWN);
        preset_b.interval(oled.preset_b.debounce_time);
    }
    if(oled.preset_c.pin != -1) {
        preset_c.attach(oled.preset_c.pin, oled.preset_c.pullup? INPUT_PULLUP : INPUT_PULLDOWN);
        preset_c.interval(oled.preset_c.debounce_time);
    }

    rotary_encoder = new Encoder(oled.encoder_a_pin, oled.encoder_b_pin);

    // timer = timerBegin(ISR_TIMER_POT, 80, true); //use timer 0 with x80 divider
    // timerAttachInterrupt(timer, Controller::update_encoder, true);
    // timerAlarmWrite(timer, 100, true); // every 0.001 seconds
    // timerAlarmEnable(timer);

    pinMode(oled.encoder_a_pin, INPUT_PULLUP);
    pinMode(oled.encoder_b_pin, INPUT_PULLUP);
    attachInterrupt(oled.encoder_a_pin, Controller::update_encoder, CHANGE);
    attachInterrupt(oled.encoder_b_pin, Controller::update_encoder, CHANGE);

    encoder_a.attach(oled.encoder_a_pin, INPUT_PULLUP);
    encoder_a.interval(1);
    encoder_b.attach(oled.encoder_b_pin, INPUT_PULLUP);
    encoder_b.interval(1);

}


void Controller::deinit() {
    //timerDetachInterrupt(timer);
    //timerEnd(timer);
}


IRAM_ATTR void Controller::update_encoder() {
    //this function should never be executed outside the ITTR
    //encoder_value = rotary_encoder->read();

    encoder_a.update();
    encoder_b.update();

    static uint8_t state = 0;
    bool CLKstate = encoder_a.read();
    bool DTstate = encoder_b.read();

    switch (state) {
        case 0:                         // Idle state, encoder not turning
            if (!CLKstate){             // Turn clockwise and CLK goes low first
                state = 1;
            } else if (!DTstate) {      // Turn anticlockwise and DT goes low first
                state = 4;
            }
            break;
        // Clockwise rotation
        case 1:                     
            if (!DTstate) {             // Continue clockwise and DT will go low after CLK
                state = 2;
            } 
            break;
        case 2:
            if (CLKstate) {             // Turn further and CLK will go high first
                state = 3;
            }
            break;
        case 3:
            if (CLKstate && DTstate) {  // Both CLK and DT now high as the encoder completes one step clockwise
                state = 0;
                ++encoder_value;
            }
            break;
        // Anticlockwise rotation
        case 4:                         // As for clockwise but with CLK and DT reversed
            if (!CLKstate) {
                state = 5;
            }
            break;
        case 5:
            if (DTstate) {
                state = 6;
            }
            break;
        case 6:
            if (CLKstate && DTstate) {
                state = 0;
                --encoder_value;
            }
            break; 
    }
}


// typically, the only thread that should be reading input should be the thread calling this tick function,
// packaged conclusions should be send to other threads as-needed (like the display)
void Controller::tick(Settings& settings) {

    auto handle = settings.get_handle_settings_ref();
    auto oled = settings.get_oled_interface_settings_ref();


    trigger.update();
    preset_a.update();
    preset_b.update();
    preset_c.update();

    //todo: better define the trigger conditions in the JSON
    //trigger state currently assumes a reading of HIGH means the trigger is NOT closed

    //nc - LOW -> OFF
    //nc - HIGH -> ON
    //no - LOW -> ON
    //no - HIGH -> OFF

    bool n_trigger_state = trigger.read() == handle.shoot_trigger.normally_closed;
    bool n_preset_a_state = preset_a.read() == oled.preset_a.normally_closed;
    bool n_preset_b_state = preset_b.read() == oled.preset_b.normally_closed;
    bool n_preset_c_state = preset_c.read() == oled.preset_c.normally_closed;

    //capture changes
    trigger_changed = n_trigger_state != trigger_state;
    preset_a_changed = n_preset_a_state != preset_a_state;
    preset_b_changed = n_preset_b_state != preset_b_state;
    preset_c_changed = n_preset_c_state != preset_c_state;

    //update old values
    trigger_state = n_trigger_state;
    preset_a_state = n_preset_a_state;
    preset_b_state = n_preset_b_state;
    preset_c_state = n_preset_c_state;

    //update encoder layer
    delta_encoder_val = encoder_value - last_encoder_val;
    last_encoder_val = encoder_value;

    //Serial.printf("aa: %d, bb: %d\n", aa, bb);

}


hw_timer_t* Controller::timer = NULL;
Encoder* Controller::rotary_encoder = NULL;

Bounce Controller::trigger;
bool Controller::trigger_state = false;
bool Controller::trigger_changed = false;

Bounce Controller::preset_a;
bool Controller::preset_a_state = false;
bool Controller::preset_a_changed = false;

Bounce Controller::preset_b;
bool Controller::preset_b_state = false;
bool Controller::preset_b_changed = false;

Bounce Controller::preset_c;
bool Controller::preset_c_state = false;
bool Controller::preset_c_changed = false;


int32_t Controller::encoder_value = 0;
int32_t Controller::delta_encoder_val = 0;
int32_t Controller::last_encoder_val = 0;

Bounce Controller::encoder_a;
Bounce Controller::encoder_b;

























