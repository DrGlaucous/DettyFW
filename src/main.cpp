//final tests (old)
/*
#include <Arduino.h>
#include "filesystem/filesystem.h"
#include "settings/settings.h"
#include "state/state.h"
#include "flywheels/flywheels.h"
#include "display/display.h"

State* state = NULL;
Flywheels* flywheel_set = NULL;
Display* menu = NULL;
TaskHandle_t displayTaskHandle = NULL;


//test
int max_tim = 0;
// hw_timer_t *timer = NULL;
// void IRAM_ATTR logic_loop()
// {
//     timerStop(timer);
//     timerStart(timer);
// }

QueueHandle_t queue_handle;

//priority-0 task, no need for yeild; other tasks will just step on this
void display_loop(void *pvParameters) {

    //setup
    menu = new Display(state->settings);

    DisplayData last_data = DisplayData();
    DisplayData this_data = DisplayData();

    for(;;) {

        bool data_has_changed = false;
        auto result = xQueueReceive(queue_handle, &this_data, 0);
        if (result == pdTRUE) {
            data_has_changed = !!memcmp(&this_data, &last_data, sizeof(DisplayData));
        }

        if(data_has_changed) {
            memcpy(&last_data, &this_data, sizeof(DisplayData));
            menu->tick(last_data);
        }

    }

}


//allocate resources (currently unused)
void setup() {

    //filesystem save/load tests
    // if(false) {
    //     filesystem::initialize();
    //     Settings set_of_settings = Settings(SETTINGS_DIRECTORY);
    //     set_of_settings.load();
    //     //set_of_settings.save();
    // }

    //loads json stuff in here
    
    // //set up ISR
    // timer = timerBegin(0, 80, true); //microsecond resolution timer
    // timerAttachInterrupt(timer, &logic_loop, true);
    // timerAlarmWrite(timer, 400, true); // every 0.01 seconds
    // timerAlarmEnable(timer);

}

//xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
//runs from a freeRTOS task with priority 1
void loop() {


    //setup

    Serial.begin(115200);
    Serial.printf("bet\n");

    state = new State();
    flywheel_set = new Flywheels(state->settings);
    Controller::init(state->settings);

    //create display task + queue to pass in data from the other task
    queue_handle = xQueueCreate(1, sizeof(DisplayData));
    xTaskCreateUniversal(display_loop, "displayTask", getArduinoLoopTaskStackSize(), NULL, 0, &displayTaskHandle, ARDUINO_RUNNING_CORE);
    DisplayData disp_d = DisplayData();

    for(;;) {

        int time_0 = micros();


        state->tick(micros());
        Controller::tick(state->settings);

        int time_1 = micros() - time_0;

        int time_2 = micros() - time_1 - time_0;

        flywheel_set->tick(*state);

        int time_3 = micros() - time_2 - time_1 - time_0;

        if(time_3 > max_tim) {
            max_tim = time_3;
        }


        auto& preset = *state->settings.get_current_preset_mut();
        
        //dirty update HUD info
        {
            bool delta = false;
            //change target RPM and update display
            if(Controller::get_preset_a_changed() && Controller::get_preset_a()) {
                disp_d.multiplier = (disp_d.multiplier * 10) % 1000;
                if(disp_d.multiplier == 0) {
                    disp_d.multiplier = 1;
                }

                delta = true;
            }
            if(Controller::get_preset_b_changed() && Controller::get_preset_b()) {
                preset.flywheel_rpm += disp_d.multiplier;
                delta = true;
            }
            if(Controller::get_preset_c_changed() && Controller::get_preset_c()) {
                preset.flywheel_rpm -= disp_d.multiplier;
                delta = true;
            }

            disp_d.target_speed = preset.flywheel_rpm;
            disp_d.wheel_speed_l = state->flywheel_l_current_rpm;
            disp_d.wheel_speed_r = state->flywheel_r_current_rpm;

            xQueueOverwrite(queue_handle, &disp_d);
            // if(delta) {
            //     xQueueOverwrite(queue_handle, &disp_d);
            // }
        

        }

        //dirty temp. connectome:
        {

            if(Controller::get_trigger()) {
                state->flywheel_target_rpm = preset.flywheel_rpm;
                state->flywheel_rpm_active = true;
                
            } else {
                state->flywheel_target_rpm = 0;
                state->flywheel_rpm_active = false;
            }
        }

        int time_4 = micros() - time_3 - time_2 - time_1 - time_0;

        //Serial.printf("q%d, w%d, e%d, r%d, t%d\n", time_1, time_2, time_3, time_4, max_tim);

        //the finest actual resolution of the freeRTOS delay function is 1 ms
        delay(1);
        
        /// delay types:
        //delay(); //just sleeps the thread
        //delayMicroseconds() //actually halts the processor
        //portTICK_PERIOD_MS
        //configTICK_RATE_HZ
        //vTaskDelay(ms / portTICK_PERIOD_MS);

    }


}

*/

