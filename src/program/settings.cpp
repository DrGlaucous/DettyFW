#include <DShotRMT.h>
#include <Arduino.h>
#include "configuration.h"
#include "settings.h"

//this is less "settings" now and more "shared memory"
//we will also store button states in here, too (primarily for the menu...)


//create queues to send settings to the different backends
sharedSettingsHandler::sharedSettingsHandler()
{
    //settings_queue = xQueueCreate(1, sizeof(all_settings_t));
    //telemetry_queue = xQueueCreate(1, sizeof(telemetry_settings_t));
}

void sharedSettingsHandler::start()
{
    settings_queue = xQueueCreate(1, sizeof(all_settings_t));
    telemetry_queue = xQueueCreate(1, sizeof(telemetry_settings_t));
}

//send global settings
bool sharedSettingsHandler::sendSettings(all_settings_t* settingsBuffer)
{
    return xQueueOverwrite(settings_queue, settingsBuffer);
}
//get global settings
bool sharedSettingsHandler::getSettings(all_settings_t* settingsBuffer)
{
    //xQueuePeek
    return xQueueReceive(settings_queue, settingsBuffer, 0);
}

//send telemetry
bool sharedSettingsHandler::sendTelemetry(telemetry_settings_t* telemBuffer)
{
    return xQueueOverwrite(telemetry_queue, telemBuffer);
}
//get telemetry
bool sharedSettingsHandler::getTelemetry(telemetry_settings_t* telemBuffer)
{
    return xQueueReceive(telemetry_queue, telemBuffer, 0);
}






