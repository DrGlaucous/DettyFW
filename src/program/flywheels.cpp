
#include <Arduino.h>
#include <ESP32Servo.h>
#include <DShotRMT.h>
#include "main.h"
#include "configuration.h"

#include "flywheels.h"


void singleMotor::attach(int pinNum,
                        unsigned int delayMillis,
                        flywheel_type_t flywheelType,
                        dshot_mode_t dshotMode,
                        bidirectional_mode_t dshotBidir,
                        int* outMin, int* outMax,
                        int* rpmMin, int* rpmMax
                        )
{
    //prevent running this more than 1x
    if(has_been_attached)
        return;
    has_been_attached = true;

    //save settings to the class
    pin_num = pinNum;
    delay_millis = delayMillis;
    flywheel_type = flywheelType;
    dshot_mode = dshotMode;
    dshot_bidir = NO_BIDIRECTION;

    output_min = outMin; //TODO: change these to refer to global settings
    output_max = outMax;
    rpm_min = rpmMin;
    rpm_max = rpmMax;

    //set up the output pin
    switch(flywheel_type)
    {
        default:
        case FLYWHEEL_TYPE_NULL:
            //nothing to initialize
            break;
        case FLYWHEEL_TYPE_BRUSHED:
            //set pin to analog out
            pinMode(pin_num, OUTPUT);
            break;
        case FLYWHEEL_TYPE_PWM:
            //allocate a new servo channel to drive the ESC
            pwm_driver = new Servo();
            pwm_driver->attach(pin_num);
            break;
        case FLYWHEEL_TYPE_DSHOT:
            //allocate a new DSHOT channel to drive the ESC
            dshot_driver = new DShotRMT(pin_num);

            //only enable bidirection if the conditions are correct
            if(dshot_mode != DSHOT150)
                dshot_bidir = dshotBidir;

            break;
    }


}

//we might move what's in here into the attach() function
void singleMotor::start()
{
    //does not work unless we call attach() first
    if(!has_been_attached)
        return;
    
        //set up the output pin
    switch(flywheel_type)
    {
        default:
        case FLYWHEEL_TYPE_NULL:
        case FLYWHEEL_TYPE_BRUSHED:
            //nothing to do
            break;
        case FLYWHEEL_TYPE_PWM:
            //arm the servo at min value
            pwm_driver->write(*output_min);
            break;
        case FLYWHEEL_TYPE_DSHOT:
            //final setup of the RMT backend (we don't send values until we explicitly call that function)
            dshot_driver->begin(dshot_mode, dshot_bidir, ESC_DSHOT_MOTOR_POLES);

            break;
    }
}

bool singleMotor::update()
{

    unsigned long deltaMillis = 0;

    //only check on the tick changeover
    if(millis() != last_millis)
    {
        //handle call gaps larger than 1 millisecond
        deltaMillis = millis() - last_millis;
        last_millis = millis();
    }

    //the state of transition_millis means that the motor can be faster, but will never be slower than the speed we want

    //should be 1 step closer
    if(transition_millis > 0)
        transition_millis -= deltaMillis;
    if(transition_millis < 0)
        transition_millis = 0; //avoid negatives



    //re-map speed to motor-used parameters ()
    int mappedSpeed; //value to be written out to the motors
    int rampSpeed;
    if(target_speed >= last_speed) //direct write if we are speeding up
        rampSpeed = target_speed; //instant ramp-up
    else
    {
        //slow rampdown
        if(ramp_down_millis > 0)
            ramp_down_millis -= deltaMillis;
        if(ramp_down_millis < 0)
            ramp_down_millis = 0; //avoid negatives


        rampSpeed = map(ramp_down_millis, start_rd_millis, 0,  last_speed, target_speed);
    }
    mappedSpeed = map(rampSpeed, *rpm_min, *rpm_max, *output_min, *output_max);
    //ensure it is mapped within bounds
    if(mappedSpeed > *output_max)
        mappedSpeed = *output_max;
    if(mappedSpeed < *output_min)
        mappedSpeed = *output_min;


    //update stored RPM
    switch(flywheel_type)
    {
        case FLYWHEEL_TYPE_DSHOT:

            //get real RPM from telemetry if bidirection is enabled
            if(dshot_bidir == ENABLE_BIDIRECTION)
            {
               current_speed = dshot_driver->get_dshot_RPM();
               break;
            }

            //Fallthrough
        default: //for these, predict motor speed based on delay and previous motor speed
        case FLYWHEEL_TYPE_NULL:
        case FLYWHEEL_TYPE_BRUSHED:
        case FLYWHEEL_TYPE_PWM:

            //use simulated motor spinup time if we are accelerating, otherwise use rampdown speed directly
            current_speed = (start_transition_millis == 0 || target_speed < last_speed) ?
                            rampSpeed : map(transition_millis, start_transition_millis, 0, last_speed, rampSpeed);

            break;

    }



    //write target speed to the motor
    switch(flywheel_type)
    {
        default:
        case FLYWHEEL_TYPE_NULL:
            break;
        case FLYWHEEL_TYPE_BRUSHED:
            analogWrite(pin_num, mappedSpeed);
            break;
        case FLYWHEEL_TYPE_PWM:
            pwm_driver->write(mappedSpeed);
            break;
        case FLYWHEEL_TYPE_DSHOT:
            counter_millis += deltaMillis;
            if(counter_millis >= 1) //make sure that the dshot value is only sent 1x each 5 ms
            {
                counter_millis = 0;
                dshot_driver->send_dshot_value(mappedSpeed);
            }
            break;

    }


    //debug section
    // Serial.print(current_speed);
    // Serial.print("\t");
    // Serial.print(transition_millis);
    // Serial.print("\t");
    // Serial.print((current_speed > target_speed * 9 / 10) * 4000);
    // Serial.print("\t");
    // Serial.println(mappedSpeed);



    //check if real speed is close enough to target speed (10% error)
    if(current_speed >= target_speed * 85 / 100)//9 / 10)
        return true; //motor is at speed
    else
        return false; //motor needs more time



}

