#include <Arduino.h>
#include "filesystem/filesystem.h"
#include "settings/settings.h"

Settings set_of_settings = Settings(SETTINGS_DIRECTORY);

void setup() {

    Serial.begin(115200);
    Serial.printf("bet\n");

    filesystem::initialize();

    set_of_settings.load();
    //set_of_settings.save();

}

void loop() {


}

