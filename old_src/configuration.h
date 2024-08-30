//configuration stuff goes here

#pragma once

#include <DShotRMT.h>



//-------------------------------//
//------------outputs------------//
//-------------------------------//

//#define SOLENOID 25 // (HIGH=ON)
#define BUZZER 13 // (HIGH=ON)

//////////////FLYWHEELS//////////////

//use this range to set the min/max values the motor class will take to control the motor, since everything is mapped, values here don't matter too much
//range of valid input
#define FLYWHEEL_SPEED_MIN 0
#define FLYWHEEL_SPEED_MAX 1024 


#define ESC_TYPE_BRUSHED 1 
#define ESC_TYPE_PWM 2 
#define ESC_TYPE_DSHOT 3 

//use the defines above to define this
#define ESC_TYPE ESC_TYPE_DSHOT 

//time in milliseconds between 0 and full before the motor is considered "at speed"
//if ESC_DSHOT_BIDIR is set to true, then this value is ignored
#define ESC_REV_DELAY 1000

//delay in milliseoconds the program should take to decelerate the ESC.
//it hurts both the battery and the ESC to be dropped right to 0 each time the trigger is let off (many ESCs enforce active motor breaking),
//this will slowly ramp down the target speed instead of instantly drop it
//if the motor is brushed, this can be set to 0 (no active breaking there!)
#define ESC_DOWNTHROTTLE_DELAY 3000

//MIN and MAX values for each type of motor output
//for brushed motors, this is the duty cycle of the driver (0-255)
//for PWM escs, this is the angle of the servo (37-180)
//for DSHOT, this is the digital values (default: 48-2047)
#define ESC_OUTPUT_MIN 48 
#define ESC_OUTPUT_MAX 2047 

//the RPM the program expects to get from the motor
//to find most accurate real values for this, motors with telemetry will need to be calibrated
//(can be closely estimated by multiplying kv by voltage)
//for motors without telemetry, these value can be set to anything with decent resolution (to get a "fake" telemetry gauge)
#define ESC_RPM_MIN 0 
#define ESC_RPM_MAX 36800 //39733 //36800 || //29800 //27600 


//only have the option for this if we have the dshot type,
#if ESC_TYPE == ESC_TYPE_DSHOT 
    //dshot mode
    #define ESC_DSHOT_MODE DSHOT300
    //enables eRPM telemetry
    #define ESC_DSHOT_BIDIR true
    //number of magnet poles the motor has, important for getting real RPM from eRPM
    #define ESC_DSHOT_MOTOR_POLES 14
#endif


//esc pinouts
#define ESC_1_PIN 18
#define ESC_2_PIN 23

//#define FOUR_ESCS
#ifdef FOUR_ESCS
    #define ESC_3_PIN
    #define ESC_4_PIN
#endif

//////////////PUSHER//////////////

//the range of input (full speed = PUSHER_SPEED_MAX)
#define PUSHER_SPEED_MAX 1024
#define PUSHER_SPEED_MIN 0

#define PUSHER_TYPE_BRUSHED 0 //not implemented
#define PUSHER_TYPE_STEPPER 1 //not implemented
#define PUSHER_TYPE_SOLENOID 2
#define PUSHER_TYPE_NULL 3

//set the pusher type here
#define PUSHER_TYPE PUSHER_TYPE_SOLENOID

//set the corresponding defines here. you only need to touch the ones that correspond with PUSHER_TYPE
//PUSHER_TYPE_BRUSHED
//the two pins on the h bridge driver
#define H_A_PIN -1
#define H_B_PIN -1

//PUSHER_TYPE_STEPPER
//the pins that control the stepper driver
#define S_STEP_PIN -1
#define S_ENABLE_PIN -1
#define S_DIRECTION_PIN -1


//PUSHER_TYPE_SOLENOID
//the pin that controls the solenoid FET
#define SO_FET_PIN 32 //use buzzer for testing
//time in ms it takes the solenoid to extend
#define SO_MIN_EXT_TIME 25
//time in ms it takes the solenoid to retract
#define SO_MIN_RET_TIME 25
//max time the solenoid is allowed to be ON
#define SO_MAX_EXT_TIME 200



//------------------------------//
//------------inputs------------//
//------------------------------//


//////////////buttons//////////////
//all buttons will use internal pullup resistors by default

//debounce time in ms
#define DEBOUNCE_TIME 10

//the pin that this element occupies
#define TRIGGER_PIN 17
//if normally closed, set this to true, if normally open, set to false
#define TRIGGER_NC false

#define MAG_TRIG_PIN 5
#define MAG_TRIG_NC false

//do we have a rev trigger in the mix? if yes, we change the way some rev behavior works
//TODO: implement this.
//#define USE_REV_TRIG
#ifdef USE_REV_TRIG
  #define REV_TRIG_PIN 16
  #define REV_TRIG_NC false
#endif

//preset buttons
#define PRESET_A_PIN 27
#define PRESET_A_NC false

#define PRESET_B_PIN 14
#define PRESET_B_NC false

#define PRESET_C_PIN 12
#define PRESET_C_NC false

//encoder pins for the UI interaction
#define ENCODER_A_PIN 25
#define ENCODER_B_PIN 33

#define ENCODER_BUTTON_PIN 26
#define ENCODER_BUTTON_NC false


//////////////ammo counter//////////////
//voltage+ pin for the reciever diode (not absolutely needed, but saves power as it is only set ON when it is time to read)
#define IR_REC_POWER 15
//other side of the diode, measures IR light level
#define IR_REC_READBACK 35
//IR emitter LED, not absolutely needed, but it saves wear on the LED and battery if used
#define IR_EMITTER 2

//value used to trigger the start of a beam break (a dart has entered the detector)
#define IR_ADC_FALL_VAL 100
//value used to trigger the end of a beam break (the dart has left)
//this is larger than the other one to prevent ADC fuzziness
#define IR_ADC_RISE_VAL 200
//we need to know this because we only have one detector. By default, this is the length of a half dart (in mm)
#define DART_LENGTH 36


//voltmeter
#define VOLTMETER_PIN 34
//ohms of the resistors in the voltage divider (units don't matter here, so long as they're the same)
//for instance, these are kOhms for me.
#define R1_VAL 330
#define R2_VAL 22
//set these to map the ADC properly (what is the value of the ADC with respect to this voltage?)
//example: if we read a voltage of 0.772 volts, we should expect an ADC reading of 800
#define ADC_VALUE_REF_1 0
#define ADC_VOLTAGE_REF_1 0

#define ADC_VALUE_REF_2 800
#define ADC_VOLTAGE_REF_2 0.772

//voltages at which the battery is full and empty
#define BATTERY_FULL_CHARGE 16
#define BATTERY_EMPTY 12


//-------------------------------//
//------------display------------//
//-------------------------------//
#define I2C_SDA_PIN 19
#define I2C_SCL_PIN 22 //for the lolin32, this is also the external LED (LOW = ON with this)
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

//setup i2c display, see https://github.com/olikraus/u8g2/wiki/u8g2setupcpp for a list of proper constructors
//currently, this sets up a generic 128x64 display on the hardware i2c BUS
#include <U8g2lib.h>
#define INIT_CLASS U8G2_SSD1306_128X64_NONAME_1_HW_I2C


#define DEBUG_BAUD_RATE 115200//9600
