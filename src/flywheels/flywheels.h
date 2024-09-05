#pragma once

#include <Arduino.h>
#include <DShotRMT.h>


#include "../settings/settings.h"
#include "../state/state.h"


class PIDController {

	public:

	PIDController(
		int32_t start_output_offset,
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

    //runs the pid loop for one cycle
	int32_t tick(uint32_t target_rpm, uint32_t curr_rpm, uint32_t delta_micros) {

		//small RPM = big throttle value, direct acting

		delta_micros = 200; //for testing

		//determine PID values

		float pval = (float)target_rpm - (float)curr_rpm;

		//if our value is increasing/decreasing and we limited the output, don't continue integration
		if(
			!(
			(pval > 0.0 && has_limited_high) ||
			(pval < 0.0 && has_limited_low)
			)
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

	private:

	//vars
	float p_const;
	float i_const;
	float d_const;

	int32_t max_lim = DSHOT_THROTTLE_MAX;
	int32_t min_lim = DSHOT_THROTTLE_MIN;

	int32_t start_output_offset;

	//data retainers
	float last_pval = 0;
	float ival = 0;

	bool has_limited_high = false;
	bool has_limited_low = false;

};


class Flywheels {

    public:


    //see: initializer list
    Flywheels(State& state);
    ~Flywheels();

    //all inputs are passed in through the "state"
    void tick(State& state);


    private:

	//if true, tick will return immediately
    bool disabled_mode = true;

	uint32_t motor_tx_delay_micros = 0;

    DShotRMT* motor_l;
	PIDController motor_l_controller = PIDController(
		DSHOT_THROTTLE_MIN, //start output
		DSHOT_THROTTLE_MAX, //min output lim
		DSHOT_THROTTLE_MIN, //max output lim
		0.0012, //p
		0.00015, //i
		0.002 //d
	);
	uint32_t l_last_micros_got = 0;

	DShotRMT* motor_r;
	PIDController motor_r_controller = PIDController(
		DSHOT_THROTTLE_MIN, //start output
		DSHOT_THROTTLE_MAX, //min output lim
		DSHOT_THROTTLE_MIN, //max output lim
		0.0012, //p
		0.00015, //i
		0.002 //d
	);
	uint32_t r_last_micros_got = 0;


	//timestamp when we sent the last dshot packet (shared)
	uint32_t last_micros_sent = 0;

};


