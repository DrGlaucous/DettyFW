#include <Arduino.h>
#include "configuration.h"
#include "input.h"
#include "utilities.h"



//serial wrapper functions:

serialHandler::serialHandler(int baudRate)
{
    Serial.begin(baudRate);
}
serialHandler::~serialHandler()
{
    Serial.end(true);
}
void serialHandler::send_message(const char* message)
{
    Serial.print(message);
}
void serialHandler::send_message_line(const char* message)
{
    Serial.println(message);
}


//chrony functions
//uses hardware timer '1'

//make non-classed function and variables for timer interrupt
hw_timer_t *chron_timer = NULL; //timer object
bool chron_state = false; //true if a dart is inside the trigger zone
unsigned int chron_count = 0; //how many darts have passed through the chronograph
//these only apply to the most recent dart
unsigned long micros_start = 0; //when the dart entered
unsigned long micros_end = 0; //when the dart left
void IRAM_ATTR timerHandle()
{

    int result = safeAnalogRead(IR_REC_READBACK);

    //when the dart first enters
    if (chron_state == false && result < IR_ADC_FALL_VAL)
    {
        chron_state = true;
        micros_start = micros();
    }
    else if(chron_state == true && result > IR_ADC_RISE_VAL)
    {
        chron_state = false;
        micros_end = micros();
        ++chron_count;
    }
}


//set pinmodes and configure timer inturrupts
chronyHandler::chronyHandler()
{
    //set pins to the correct mode
    pinMode(IR_REC_POWER, OUTPUT);
	pinMode(IR_REC_READBACK, INPUT);
	pinMode(IR_EMITTER, OUTPUT);


    //create fast timer and immediately stop it (initialize only)
    chron_timer = timerBegin(1, 80, true);
    timerStop(chron_timer);

    //create inturrupt
    timerAttachInterrupt(chron_timer, &timerHandle, true);
    timerAlarmWrite(chron_timer, 100, true); // every 100 microseconds (may need to change...)
    //timerAlarmEnable(chron_timer);


}
//start the interrupt timer and turn on the LEDs
void chronyHandler::begin_isr()
{

    //enable LEDs
    digitalWrite(IR_REC_POWER, true);
	digitalWrite(IR_EMITTER, true);

    //start ISR callback
    timerStart(chron_timer);
    timerAlarmEnable(chron_timer);

}
//stop the timer and turn off the LEDs
void chronyHandler::end_isr()
{

    //stop ISR callback
    timerAlarmDisable(chron_timer);
    timerStop(chron_timer);

    //disable LEDs
    digitalWrite(IR_REC_POWER, true);
	digitalWrite(IR_EMITTER, true);

}
//get current chronograph dart count and speed
void chronyHandler::get_darts(unsigned long * deltaMicros, unsigned int * count)
{

    //do not update values if we're in the middle of a read (will yield a garbage value otherwise)
    if(!chron_state)
    {
        //overflow protection
        if(micros_end < micros_start)
            lastDeltaMicros = (0xFFFFFFFF - micros_start) + micros_end;
        else
            lastDeltaMicros = micros_end - micros_start;
    }


    *deltaMicros = lastDeltaMicros;
    *count = chron_count;

}
//reset dart count back to 0
void chronyHandler::reset_darts()
{
    chron_count = 0;
}




//buzzer functions:

buzzerHandler::buzzerHandler(int pinNo)
{
    pin_no = pinNo;
    //do not initialize an output if we set the pin to -1
    if(pinNo == -1)
        return;

    pinMode(pinNo, OUTPUT);
    digitalWrite(pinNo, LOW);
}
void buzzerHandler::update()
{

    //only check on the tick changeover
    if(micros() == last_micros)
        return;

    //ensure the buzzer is OFF when we run out of cycles
    if(count_times < 1 && loop == false)
    {
        set_pin(pin_no, LOW);
        return;
    }

    current_micros = micros() - start_micros;

    //after low time, decrement count and switch pin ON
    if(current_micros > micros_on + micros_off)
    {
        start_micros = micros();
        --count_times;
    }
    else if(current_micros > micros_on)
    {
        set_pin(pin_no, LOW);
    }
    else
    {
        set_pin(pin_no, HIGH);
    }

    last_micros = micros();

}
void buzzerHandler::beep_single(unsigned int microsOn)
{
    micros_on = microsOn;
    micros_off = 0;
    count_times = 1;
    current_micros = micros();
    start_micros = current_micros;
    loop = false;
}
void buzzerHandler::beep_multiple(unsigned int microsOn, unsigned int microsOff, unsigned int countTimes)
{
    micros_on = microsOn;
    micros_off = microsOff;
    count_times = countTimes;
    current_micros = micros();
    
    loop = false;
}
void buzzerHandler::beep_multiple_continuous(unsigned int microsOn, unsigned int microsOff)
{
    micros_on = microsOn;
    micros_off = microsOff;
    current_micros = micros();
    start_micros = current_micros;
    loop = true;
}
void buzzerHandler::set_beep(bool on)
{
    set_pin(pin_no, on);
    loop = true;
}
//handle -1 pins
inline void buzzerHandler::set_pin(int pinNo, bool value)
{
    if(pinNo < 0)
        return;
    digitalWrite(pinNo, value);
}


//trig functions
trigHandler::trigHandler()
{
    for (int i = 0; i < 0x100; ++i)
            sinTbl[i] = (short)(sin(i * TWO_PI / 256.0) * 512.0);
}

short trigHandler::GetSin(unsigned char deg)
{
	return sinTbl[deg];
}

short trigHandler::GetCos(unsigned char deg)
{
	deg += 0x40;//90 degree offset
	return sinTbl[deg];
}


