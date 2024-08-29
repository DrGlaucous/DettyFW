/*
Handles generic user-acutated inputs (things like buttons and such...)
*/

#include <Arduino.h>
#include <Bounce2.h>

#include "main.h"
#include "input.h"
#include "configuration.h"

#include <Encoder.h>



inputHandler::inputHandler(void)
{
    //protect against multiple inits
    if(hasBeenInitialzed)
        return;

    hasBeenInitialzed = true;

    //attach debounching to the corresponding GPIO
    {
        triggerSwitch.attach(TRIGGER_PIN, INPUT_PULLUP);
        triggerSwitch.interval(DEBOUNCE_TIME);
        triggerSwitch.setPressedState(TRIGGER_NC);


        magReleaseSwitch.attach(MAG_TRIG_PIN, INPUT_PULLUP);
        magReleaseSwitch.interval(DEBOUNCE_TIME);
        magReleaseSwitch.setPressedState(TRIGGER_NC);

    #ifdef USE_REV_TRIG
            revTriggerSwitch.attach(REV_TRIG_PIN, INPUT_PULLUP);
            revTriggerSwitch.interval(DEBOUNCE_TIME);
            revTriggerSwitch.setPressedState(TRIGGER_NC);
    #endif

        presetA.attach(PRESET_A_PIN, INPUT_PULLUP);
        presetA.interval(DEBOUNCE_TIME);
        presetA.setPressedState(TRIGGER_NC);

        presetB.attach(PRESET_B_PIN, INPUT_PULLUP);
        presetB.interval(DEBOUNCE_TIME);
        presetB.setPressedState(TRIGGER_NC);

        presetC.attach(PRESET_C_PIN, INPUT_PULLUP);
        presetC.interval(DEBOUNCE_TIME);
        presetC.setPressedState(TRIGGER_NC);


        //encoderA.attach(ENCODER_A, INPUT_PULLUP);
        //encoderA.interval(DEBOUNCE_TIME);
        //encoderA.setPressedState(TRIGGER_NC);

        //encoderB.attach(ENCODER_B, INPUT_PULLUP);
        //encoderB.interval(DEBOUNCE_TIME);
        //encoderB.setPressedState(TRIGGER_NC);

        encoderSwitch.attach(ENCODER_BUTTON_PIN, INPUT_PULLUP);
        encoderSwitch.interval(DEBOUNCE_TIME);
        encoderSwitch.setPressedState(TRIGGER_NC);
    }

    //nothing to do here for the encoder: everything's been constructed already

    //analogRead voltmeter
    pinMode(VOLTMETER_PIN, OUTPUT);
}

inputHandler::~inputHandler()
{
    //destroy all button debouce objects
    {
        delete(&triggerSwitch);
        delete(&magReleaseSwitch);
#ifdef USE_REV_TRIG
        delete(&revTriggerSwitch);
#endif
        delete(&presetA);
        delete(&presetB);
        delete(&presetC);
        //delete(&encoderA);
        //delete(&encoderB);
        delete(&encoderSwitch);

    }
}

void inputHandler::update(void)
{

    //check for all inputs

    //update the bounce2 backend
    {
        triggerSwitch.update();
        magReleaseSwitch.update();
#ifdef USE_REV_TRIG
        revTriggerSwitch.update();
#endif
        presetA.update();
        presetB.update();
        presetC.update();
        //encoderA.update();
        //encoderB.update();
        encoderSwitch.update();

    }

    //last_encoder_val = encoder_val; //not needed if we read and reset, but needed if we don't

    //zero the encoder
    delta_encoder_val = encoder_val/4 - last_encoder_val;
    last_encoder_val = encoder_val / 4;

    //read the raw value from the ADC
    voltmeter_read = analogRead(VOLTMETER_PIN);

}

void inputHandler::update_encoder(void)
{
    //keeps track of all updates, zeros out every time the normal update function is called, like the rest of the inputs
    encoder_val = rotaryEncoder.read();
}


