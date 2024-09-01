#include <Arduino.h>

#include "settings/settings.h"

Settings set_of_settings = Settings(SETTINGS_DIRECTORY);

void setup() {

    set_of_settings.load();
    set_of_settings.save();

}

void loop() {


}