//known-working benchmark but it's broken now for some reason, reason: the delta time part of the PID is broken
/*
#include <Arduino.h>

//TEST
#include "flywheels/flywheels.h"
#include <hal/gpio_hal.h>

#include <DShotRMT.h>

// USB serial port needed for this example
const int USB_SERIAL_BAUD = 115200;
#define USB_Serial Serial

// Define the GPIO pin connected to the motor and the DShot protocol used
const auto MOTOR01_PIN = 23;
const auto MOTOR02_PIN = 18;
const auto DSHOT_MODE = DSHOT600;

// Define the failsafe and initial throttle values
const auto FAILSAFE_THROTTLE = 0;
const auto INITIAL_THROTTLE = 48;

//is the handle trigger pin
#define TRIGGER_PIN 17
#define MAG_PIN 5
//SCL on the PCB
#define POT_PIN 33
#define SUPPLY_PIN 25

DShotRMT anESC = {};//MOTOR01_PIN
DShotRMT anotherESC = {};//MOTOR02_PIN

//good practice: declare vars and methods, in the header file, and define methods in the source file.
//implement PID control for the ESC speed

Flywheels::PIDController* controller;

int loop_count = 0;
uint32_t last_millis = 0;

//time when we sent the last packet
uint32_t last_micros_sent = 0;

//time when we got the last packet
uint32_t last_micros_got = 0;

//persistent PID values
uint32_t last_good_rpm = 0; //feedback RPM
uint32_t esc_throttle = DSHOT_THROTTLE_MIN; //value written to the ESC
uint32_t target_rpm = 0; //RPM we want the ESCs to run at

void setup()
{


	USB_Serial.begin(USB_SERIAL_BAUD);
	anESC.begin(MOTOR02_PIN, DSHOT_MODE, ENABLE_BIDIRECTION, 14);
	anotherESC.begin(MOTOR01_PIN, DSHOT_MODE, ENABLE_BIDIRECTION, 14);

	pinMode(TRIGGER_PIN, INPUT_PULLUP);
	pinMode(SUPPLY_PIN, OUTPUT);
	digitalWrite(SUPPLY_PIN, HIGH);

	pinMode(POT_PIN, INPUT);

	controller = new Flywheels::PIDController(
		DSHOT_THROTTLE_MIN, //start output
		DSHOT_THROTTLE_MAX, //min output lim
		DSHOT_THROTTLE_MIN, //max output lim
		0.0012, //p
		0.00015, //i
		0.002 //d
	);

	Serial.println("starting");
	
}

float rpm_set_now = (float)DSHOT_THROTTLE_MIN;
bool adjust_mode = false;

uint32_t lc2 = 0;
void loop()
{
    lc2 += 1;

	uint16_t rpm_set_now = 400;
	//start halted
	controller->set_output_offset(0);
	
	//throttle control sub-section
	{
		//update our setpoint
		if(digitalRead(MAG_PIN) == 0) {
			uint16_t tpot_value = analogRead(POT_PIN);
			uint16_t pot_value = abs(tpot_value - pot_value) > 50 ? tpot_value : pot_value;

			target_rpm = map(pot_value, 0, 4095, 0, 32200);
			if(target_rpm > 32200) target_rpm = 32200;
			if(target_rpm < 0) target_rpm = 0;
			Serial.println(target_rpm);

		}
		//execute our setpoint
		if(digitalRead(TRIGGER_PIN) == 0) {
			rpm_set_now = target_rpm;

			//start full throttle (we should possible ramp this if we're not shooting at high RPMS)
			controller->set_output_offset(DSHOT_THROTTLE_MAX);

            
		}

	}
	

	//ESC control section
	{
		uint32_t dshot_rpm;
		dshot_get_packet_exit_mode_t response = anESC.get_dshot_packet(&dshot_rpm);

		//good timeout values:
		//dshot300: 300micros
		//dshot600: 150micros
		//using timeout values shorter than this will result in our favorite uninitialized errors (and maybe the `~` to boot) because we're trying to send while getting stuff back
		//it also seems that we can't just up and send a frame right back as soon as we get one, because the ESC doesn't always respond right away.

		//resend a packet as soon as we get a successful rpm value or if we've timed out.
		uint32_t mcro = micros();

		//we got a good value back, update our PID loop
		if(response == DECODE_SUCCESS) {
			esc_throttle = (uint16_t)controller->tick(rpm_set_now, dshot_rpm, mcro - last_micros_got);

			last_micros_got = mcro;

			Serial.printf("%d | %d || %d\n", dshot_rpm, rpm_set_now, esc_throttle);

		}

		//for some reason, it's stepping on it. I'm going to force regular intervals
		//if(response == DECODE_SUCCESS || mcro - last_micros_sent > 150) {
		if(mcro - last_micros_sent > 200) {
			anESC.send_dshot_value(esc_throttle);
			last_micros_sent = mcro;

            // lc2++;
            // if(lc2 % 200 == 0) {
            //     lc2 = 0;
            //     Serial.printf("%d | %d || %d\n", dshot_rpm, rpm_set_now, esc_throttle);
            // }

		}

	}



}
*/


