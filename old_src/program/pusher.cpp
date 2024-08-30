
#include <Arduino.h>

#include "main.h"
#include "configuration.h"
#include "pusher.h"


/// SOLENOID CLASS

solenoidHandler::solenoidHandler(int pinFet, int* minExtendTime, int* minRetractTime, int* maxExtendTime, int* maxRetractTime)
{
    pin_fet = pinFet;
    min_extend_time = minExtendTime;
    max_extend_time = maxExtendTime;

    min_retract_time = minRetractTime;
    max_retract_time = maxRetractTime;

}

void solenoidHandler::start()
{
    pinMode(pin_fet, OUTPUT);
}

void solenoidHandler::pushFullAuto(int speed)
{
    if(speed > PUSHER_SPEED_MAX)
        speed = PUSHER_SPEED_MAX;
    else if(speed < PUSHER_SPEED_MIN)
        speed = PUSHER_SPEED_MIN;

    //find how quickly we need to cycle the solenoid
    //currently, the code performs a 50/50 duty cycle instead of doing short bursts at a dynamic space
    //both ON state and OFF state change with speed
    curr_extend_time = map(speed, PUSHER_SPEED_MIN, PUSHER_SPEED_MAX, *max_extend_time, *min_extend_time);
    curr_retract_time = map(speed, PUSHER_SPEED_MIN, PUSHER_SPEED_MAX, *max_retract_time, *min_retract_time);

    // Serial.print(curr_extend_time);
    // Serial.print("\t");
    // Serial.print(curr_retract_time);
    // Serial.println(" []");



    //check for already running operations and stop them
    if(burst_count > 0)
        halt();

    begin = true;

}

void solenoidHandler::pushBurst(int speed, unsigned int count)
{
    pushFullAuto(speed);
    burst_count = count;
}

bool solenoidHandler::update()
{
    bool isPushing = (begin || burst_count > 0); //return current push state

    unsigned long deltaMillis = millis() - last_millis;
    last_millis = millis();
    //don't waste cycles if the time is the same
    if(deltaMillis == 0)
        return isPushing;


    bool fullAuto = false;
    //started new command or in full-auto mode
    if(begin == true)
    {
        if(burst_count > 0) //we're entering a burst round
        {
            begin = false;
        }
    }

    //do the pushing
    if(begin || burst_count > 0)
    {
        //time to change states
        if((countdown_millis -= deltaMillis) <= 0)
        {
            //1/2 of total time, for slow speeds, use a 50/50 duty cycle
            countdown_millis = (curr_retract_time + curr_extend_time) / 2;

            //prioritize the slower of the two times
            //check if the duty cycle needs altering to account for the cycles
            // if(*min_extend_time < *min_retract_time)
            // {
            //     //if the solenoid is currently extended, use the retract time directly, otherwise use what's leftover
            //     if(countdown_millis < *min_retract_time)
            //         countdown_millis = solenoid_state ? *min_retract_time : countdown_millis - *min_retract_time;
            // }
            // else
            //     if(countdown_millis < *min_extend_time)
            //         countdown_millis = solenoid_state ? countdown_millis - *min_extend_time : *min_extend_time;

            if(solenoid_state == true)
                --burst_count;
            
            solenoid_state = !solenoid_state;

            // //if the solenoid is currently OUT
            // if(solenoid_state == true)
            // {
            //     --burst_count; //we shot a dart, decrement cycler
            //     countdown_millis = curr_retract_time; //setup wait for the retract part
            // }
            // else
            //     countdown_millis = curr_extend_time; //setup wait for the extend part
            // //flip-flop the solenoid state
            // solenoid_state = !solenoid_state;
 
        }
    }
    else
    {
        solenoid_state = false; //ensure the solenoid is OFF when nothing is happening
    }

    digitalWrite(pin_fet, solenoid_state);

    return isPushing;

}

void solenoidHandler::halt()
{
    begin = false; //stop any potential full-auto or new starts
    burst_count = 0; //zero out remaining burst time
    countdown_millis = 0;
}


///TODO: the other classes



//pusher wrapper
pusherHandler::pusherHandler()
{

    //initialize the proper backend object
    switch(PUSHER_TYPE)
    {
        case PUSHER_TYPE_BRUSHED:
            hbHandler = new brushedHBridgeHandler();
            break;
        case PUSHER_TYPE_STEPPER:
            stpHandler = new stepperHandler();
            break;
        case PUSHER_TYPE_SOLENOID:
            solHandler = new solenoidHandler(SO_FET_PIN, &gSettings.so_min_ext_time, &gSettings.so_min_ret_time, &gSettings.so_max_ext_time, &gSettings.so_max_ret_time);
            solHandler->start();
            break;
        default:
        case PUSHER_TYPE_NULL:
            break;
    }

}

void pusherHandler::update()
{
    //update the correct backend
    switch(PUSHER_TYPE)
    {
        case PUSHER_TYPE_BRUSHED:
            //hbHandler->update();
            break;
        case PUSHER_TYPE_STEPPER:
            //stpHandler->update();
            break;
        case PUSHER_TYPE_SOLENOID:
            is_pushing = solHandler->update();
            break;
        default:
        case PUSHER_TYPE_NULL:
            break;
    }



}

void pusherHandler::halt()
{
    //update the correct backend
    switch(PUSHER_TYPE)
    {
        case PUSHER_TYPE_BRUSHED:
            //hbHandler->update();
            break;
        case PUSHER_TYPE_STEPPER:
            //stpHandler->update();
            break;
        case PUSHER_TYPE_SOLENOID:
            solHandler->halt();
            break;
        default:
        case PUSHER_TYPE_NULL:
            break;
    }



}

void pusherHandler::pushFullAuto(int speed)
{
    //update the correct backend
    switch(PUSHER_TYPE)
    {
        case PUSHER_TYPE_BRUSHED:
            //hbHandler->pushFullAuto(speed);
            break;
        case PUSHER_TYPE_STEPPER:
            //stpHandler->pushFullAuto(speed);
            break;
        case PUSHER_TYPE_SOLENOID:
            solHandler->pushFullAuto(speed);
            break;
        default:
        case PUSHER_TYPE_NULL:
            break;
    }



}

void pusherHandler::pushBurst(int speed, unsigned int count)
{
    //update the correct backend
    switch(PUSHER_TYPE)
    {
        case PUSHER_TYPE_BRUSHED:
            //hbHandler->pushBurst(speed, count);
            break;
        case PUSHER_TYPE_STEPPER:
            //stpHandler->pushBurst(speed, count);
            break;
        case PUSHER_TYPE_SOLENOID:
            solHandler->pushBurst(speed, count);
            break;
        default:
        case PUSHER_TYPE_NULL:
            break;
    }



}





