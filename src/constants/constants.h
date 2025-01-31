#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include <time.h>




/* You only need to format LittleFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin */
#define FORMAT_LITTLEFS_IF_FAILED true



#define ISR_TIMER_POT 0
#define ISR_TIMER_FLYWHEEL 1
#define ISR_TIMER_CHRONO 2





