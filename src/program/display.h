#pragma once



#include <Arduino.h>

#include "main.h"
#include "configuration.h"
#include "settings.h"

//the menu library
#include <GEM_u8g2.h>


class menuHandler
{
    public:
    menuHandler();
    void start();

    void update();

    private:

    //types

    //enum list of things that can be drawn onscreen
    typedef enum display_list_e
    {
        DISPLAY_NULL, //draw nothing
        DISPLAY_MENU, //main navegation menu
        DISPLAY_FLYWHEEL_POWER, //gauges n such
        DISPLAY_PUSH_RATE,
        DISPLAY_FLYWHEEL_RPM,
        DISPLAY_AMMO_COUNT,
        DISPLAY_FPS,
        DISPLAY_BIND_PRESET,
        DISPLAY_MAX,

    } display_list_t;

    typedef enum action_list_e
    {
        ACTION_CHANGE_MENU,
        ACTION_SAVE_SETTINGS,
        ACTION_RPM_AUTOCONFIG,
        ACTION_PROCESS_PUSHER_SETTINGS,
        ACTION_PUSH_LIVE_SETTINGS,

        ACTION_MAX,
    } action_list_t;


    //callback datapacks for the various buttons
    typedef struct callback_datapack_s //for setting a number
    {
        menuHandler* parent;
        action_list_t action_no;
        display_list_t screen_type;
        all_settings_t* settings;

    } callback_datapack_t;




    //vars

    //for drawing pages outside a do-while loop
    bool drawing_screen = true; //re-draw screen on init
    bool first_page = true;

    //for menu long-press (mainly for editing numbers)
    bool was_held = false;

    //one of the preset buttons was held for sufficient time (shared by all 3)
    bool preset_switched = false;

    //preset bar labels
    const char* preset_labels[3] =
    {
        "pre A",
        "pre B",
        "pre C",
    };


    //local copy of the blaster's settings to be modified and pushed back to the backend
    all_settings_t settings_clone; //no need to initialize the object, all vars come pre-initalized
    //indicates it's time to push out a settings update
    bool settings_changed = false;

    //a re-used live setting list to act as an intermediary between the GUI and the real live settings
    live_settings_t current_live_set = {};
    int testInt = 0;

    //display class
    INIT_CLASS u8g2;

    //current selected screen
    display_list_t screen_type = DISPLAY_MENU;


    //callback settings for the menu
    const callback_datapack_t saveSettingsDP = {this, ACTION_SAVE_SETTINGS, DISPLAY_NULL, &settings_clone};
    const callback_datapack_t bindPresetDP = {this, ACTION_CHANGE_MENU, DISPLAY_BIND_PRESET, &settings_clone};
    const callback_datapack_t rpmAutoconfigDP = {this, ACTION_RPM_AUTOCONFIG, DISPLAY_NULL, &settings_clone};
    const callback_datapack_t pusherSettingChangeDP = {this, ACTION_PROCESS_PUSHER_SETTINGS, DISPLAY_NULL, &settings_clone};
    const callback_datapack_t shootPowerDP = {this, ACTION_CHANGE_MENU, DISPLAY_FLYWHEEL_POWER, &settings_clone};
    const callback_datapack_t shootRateDP = {this, ACTION_CHANGE_MENU, DISPLAY_PUSH_RATE, &settings_clone};
    const callback_datapack_t flywheelRPMDP = {this, ACTION_CHANGE_MENU, DISPLAY_FLYWHEEL_RPM, &settings_clone};
    const callback_datapack_t ammoCountDP = {this, ACTION_CHANGE_MENU, DISPLAY_AMMO_COUNT, &settings_clone};
    const callback_datapack_t fpsViewDP = {this, ACTION_CHANGE_MENU, DISPLAY_FPS, &settings_clone};
    const callback_datapack_t liveSettingsDP = {this, ACTION_PUSH_LIVE_SETTINGS, DISPLAY_NULL, &settings_clone};

    //list items
    SelectOptionInt fireTypeListOptions[3] =
    {
        {"Full", SHOOT_MODE_FULL_AUTO},
        {"Sel.", SHOOT_MODE_SELECT_FIRE},
        {"Cache", SHOOT_MODE_CACHE},
    };
    SelectOptionInt triggerTypeListOptions[2] =
    {
        {"Tap", TRIGGER_MODE_PRESS},
        {"Hold", TRIGGER_MODE_COMMIT},
    };

    //main menu objects (indented for readability)
    GEMPage mainMenu;
    GEMPage settingsPage;
    GEMItem toSettingsPage;
        GEMItem saveAllSettings;
        GEMItem bindPreset;
        GEMPage escConfigPage;
        GEMItem toEscConfigPage;
            GEMItem escMinOutput;
            GEMItem escMaxOutput;
            GEMItem toEscFeedbackPage;
            GEMPage escFeedbackPage;
                GEMItem escMinRPM;
                GEMItem escMaxRPM;
                GEMItem autoConfRPM;
        GEMPage pusherConfigPage; //these will only be present if the pusher backend is defined
        GEMItem toPusherConfigPage;
            GEMItem minExtendTime;
            GEMItem maxExtendTime;
            GEMItem minRetractTime;   
        GEMPage wifiPage; //unfinished
        GEMItem toWifiPage; 
            GEMItem wifiButton;
    GEMItem shootPower;
    GEMItem shootRate;
    GEMPage shootModePage;
    GEMItem toshootModePage;
        GEMSelect fireTypeList;
        GEMItem fireType;
        GEMItem burstCount;
        GEMSelect triggerTypeList;
        GEMItem triggerType;
        GEMItem cacheDelay;
    GEMItem flywheelRPM;
    GEMItem ammoCount;
    GEMItem fpsView;

    GEM_u8g2 menu;


	//functions

    bool handlePreset(); //switch active preset with preset buttons

	bool handleNav(); //process button input and navegate menu


    //handles all callbacks that come from the menu
    //sets a flag that corresponds to the menu that should be entered
    static void handleCallback(GEMCallbackData inData);

    //copy the settings the menu holds to the master settings index
    void pushLiveSettings(live_settings_t fromThis, live_settings_t* toThis);

    //puts the menu elements onscreen
    void screenDrawLoop(void);

    //various draw utilities, specific gauges re-use these elements
    int loopItem(int item, int min, int max);
    static float map_float(float x, float in_min, float in_max, float out_min, float out_max);
    static void drawDialGauge(INIT_CLASS *miniDisp, short x, short y, short radius, int min, int max, int val);
    static void drawBarGauge(INIT_CLASS *miniDisp, bool isHorizontal, short x, short y, short width, short height, int min, int max, int val);

    void drawPowerGauge(INIT_CLASS *miniDisp, short x, short y);

    void drawSelectedPreset(INIT_CLASS *miniDisp, short x, short y, short width, short height,
							const char** labels, unsigned char box_count, unsigned char selected_box);


};
