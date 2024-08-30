#pragma once



#include <Arduino.h>
#include "configuration.h"


//a wrapper for serial communication
class serialHandler
{
    public:
        serialHandler(int baudRate);
        ~serialHandler();
        void send_message(const char* message);
        void send_message_line(const char* message);



};


//buzzer handler: let's make some noize!
//currently only supports active buzzers (beep when a voltage is applied, no PWM)
class buzzerHandler
{
    public:
        buzzerHandler(int pinNo);
        void update();
        void beep_single(unsigned int microsOn);
        void set_beep(bool on);
        void beep_multiple(unsigned int microsOn, unsigned int microsOff, unsigned int countTimes);
        void beep_multiple_continuous(unsigned int microsOn, unsigned int microsOff);
    
    private:

        int pin_no = 0;

        unsigned long start_micros = 0;
        //micros() - start_micros
        unsigned long current_micros = 0;
        //used to detect a change in ticks
        unsigned long last_micros = 0;

        unsigned int micros_on = 0;
        unsigned int micros_off = 0;
        unsigned int count_times = 0;
        bool loop = false;

        inline void set_pin(int pinNo, bool value);

};


//contains lightweight implementations of sin() and cos() via lookup table
//unit circle represented with 256 degrees and a radius of +-512
class trigHandler
{
    //guess what? TWO_PI is already defined in Arduino.h!
    //#define TWO_PI 6.2831998

    public:
    trigHandler();
    short GetSin(unsigned char deg);
    short GetCos(unsigned char deg);

    private:
    short sinTbl[0x100];
    


};