void singleMotor::set_target_speed(int speed)
{
    if(speed > FLYWHEEL_SPEED_MAX)
        speed = FLYWHEEL_SPEED_MAX;
    else if(speed < FLYWHEEL_SPEED_MIN)
        speed = FLYWHEEL_SPEED_MIN;
    
    //update last speed
    last_speed = current_speed;

    //convert speed to correct units ("real" RPM)
    speed = map(speed, FLYWHEEL_SPEED_MIN, FLYWHEEL_SPEED_MAX, *rpm_min, *rpm_max);

    //map change in speed to ammount of delay we need to reach it

    //for rampdown (if needed)
    if(last_speed > speed)
    {
        ramp_down_millis = map((last_speed - speed), *rpm_min, *rpm_max,  0, ESC_DOWNTHROTTLE_DELAY);
        start_rd_millis = ramp_down_millis;
    }

    //it doesn't matter if this is negative or not (as it will sometimes be)
    transition_millis = map(abs(speed - current_speed), *rpm_min, *rpm_max, 0, delay_millis);

    start_transition_millis = transition_millis;

    //update speed
    target_speed = speed;


}

int singleMotor::get_target_speed()
{
    //convert back to input speed value
    return map(target_speed, *rpm_min, *rpm_max, FLYWHEEL_SPEED_MIN, FLYWHEEL_SPEED_MAX);
}

int singleMotor::get_current_speed(bool actualSpeed)
{

    int speedToReturn = 0;
    //if we have bidirection, return the real speed
    if(dshot_bidir == ENABLE_BIDIRECTION && actualSpeed)
        //return speed directly without mapping
        speedToReturn = current_speed;
    else
        //map simulated speed back to throttle values
        speedToReturn = map(current_speed, *rpm_min, *rpm_max, FLYWHEEL_SPEED_MIN, FLYWHEEL_SPEED_MAX);


    return speedToReturn;


}



//constructor, assigns pins and creates ESC objects based on the contents of 'configuration.h'
flywheelHandler::flywheelHandler()
{

    //setup the flywheel type based on the defines we made earlier
    flywheel_type_t fType = FLYWHEEL_TYPE_NULL;
    dshot_mode_t dType = DSHOT_OFF;
    bidirectional_mode_t bType = NO_BIDIRECTION;
    unsigned int delayM = ESC_REV_DELAY;

    //TODO: make this logic tree out of preprocessor defines
    switch (ESC_TYPE)
    {
        case ESC_TYPE_BRUSHED:
            fType = FLYWHEEL_TYPE_BRUSHED;
            break;
        case ESC_TYPE_PWM:
            fType = FLYWHEEL_TYPE_PWM;

            break;
        case ESC_TYPE_DSHOT:
            fType = FLYWHEEL_TYPE_DSHOT;
            dType = ESC_DSHOT_MODE;
            bType = ESC_DSHOT_BIDIR ? ENABLE_BIDIRECTION : NO_BIDIRECTION;
            break;
        default:
            fType = FLYWHEEL_TYPE_NULL;
            break;
    }


    //attach ESCs to their corresponding pins
    flywheelArray[0].attach(ESC_1_PIN, delayM, fType, dType, bType, &gSettings.esc_output_min, &gSettings.esc_output_max, &gSettings.esc_rpm_min, &gSettings.esc_rpm_max);
    flywheelArray[1].attach(ESC_2_PIN, delayM, fType, dType, bType, &gSettings.esc_output_min, &gSettings.esc_output_max, &gSettings.esc_rpm_min, &gSettings.esc_rpm_max);
#ifdef FOUR_ESCS 
    flywheelArray[2].attach(ESC_3_PIN, delayM, fType, dType, bType &gSettings.esc_output_min, &gSettings.esc_output_max, &gSettings.esc_rpm_min, &gSettings.esc_rpm_max);
    flywheelArray[3].attach(ESC_4_PIN, delayM, fType, dType, bType &gSettings.esc_output_min, &gSettings.esc_output_max, &gSettings.esc_rpm_min, &gSettings.esc_rpm_max);
#endif

    //start motor objects
    for(int i  = 0; i < flywheel_count; ++i)
    {
        flywheelArray[i].start();
    }


}


void flywheelHandler::update()
{
    //updates all ESCs and checks to see if the flywheels as a whole are all at speed
    bool AllAtSpeed = true;
    for(int i = 0; i < flywheel_count; ++i)
    {
        if(!flywheelArray[i].update())
            AllAtSpeed = false;
    }
    at_speed = AllAtSpeed;

}


void flywheelHandler::set_target_speed(int speed)
{
    //avoid unneeded updates
    if(last_speed == speed)
        return;

    for(int i = 0; i < flywheel_count; ++i)
    {
        flywheelArray[i].set_target_speed(speed);
    }
    last_speed = speed;

}


