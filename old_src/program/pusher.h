#pragma once


#include <Arduino.h>

#include "main.h"
#include "configuration.h"



//types of pusher backends
class solenoidHandler
{
    public:
    //assign variables
    solenoidHandler(int pinFet, int* minExtendTime, int* minRetractTime, int* maxExtendTime, int* maxRetractTime);
    //activate pins
    void start();
    //run this as often as possible
    bool update();

    //stop a fullAuto or pushBurst operation
    void halt();

    //start running at full auto at this speed
    void pushFullAuto(int speed);

    //run a round burst at this speed
    void pushBurst(int speed, unsigned int count);

    private:


    //config settings//
    int pin_fet = -1;

    //values correspond to max speed
    //the ammount of time it takes the solenoid to make a full stroke (ms)
    int* min_extend_time;
    int* min_retract_time;

    //values correspond to min speed
    //the max ammount of time the solenoid can be on (100% duty cycle will destroy a solenoid)
    int* max_extend_time;
    int* max_retract_time; //caluclated from the other values

    //dynamics//
    unsigned int burst_count = 0; //how many times we need to push yet
    //unsigned int start_burst_count = 0; //how many times we initially had to push

    //for the session, how long (ms) the solenoid should be in these states
    int curr_extend_time = 0;
    int curr_retract_time = 0;

    //did we jsut start a pushing operation
    bool begin = false;

    int countdown_millis = 0; //used to count down the delay for each push in/out cycle (may be negative!)
    bool solenoid_state = false;//the state of the solenoid (true == ON)

    unsigned long last_millis = 0; //for changing only 1x per tick


};

//TODO: these classes are not done yet
class brushedHBridgeHandler
{
    private:

    int pin_A;
    int pin_B;

    //the slowest analogWrite value that will produce movement, corresponds to PUSHER_SPEED_MIN
    unsigned char min_duty_cycle = 0x0;
    //the fastest analogWrite value that will be used (mapped from PUSHER_SPEED_MAX)
    unsigned char max_duty_cycle = 0xFF;

};
class stepperHandler
{
    private:

    int pin_step;
    int pin_enable;
    int pin_direction;




};


//wraps the different pusher backends into one to be used anywhere else in the program
class pusherHandler
{
    public:
    pusherHandler();

    //run this as often as possible
    void update();

    //stop a fullAuto or pushBurst operation
    void halt();

    //start running at full auto at this speed
    void pushFullAuto(int speed);

    //run a round burst at this speed
    void pushBurst(int speed, unsigned int count);

    bool is_pushing = false; //current active state of the pusher

    private:
    //pointers to backend objects of each pusher type
    //(the pointer will only point to a valid object if that type is enabled in the configuration)
    solenoidHandler* solHandler;
    brushedHBridgeHandler* hbHandler;
    stepperHandler* stpHandler;


};