//final tests

#include <Arduino.h>
#include <DShotRMT.h>

#include "filesystem/filesystem.h"
#include "settings/settings.h"
#include "state/state.h"
#include "flywheels/flywheels.h"

//Flywheels::Wheel whl = {};

State state = State();

void setup() {

    Serial.begin(115200);
    Serial.printf("bet\n");

    state.load_settings();

    Controller::init(state.settings);
    
    Flywheels::init(state.settings);
    Flywheels::set_throttle_override(0); //clamped to DSHOT_RPM_MIN


    delay(1000);

}

float throttle_override = (float)0.0;
bool adjust_mode = false;
void loop() {

    Controller::tick(state.settings);

    //click-on-off between adjusting throttle and not
    if(Controller::get_trigger() && Controller::get_trigger_changed()) {
        adjust_mode = !adjust_mode;
    }

    if(adjust_mode) {
        // if(Controller::get_preset_a()) {
        //     throttle_override += 0.5;
        // } else {
        //     throttle_override -= 0.5;
        // }
        throttle_override = 20000.0;
    } else {
        throttle_override -= 100.0;
    }


    // if(throttle_override > (float)DSHOT_THROTTLE_MAX) {
    //     throttle_override = (float)DSHOT_THROTTLE_MAX;
    // } 
    if (throttle_override < (float)0.0) {
        throttle_override = (float)0.0;
    }
    
    if(throttle_override < 1.0) {
        Flywheels::set_throttle_override(DSHOT_THROTTLE_MIN);
    } else {
        Flywheels::set_target_rpm((uint32_t)throttle_override);
    }


    //push back our latest throttle changes to the driver
    Flywheels::update_pid();



    //Serial.printf("%d || %d\n", Controller::get_trigger(), Controller::get_preset_a());

    //according to variance graphs, the motors shouldn't really have a variance of more than 300 between stable RPMs,
    //at higher speeds, this variance grows (+=300 given 12000 RPM)
    //16700-15800 given 16200
    //23000 <- 12 volt overcurrent threshhold for bench PSU

    //occasionally we get a bad packet that is off by a factor of 2000 or so. We need to discard these.
    //if ramping up, discard RPM readbacks that are more than -1000 from current value
    //once we meet or surpass the goal, switch to +-1000 from prev. value

    //testing debug stuff
    delay(1);
    Serial.printf("%d || %d || %f || %f || %f || %f || %f\n",
        Flywheels::get_wheel_l_rpm(),
        Flywheels::get_wheel_r_rpm(),
        throttle_override,
        Flywheels::wheel_l.motor_controller.ival,
        Flywheels::wheel_r.motor_controller.ival,
        Flywheels::wheel_l.motor_controller.last_pval,
        Flywheels::wheel_r.motor_controller.last_pval
    );


}





