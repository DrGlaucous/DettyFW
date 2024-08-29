#pragma once

#include <Arduino.h>
#include "utilities.h"
#include "settings.h"
#include "input.h"
#include "utilities.h"
#include "flywheels.h"
#include "pusher.h"
#include "settings.h"


extern all_settings_t gSettings;
extern sharedSettingsHandler gSettingsShare;
extern inputHandler gPins;
extern flywheelHandler gFlywheel;
extern pusherHandler gPusher;
extern serialHandler gInfo;
extern buzzerHandler gBuzzer;
extern trigHandler gTrig;
//extern chronyHandler gChronograph;




