#pragma once

#include <Arduino.h>
#include <DShotRMT.h>


#include "../settings/settings.h"
#include "../state/state.h"



namespace Flywheels {

	class PIDController {

		public:

		PIDController(
			int32_t start_output_offset = DSHOT_THROTTLE_MIN,
			int32_t max_lim = DSHOT_THROTTLE_MAX,
			int32_t min_lim = DSHOT_THROTTLE_MIN,
			float p_const = 0.2,
			float i_const = 0.2,
			float d_const = 0.2
		) {
			if(start_output_offset > max_lim)
				start_output_offset = max_lim;

			this->start_output_offset = start_output_offset;
			this->max_lim = max_lim;
			this->min_lim = min_lim;

			this->p_const = p_const;
			this->i_const = i_const;
			this->d_const = d_const;

		};

		~PIDController() {
		};

		//runs the pid loop for one cycle (uses floats, non-ISR-safe)
		int32_t tick(uint32_t target_rpm, uint32_t curr_rpm, uint32_t delta_micros) {

			//small RPM = big throttle value, direct acting

			//delta_micros = 200; //for testing

			//determine PID values

			float pval = (float)target_rpm - (float)curr_rpm;

			//if our value is increasing/decreasing and we limited the output, don't continue integration
			//or if our current value is 0 (rely only on P control for startup)
			if(
				!(
				(pval > 0.0 && has_limited_high) ||
				(pval < 0.0 && has_limited_low)
				)
				&& curr_rpm != 0
			) {
				ival += pval * i_const * (float)delta_micros;
			}

			float dval = pval - last_pval;
			last_pval = pval;

			//sum and multiply by constants
			int32_t pid_out = (int32_t)(
				pval * p_const * (float)delta_micros
				+ ival //integral constant is applied above
				+ dval * d_const * (float)delta_micros
			) + start_output_offset;

			//limit output
			if(pid_out > max_lim) {
				pid_out = max_lim;
				has_limited_high = true;
				has_limited_low = false;
			} else if (pid_out < min_lim) {
				pid_out = min_lim;
				has_limited_high = false;
				has_limited_low = true;
			} else {
				has_limited_high = false;
				has_limited_low = false;
			}

			return(pid_out);
		
		}

		//called whenever we take manual control of the loop, resets persistent I and D values
		void reset() {
			last_pval = 0;
			ival = 0;
			has_limited_high = false;
			has_limited_low = false;
		}

		//controls what extra value will be added to the loop, typically for starting at "full throttle"
		void set_output_offset(int32_t offset) {
			if(offset < min_lim) {
				start_output_offset = min_lim;
			} else if(offset > max_lim) {
				start_output_offset = max_lim;
			} else {
				start_output_offset = offset;
			}
		}


		int32_t test(uint32_t target_rpm, uint32_t curr_rpm, uint32_t delta_micros) {


			Serial.printf("A TEST %d\n", target_rpm);

			float pval = (float)target_rpm;

			//test
			Serial.printf("==%f==1\n", pval);
			pval = 2.0;
			
			Serial.printf("==%f==2\n", pval);
			return 0;
		}


		private:

		//vars
		float p_const = 0.0;
		float i_const = 0.0;
		float d_const = 0.0;

		//data retainers
		float last_pval = 0;
		float ival = 0;

		int32_t max_lim = DSHOT_THROTTLE_MAX;
		int32_t min_lim = DSHOT_THROTTLE_MIN;

		int32_t start_output_offset = 0;


		bool has_limited_high = false;
		bool has_limited_low = false;

	};

	class Wheel {

		public:

		/// constructors empty for the time being: we pre-allocate everything on initialization
		Wheel() {}

		~Wheel() {
		}

		/// creates dshot driver, starts pid, etc.
		void init(
			PIDController& controller,
			uint8_t pin,
			dshot_mode_t mode,
			bidirectional_mode_e bd_mode,
			int16_t pole_count
		) {
			//don't try to do it more than once
			if(initialized) {
				return;
			}

			//set up motor driver
			motor_controller = controller;

			//todo: motorPoleCount==0 needs to be caught in the dshotRMT function itself, not here
			motor.begin(
				pin,
				mode,
				bd_mode,
				pole_count == 0 ? 1 : pole_count
			);

			//pre-make a dshot value to store in case send_dshot_packet is called before prepare_dshot_packet
			motor.prepare_dshot_value(motor_raw_trottle);

			initialized = true;

		}

