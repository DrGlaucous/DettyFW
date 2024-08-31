#pragma once

#include <vector>
#include <string>

using std::string;
using std::vector;


#define SETTINGS_DIRECTORY "settings/settings.json"

//convention:
/*
variables use snake_case names
classes, enums, and structs use CamelCase names
use of enums must always use the qualifier: enum::qualifier
*/

typedef enum ShootMode
{
    Null = -1,
    FullAuto = 0,
    SelectFire = 1,
    Cache = 2,
    Max,

}ShootMode;
typedef enum TriggerMode
{
    Null = -1,
    Press = 0,
    Commit = 1,
    Max,
}TriggerMode;

typedef struct Preset {

    string name;
    TriggerMode trigger_mode;
    ShootMode shoot_mode;
    int burst_count;
    int cache_delay_ms;
    int push_rate_pct;
    int flywheel_rpm;

} Preset;


// typedef struct Variables {
//     vector<Preset> preset_array;
//     uint8_t preset_a;
//     uint8_t preset_b;
//     uint8_t preset_c;
// } Variables;



class Settings {


    public:

    Settings(const char* directory);

    //deserialize json and populate struct
    bool load();

    //serialize and save json to directory
    bool save();

    //return a preset object with the current index
    Preset get_preset(size_t index);
    


    private:

    //where in the filesystem the settings are loaded from
    string directory;



};










