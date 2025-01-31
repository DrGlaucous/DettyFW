#pragma once

#include <vector>
#include <string>
#include <Bounce2.h>
#include <Encoder.h>

#include "../settings/settings.h"


class Controller {
    public:




    static void init(Settings& settings);
    static void deinit();

    static inline bool get_trigger() {
        return trigger_state;
    }
    static inline bool get_preset_a() {
        return preset_a_state;
    }
    static inline bool get_preset_b() {
        return preset_b_state;
    }
    static inline bool get_preset_c() {
        return preset_c_state;
    }
    static inline bool get_trigger_changed() {
        return trigger_changed;
    }
    static inline bool get_preset_a_changed() {
        return preset_a_changed;
    }
    static inline bool get_preset_b_changed() {
        return preset_b_changed;
    }
    static inline bool get_preset_c_changed() {
        return preset_c_changed;
    }

    static inline int get_encoder_abs() {
        return last_encoder_val;
    }
    static inline int get_encoder_delta() {
        return delta_encoder_val;
    }

    
    static void tick(Settings& settings);

    private:


    static IRAM_ATTR void update_encoder();

    static hw_timer_t* timer;
    static IRAM_ATTR Encoder* rotary_encoder;

    static Bounce trigger;
    static bool trigger_state;
    static bool trigger_changed;

    static Bounce preset_a;
    static bool preset_a_state;
    static bool preset_a_changed;

    static Bounce preset_b;
    static bool preset_b_state;
    static bool preset_b_changed;

    static Bounce preset_c;
    static bool preset_c_state;
    static bool preset_c_changed;

    static int32_t encoder_value;
    static int32_t delta_encoder_val;
    static int32_t last_encoder_val;

    static Bounce encoder_a;
    static Bounce encoder_b;

};













