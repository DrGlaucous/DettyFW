#include <string>

#include <Arduino.h>
#include <ArduinoJson.h>

#include "settings.h"
#include "../filesystem/filesystem.h"

using std::string;

Settings::Settings(const char* directory) {
    this->directory = string(directory);
}
Settings::~Settings() {

}


bool Settings::load() {

    JsonDocument document;
    {
        uint8_t *filebuf;
        size_t filelen;
        auto result = filesystem::readFile(directory.c_str(), &filebuf, &filelen);

        //failed to load settings, reset to default
        if(!result) {
            *this = Settings(this->directory.c_str());
            return false;
        }

        deserializeJson(document, *filebuf, filelen);

        //finished decoding, free filebuffer
        free(filebuf);
    }

    





}
bool Settings::save() {

}