//known-working benchmark but it works for some reason (ans: constant delta time in the PID controller)
/*
#include <Arduino.h>

//TEST
#include <hal/gpio_hal.h>
#include <DShotRMT.h>

#include "flywheels/flywheels.h"

// USB serial port needed for this example
const int USB_SERIAL_BAUD = 115200;
#define USB_Serial Serial

// Define the GPIO pin connected to the motor and the DShot protocol used
const auto MOTOR01_PIN = 23;
const auto MOTOR02_PIN = 18;
const auto DSHOT_MODE = DSHOT600;

// Define the failsafe and initial throttle values
const auto FAILSAFE_THROTTLE = 0;
const auto INITIAL_THROTTLE = 48;

//is the handle trigger pin
#define TRIGGER_PIN 17
#define MAG_PIN 5
//SCL on the PCB
#define POT_PIN 33
#define SUPPLY_PIN 25

DShotRMT anESC;//MOTOR01_PIN
DShotRMT anotherESC;//MOTOR02_PIN

//good practice: declare vars and methods, in the header file, and define methods in the source file.
//implement PID control for the ESC speed

Flywheels::PIDController* controller;

int loop_count = 0;
uint32_t last_millis = 0;

//time when we sent the last packet
uint32_t last_micros_sent = 0;

//time when we got the last packet
uint32_t last_micros_got = 0;

//persistent PID values
uint32_t last_good_rpm = 0; //feedback RPM
uint32_t esc_throttle = DSHOT_THROTTLE_MIN; //value written to the ESC
uint32_t target_rpm = 0; //RPM we want the ESCs to run at



void setup()
{

	USB_Serial.begin(USB_SERIAL_BAUD);
	anESC.begin(MOTOR02_PIN, DSHOT_MODE, ENABLE_BIDIRECTION, 14);
	anotherESC.begin(MOTOR01_PIN, DSHOT_MODE, ENABLE_BIDIRECTION, 14);

	pinMode(TRIGGER_PIN, INPUT_PULLUP);
	pinMode(SUPPLY_PIN, OUTPUT);
	digitalWrite(SUPPLY_PIN, HIGH);

	pinMode(POT_PIN, INPUT);

	controller = new Flywheels::PIDController(
		DSHOT_THROTTLE_MIN, //start output
		DSHOT_THROTTLE_MAX, //min output lim
		DSHOT_THROTTLE_MIN, //max output lim
		0.0012, //p
		0.00015, //i
		0.002 //d
	);

	Serial.println("starting");
	
}


void loop()
{

	uint16_t rpm_set_now = 400;
	//start halted
	controller->set_output_offset(0);
	
	//throttle control sub-section
	{
		//update our setpoint
		if(digitalRead(MAG_PIN) == 0) {
			uint16_t tpot_value = analogRead(POT_PIN);
			uint16_t pot_value = abs(tpot_value - pot_value) > 50 ? tpot_value : pot_value;

			target_rpm = map(pot_value, 0, 4095, 0, 32200);
			if(target_rpm > 32200) target_rpm = 32200;
			if(target_rpm < 0) target_rpm = 0;
			Serial.println(target_rpm);

		}
		//execute our setpoint
		if(digitalRead(TRIGGER_PIN) == 0) {
			rpm_set_now = target_rpm;

			//start full throttle (we should possible ramp this if we're not shooting at high RPMS)
			controller->set_output_offset(DSHOT_THROTTLE_MAX);
		}

	}
	

	//ESC control section
	{
		uint32_t dshot_rpm;
		dshot_get_packet_exit_mode_t response = anESC.get_dshot_packet(&dshot_rpm);

		//good timeout values:
		//dshot300: 300micros
		//dshot600: 150micros
		//using timeout values shorter than this will result in our favorite uninitialized errors (and maybe the `~` to boot) because we're trying to send while getting stuff back
		//it also seems that we can't just up and send a frame right back as soon as we get one, because the ESC doesn't always respond right away.

		//resend a packet as soon as we get a successful rpm value or if we've timed out.
		uint32_t mcro = micros();

		//we got a good value back, update our PID loop
		if(response == DECODE_SUCCESS) {
			esc_throttle = (uint16_t)controller->tick(rpm_set_now, dshot_rpm, 200);

			last_micros_got = mcro;

			Serial.printf("%d | %d || %d\n", dshot_rpm, rpm_set_now, esc_throttle);

		}

		//for some reason, it's stepping on it. I'm going to force regular intervals
		//if(response == DECODE_SUCCESS || mcro - last_micros_sent > 150) {
		if(mcro - last_micros_sent > 200) {
			anESC.send_dshot_value(esc_throttle);
			last_micros_sent = mcro;
		}

	}


}
*/
