		/// updates PID given stored average and new RPM goal
		void tick_pid(
			uint32_t flywheel_target_rpm,
			bool flywheel_rpm_override,
			uint32_t flywheel_override_throttle
		) {

			//wheel hasn't been initialized, don't run anything
			if(!initialized) {
				return;
			}

			//calculate new RPM from rolling average since last tick
			if(got_count > 0) {
				flywheel_current_rpm = av_rpm_total / got_count;
			}

			if(flywheel_rpm_override) {
				//run motor directly from this output
				motor_raw_trottle = flywheel_override_throttle;
				motor_controller.reset();
			} else {
				//update PID
				motor_raw_trottle = motor_controller.tick(flywheel_target_rpm, flywheel_current_rpm, 200);
				//last_micros_got = abs_micros;
			}

		}
		
		//prepare the next throttle value to be transmitted from ISR (separate because we need to pause the ISR for this. it needs to be quick-as-possible)
		void prepare_dshot_packet() {
			motor.prepare_dshot_value(motor_raw_trottle);
			//reset rolling average
			got_count = 0;
			av_rpm_total = 0;
		}

		///send (and get) dshot values, should be ISR friendly
		void IRAM_ATTR send_dshot_packet() {
			uint32_t rpm = 0;
			dshot_get_packet_exit_mode_t response = motor.get_dshot_packet(&rpm);

			if(response == DECODE_SUCCESS) {
				got_count++;
				av_rpm_total += rpm;
			}

			//test
			//uint32_t aa = motor_controller.tick(fltr, flywheel_current_rpm, 200);

			motor.send_last_value();
		}

		/// get current RPM (refreshed with tick_pid)
		inline uint32_t get_rpm() const{
			return flywheel_current_rpm;
		}
		/// get current raw target throttle (refreshed with tick_pid)
		inline uint16_t get_throttle() const{
			return motor_raw_trottle;
		}


		private:

		bool initialized = false;
		PIDController motor_controller = {}; //PID controller
		DShotRMT motor = {}; //ll motor
		uint32_t last_micros_got = 0;
		uint16_t motor_raw_trottle = DSHOT_THROTTLE_MIN;
		uint32_t flywheel_current_rpm = 0;

		volatile uint32_t got_count = 0; //used for calculating the average RPM since the controller was last ticked
		volatile uint32_t av_rpm_total = 0; //average RPM between motor ticks, flywheel_current_rpm is updated with the motor tick

		//last single RPM from the motor, used to see if what we got is an outlier (note: threshhold here may need to change depending on how fast the motor ramps up)
		volatile uint32_t last_rpm = 0;

	};


	void init(const Settings& settings);
	void set_target_rpm(uint32_t rpm);
	void set_throttle_override(uint32_t throttle);
	void deinit();
	void update_pid();

	uint32_t get_wheel_l_rpm();
	uint32_t get_wheel_r_rpm();
	
	// static Wheel wheel_r;
	// static Wheel wheel_l;
	// static bool dummy_mode;
	// static hw_timer_t* timer;
	// uint32_t motor_tx_delay_micros = 8;

	extern uint32_t process_time;

	//used for testing purposes
	// extern Flywheels::Wheel wheel_l;
	// extern Flywheels::Wheel wheel_r;

};






/*
class Flywheels {

    public:


    //see: initializer list
    Flywheels(Settings& settings);
    ~Flywheels();

    //all inputs are passed in through the "state"
    void tick();


    private:

	//if true, tick will return immediately
    bool dummy_mode;

	//time between transmit packets
	uint32_t motor_tx_delay_micros;

	//timestamp when we sent the last dshot packet (shared)
	uint32_t last_micros_sent;

	//ISR handler
	hw_timer_t* timer = NULL;

	//flywheel control parameters
	bool flywheel_rpm_active = false;
	uint32_t flywheel_target_rpm = 0;

	//motor parameters
    DShotRMT* motor_l = NULL;
	PIDController motor_l_controller = {};
	uint32_t l_last_micros_got = 0;
	uint16_t motor_l_raw_trottle = 0;
	uint32_t flywheel_l_current_rpm = 0;

	DShotRMT* motor_r = NULL;
	PIDController motor_r_controller = {};
	uint32_t r_last_micros_got = 0;
    uint16_t motor_r_raw_trottle = 0;
	uint32_t flywheel_r_current_rpm = 0;


	// handle downthrottling in the connectome
	// uint8_t state_action_no = 0; //used to perform sequential actions on state change
	// uint32_t l_downthrottle_step_size = 0;
	// uint32_t r_downthrottle_step_size = 0;
};
*/

