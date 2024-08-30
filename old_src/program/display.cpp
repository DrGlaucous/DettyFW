
#include <Arduino.h>

#include "main.h"
#include "configuration.h"
#include "display.h"
#include "settings.h"


//the menu library
#include <GEM_u8g2.h>

//test function (function callbacks don't work if we refrence a non-static class funciton)
void beeper(GEMCallbackData iV)
{
	iV.valPointer;
	gBuzzer.beep_multiple(50, 150, 5);
}

menuHandler::menuHandler(): u8g2(U8G2_R0, U8X8_PIN_NONE , I2C_SCL_PIN, I2C_SDA_PIN),
mainMenu("Main"),
settingsPage("Settings"),
toSettingsPage("Settings", settingsPage),
	saveAllSettings("Save All Settings", handleCallback, (void*)&saveSettingsDP),
	bindPreset("Bind Preset", handleCallback, (void*)&bindPresetDP),
	escConfigPage("ESC Config"),
	toEscConfigPage("ESC Config", escConfigPage),
		escMinOutput("Min Out", gSettings.esc_output_min),
		escMaxOutput("Max Out", gSettings.esc_output_max),
		escFeedbackPage("Feedback"),
		toEscFeedbackPage("Feedback", escFeedbackPage),
			escMinRPM("Min RPM", gSettings.esc_rpm_min),
			escMaxRPM("Max RPM", gSettings.esc_rpm_max),
			autoConfRPM("RPM Autoconfig", handleCallback, (void*)&rpmAutoconfigDP),
	pusherConfigPage("Pusher Config"),
	toPusherConfigPage("Pusher Config", pusherConfigPage),
		minExtendTime("Min Ext Time", gSettings.so_min_ext_time, handleCallback, (void*)&pusherSettingChangeDP),
		maxExtendTime("Max Ext time", gSettings.so_max_ext_time, handleCallback, (void*)&pusherSettingChangeDP),
		minRetractTime("Min Ret Time", gSettings.so_min_ret_time, handleCallback, (void*)&pusherSettingChangeDP),
	wifiPage("Wifi"),
	toWifiPage("Wifi", wifiPage),
		wifiButton("Not Done Yet", gSettings.so_min_ret_time, true), //random value in here for now
shootPower("Power", handleCallback, (void*)&shootPowerDP),
shootRate("Rate", handleCallback, (void*)&shootRateDP),
shootModePage("Shoot Mode"),
toshootModePage("Mode", shootModePage),
	fireTypeList(sizeof(fireTypeListOptions)/sizeof(SelectOptionInt), fireTypeListOptions),
	fireType("Fire", current_live_set.shoot_mode, fireTypeList, handleCallback, (void*)&liveSettingsDP), //SelectOptionByte
	burstCount("Burst #", current_live_set.burst_count, handleCallback, (void*)&liveSettingsDP),
	triggerTypeList(sizeof(triggerTypeListOptions)/sizeof(SelectOptionInt), triggerTypeListOptions),
	triggerType("Trigger", current_live_set.trigger_mode, triggerTypeList, handleCallback, (void*)&liveSettingsDP),
	cacheDelay("Cache ms", current_live_set.cache_delay, handleCallback, (void*)&liveSettingsDP),
flywheelRPM("RPM View", handleCallback, (void*)&flywheelRPMDP),
ammoCount("Ammo View", handleCallback, (void*)&ammoCountDP),
fpsView("FPS View", handleCallback, (void*)&fpsViewDP),
menu(u8g2)
{
	pushLiveSettings(gSettings.preset_settings[gSettings.selected_preset], &current_live_set);
}

void menuHandler::start()
{
	u8g2.begin(); //led backend


	//start the menu library
	menu.setSplashDelay(0);
	menu.init();
	
	//add items to menu

	//root
	mainMenu.addMenuItem(toshootModePage);
	mainMenu.addMenuItem(shootRate);
	mainMenu.addMenuItem(shootPower);
	mainMenu.addMenuItem(flywheelRPM);
	mainMenu.addMenuItem(ammoCount);
	mainMenu.addMenuItem(fpsView);
	mainMenu.addMenuItem(toSettingsPage);

	//settings page
	settingsPage.setParentMenuPage(mainMenu);
	settingsPage.addMenuItem(saveAllSettings);
	settingsPage.addMenuItem(bindPreset);
	settingsPage.addMenuItem(toEscConfigPage);
	settingsPage.addMenuItem(toPusherConfigPage);
	settingsPage.addMenuItem(toWifiPage);

	//esc config page
	escConfigPage.setParentMenuPage(settingsPage);
	escConfigPage.addMenuItem(escMinOutput);
	escConfigPage.addMenuItem(escMaxOutput);
	escConfigPage.addMenuItem(toEscFeedbackPage);
	escConfigPage.addMenuItem(toPusherConfigPage);

	//esc feedback page
	escFeedbackPage.setParentMenuPage(escConfigPage);
	escFeedbackPage.addMenuItem(escMinRPM);
	escFeedbackPage.addMenuItem(escMaxRPM);
	escFeedbackPage.addMenuItem(autoConfRPM);

	//pusher config page
	pusherConfigPage.setParentMenuPage(settingsPage);
	pusherConfigPage.addMenuItem(minExtendTime);
	pusherConfigPage.addMenuItem(maxExtendTime);
	pusherConfigPage.addMenuItem(minRetractTime);

	//wifi page
	wifiPage.setParentMenuPage(settingsPage);
	wifiPage.addMenuItem(wifiButton);

	//shoot mode page
	shootModePage.setParentMenuPage(mainMenu);
	shootModePage.addMenuItem(fireType);
	shootModePage.addMenuItem(burstCount);
	shootModePage.addMenuItem(triggerType);
	shootModePage.addMenuItem(cacheDelay);


	//add page to menu and set it as current
	menu.setMenuPageCurrent(mainMenu);
	menu.drawMenu();





}




void menuHandler::update()
{
	bool needsRefresh = false;

	if(handlePreset()) //switch presets if needed
		needsRefresh = true;

	if(handleNav()) //navegate menu or settings
		needsRefresh = true;

	if(needsRefresh)
	{
		drawing_screen = true;
		first_page = true;
	}


	//don't draw anything
	if(drawing_screen == false)
         return;

	if(first_page)
	{
		first_page = false;
		u8g2.firstPage();
		screenDrawLoop();
	}
	else if(u8g2.nextPage())
		screenDrawLoop();
	else
		drawing_screen = false;

}

bool menuHandler::handlePreset()
{
	//only un-press if no preset buttons are pressed
	if(!gPins.presetA.isPressed()
		&& !gPins.presetB.isPressed()
		&& !gPins.presetC.isPressed()
		)
	{
		preset_switched = false;
		return false;
	}
	bool needsRefresh = false;

	//signify that the button has been pressed
	if(gPins.presetA.pressed() || gPins.presetB.pressed() || gPins.presetC.pressed())
	{
		gBuzzer.beep_single(10000);
	}

	if(preset_switched == false
		&& gPins.presetA.isPressed()
		&& gPins.presetA.currentDuration() > gSettings.preset_hold_time)
	{
		gSettings.selected_preset = PRESET_A;
		preset_switched = true;
		needsRefresh = true;
		gBuzzer.beep_single(100000);
	}
	if(preset_switched == false
		&& gPins.presetB.isPressed()
		&& gPins.presetB.currentDuration() > gSettings.preset_hold_time)
	{
		gSettings.selected_preset = PRESET_B;
		preset_switched = true;
		needsRefresh = true;
		gBuzzer.beep_single(100000);
	}
	if( preset_switched == false
		&& gPins.presetC.isPressed()
		&& gPins.presetC.currentDuration() > gSettings.preset_hold_time)
	{
		gSettings.selected_preset = PRESET_C;
		preset_switched = true;
		needsRefresh = true;
		gBuzzer.beep_single(100000);
	}

	if(needsRefresh)
		pushLiveSettings(gSettings.preset_settings[gSettings.selected_preset], &current_live_set);

	return needsRefresh;

}


//#define GEM_KEY_NONE    0                         // No key presses are detected
//#define GEM_KEY_UP      U8X8_MSG_GPIO_MENU_UP     // Up key is pressed (navigate up through the menu items list, select next value of the digit/char of editable variable, or previous option in select)
//#define GEM_KEY_RIGHT   U8X8_MSG_GPIO_MENU_NEXT   // Right key is pressed (navigate through the link to another (child) menu page, select next digit/char of editable variable, execute code associated with button)
//#define GEM_KEY_DOWN    U8X8_MSG_GPIO_MENU_DOWN   // Down key is pressed (navigate down through the menu items list, select previous value of the digit/char of editable variable, or next option in select)
//#define GEM_KEY_LEFT    U8X8_MSG_GPIO_MENU_PREV   // Left key is pressed (navigate through the Back button to the previous menu page, select previous digit/char of editable variable)
//#define GEM_KEY_CANCEL  U8X8_MSG_GPIO_MENU_HOME   // Cancel key is pressed (navigate to the previous (parent) menu page, exit edit mode without saving the variable, exit context loop if allowed within context's settings)
//#define GEM_KEY_OK      U8X8_MSG_GPIO_MENU_SELECT // Ok/Apply key is pressed (toggle boolean menu item, enter edit mode of the associated non-boolean variable, exit edit mode with saving the variable, execute code associated with button)
bool menuHandler::handleNav()
{
	//determine the direction we've rotated, so we know what navegation char to add to the buffer
	bool someUpdate = (gPins.delta_encoder_val != 0);

	switch(screen_type)
	{
		default:
		case DISPLAY_NULL:
			break;
		case DISPLAY_MENU: //navegate the menu
		{
			//for each encoder click, update the menu quickly
			for(int i = 0; i < abs(gPins.delta_encoder_val); ++i)
			{
				if(menu.readyForKey())
				{
					if(gPins.delta_encoder_val < 0)
						menu.registerKeyPress(GEM_KEY_DOWN);
					else
						menu.registerKeyPress(GEM_KEY_UP);
				}
			}
			//check for long press
			if(gPins.encoderSwitch.currentDuration() > 500 && gPins.encoderSwitch.isPressed() && was_held == false)
			{
				someUpdate = true;
				if(menu.readyForKey())
				{
					menu.registerKeyPress(GEM_KEY_RIGHT);
				}
				was_held = true;
			}
			//check for release
			if(gPins.encoderSwitch.released())
			{
				someUpdate = true;
				if(menu.readyForKey() && was_held == false)
				{
					menu.registerKeyPress(GEM_KEY_OK);
				}
				was_held = false;
			}

			break;
		}
		case DISPLAY_FLYWHEEL_POWER: //update flywheel speed in the settings
		{

			live_settings_t *currentLS = &gSettings.preset_settings[gSettings.selected_preset];
			currentLS->flywheel_speed -= gPins.delta_encoder_val * (FLYWHEEL_SPEED_MAX - FLYWHEEL_SPEED_MIN) / 48;

			if(currentLS->flywheel_speed > FLYWHEEL_SPEED_MAX)
				currentLS->flywheel_speed = FLYWHEEL_SPEED_MAX;

			if(currentLS->flywheel_speed < FLYWHEEL_SPEED_MIN)
				currentLS->flywheel_speed = FLYWHEEL_SPEED_MIN;

			if(gPins.encoderSwitch.released())
			{
				pushLiveSettings(*currentLS, &current_live_set);
				screen_type = DISPLAY_MENU;
				someUpdate = true;
			}

			break;
		}
		case DISPLAY_PUSH_RATE: //update pusher speed in the settings
		{

			live_settings_t *currentLS = &gSettings.preset_settings[gSettings.selected_preset];
			currentLS->pusher_rate -= gPins.delta_encoder_val * (PUSHER_SPEED_MAX - PUSHER_SPEED_MIN) / 48;

			if(currentLS->pusher_rate > PUSHER_SPEED_MAX)
				currentLS->pusher_rate = PUSHER_SPEED_MAX;

			if(currentLS->pusher_rate < PUSHER_SPEED_MIN)
				currentLS->pusher_rate = PUSHER_SPEED_MIN;

			if(gPins.encoderSwitch.released())
			{
				pushLiveSettings(*currentLS, &current_live_set);//put newest settings back into the menu
				screen_type = DISPLAY_MENU;
				someUpdate = true;
			}

			break;
		}
		

	}

	if(someUpdate)
		gBuzzer.beep_single(1000); //haptic ticks

	return someUpdate;
}

void menuHandler::pushLiveSettings(live_settings_t fromThis, live_settings_t* toThis)
{
	memcpy(toThis, &fromThis, sizeof(live_settings_t));
}

//performa a certain action depending on the menu option pressed
void menuHandler::handleCallback(GEMCallbackData inData)
{

	callback_datapack_t* castData = (callback_datapack_t*)inData.valPointer;

	bool change_menu = false;
	switch(castData->action_no)
	{
		default:
		case 0: //change menu to be drawn
			change_menu = true;
			break;
		case 1: //save settings
			//todo: this
			break;
		case 2: //autoconfig RPM
			//todo: this
			break;
		case 3: //process pusher settings
			gSettings.so_max_ret_time = (gSettings.so_max_ext_time / gSettings.so_min_ext_time) * gSettings.so_min_ret_time;
			break;
		case 4: //update the preset's settings
			//copy the menu's settings to the preset
			pushLiveSettings(castData->parent->current_live_set, &gSettings.preset_settings[gSettings.selected_preset]);
			break;





	}

	if(change_menu)
	{
		castData->parent->screen_type = castData->screen_type;
	}
}


//decide what to draw when it's actually time to do so
void menuHandler::screenDrawLoop(void)
{

	//switch statement controls what will be drawn, (for the code that controls what will be processed, look in handleNav)
	switch(screen_type)
	{
		default:
		case DISPLAY_NULL:
			drawPowerGauge(&u8g2, DISPLAY_WIDTH - 20, 0);
			break;
		case DISPLAY_MENU:
			menu.drawMenu(true);
			break;
		case DISPLAY_FLYWHEEL_POWER: //SET the speed
			{
				//show selected preset
				drawSelectedPreset(&u8g2, 0, DISPLAY_HEIGHT - 8, DISPLAY_WIDTH, 8, preset_labels, 3, gSettings.selected_preset);


				//show voltage
				drawPowerGauge(&u8g2, DISPLAY_WIDTH - 20, 0);

				//show how fast we want the flywheels to spin
				drawDialGauge(&u8g2, DISPLAY_WIDTH / 2,
							DISPLAY_HEIGHT / 2,
							(DISPLAY_HEIGHT - 20) / 2,
							FLYWHEEL_SPEED_MIN,
							FLYWHEEL_SPEED_MAX,
							gSettings.preset_settings[gSettings.selected_preset].flywheel_speed);

				float percentPower = map_float(gSettings.preset_settings[gSettings.selected_preset].flywheel_speed, FLYWHEEL_SPEED_MIN, FLYWHEEL_SPEED_MAX, 0, 100);
				//draw percentage below gauge
				char percentText[10] = {}; //should be 8, but 10 for safety
				sprintf(percentText, "%.2f%%", percentPower); //"000.00%"
				u8g2.setFont(GEM_FONT_SMALL); //4x6 font
				u8g2.drawStr(DISPLAY_WIDTH / 2 - (4*3),
							0,//DISPLAY_HEIGHT/2 + (DISPLAY_HEIGHT - 20) / 2,
							percentText);

				break;
			}
		case DISPLAY_PUSH_RATE: //set fire rate
			{
				//show voltage
				drawPowerGauge(&u8g2, DISPLAY_WIDTH - 20, 0);

				//show how fast we want the flywheels to spin
				drawDialGauge(&u8g2, DISPLAY_WIDTH / 2,
							DISPLAY_HEIGHT / 2,
							(DISPLAY_HEIGHT - 20) / 2,
							PUSHER_SPEED_MIN,
							PUSHER_SPEED_MAX,
							gSettings.preset_settings[gSettings.selected_preset].pusher_rate);

				float percentRate = map_float(gSettings.preset_settings[gSettings.selected_preset].pusher_rate, PUSHER_SPEED_MIN, PUSHER_SPEED_MAX, 0, 100);
				//draw percentage below gauge
				char percentText[10] = {}; //should be 8, but 10 for safety
				sprintf(percentText, "%.2f%%", percentRate); //"000.00%"
				u8g2.setFont(GEM_FONT_SMALL); //4x6 font
				u8g2.drawStr(DISPLAY_WIDTH / 2 - (4*3),
							0,//DISPLAY_HEIGHT/2 + (DISPLAY_HEIGHT - 20) / 2,
							percentText);
							

				break;
			}
		case DISPLAY_FLYWHEEL_RPM: //SEE the speed (set it too)
			{
				gBuzzer.beep_single(100000);
				screen_type = DISPLAY_MENU;
				break;
			}


	}




}



//utility functions for drawing screen elements:

//loop input between min and max value (like over/underflows) min<=x<max
int menuHandler::loopItem(int item, int min, int max)
{
    //make it so that min = 0
    item -= min;
    max -= min;

    if(item >= max)
        item = (item % max); //0+...
    else if(item < 0)
        item = max + (item % max) - 1;

    return item + min;//add min back


    //if(*selectedItem >= entryCount)
    //    *selectedItem = (*selectedItem % entryCount); //0+...
    //else if(*selectedItem < 0)
    //    *selectedItem = entryCount + (*selectedItem % entryCount);

}

//the map function but it uses floats
float menuHandler::map_float(float x, float in_min, float in_max, float out_min, float out_max) {
    const float run = in_max - in_min;
    if(run == 0){
        log_e("map(): Invalid input range, min == max");
        return -1; // AVR returns -1, SAM returns 0
    }
    const float rise = out_max - out_min;
    const float delta = x - in_min;
    return (delta * rise) / run + out_min;
}

//draw a gauge centered on this coordinate
void menuHandler::drawDialGauge(INIT_CLASS *miniDisp, short x, short y, short radius, int min, int max, int val)
{

	unsigned char degree;
	short x1, y1, x2, y2, tx, ty;
	int i;


	//draw the edge ticks
	short tick_count = 7;
	for(i = 0; i < tick_count; ++i)
	{
		//192 because we want only 3/4 of the gauge to have ticks, +96 for orientation
		degree = 192/(tick_count - 1) * i + 96;

		//do the initial calculation for offset vars
		tx = (radius * gTrig.GetCos(degree)) / 512;
		ty = (radius * gTrig.GetSin(degree)) / 512;

		//apply offsets to coordinates
		x1 = x + tx*3/4;
		y1 = y + ty*3/4;
		x2 = x + tx;
		y2 = y + ty;

		miniDisp->drawLine(x1, y1, x2, y2);
	}
	//draw face
	miniDisp->drawCircle(x, y, radius, U8G2_DRAW_ALL);

	//draw knob
	degree = map(val, min, max, 0, 192) + 96;
	x2 = x + (radius * gTrig.GetCos(degree)) / 512;
	y2 = y + (radius * gTrig.GetSin(degree)) / 512;
	miniDisp->drawLine(x, y, x2, y2);



}

//draw a bar gauge with corner at this coordinate
void menuHandler::drawBarGauge(INIT_CLASS *miniDisp, bool isHorizontal, short x, short y, short width, short height, int min, int max, int val)
{
	
	miniDisp->drawFrame(x, y, width, height);

	//keep value within valid range
	if(val < min)
		val = min;
	else if(val > max)
		val = max;

	//how tall the "filled" box should be
	int fillHeight = map(val, min, max, 0, isHorizontal ? width : height);

	if(isHorizontal)
		miniDisp->drawBox(x, y, fillHeight, height);
	else
		miniDisp->drawBox(x, y + height - fillHeight, width, fillHeight);


}

//draw power HUD
void menuHandler::drawPowerGauge(INIT_CLASS *miniDisp, short x, short y)
{

	//TODO: move this somewhere where it WON't be re-calculated for each draw page
	//convert ADC number into a raw voltage and then step that up according to the voltage divider's values
	float volts = map_float(gPins.voltmeter_read, ADC_VALUE_REF_1, ADC_VALUE_REF_2, ADC_VOLTAGE_REF_1, ADC_VOLTAGE_REF_2) *
					(R1_VAL + R2_VAL) / R2_VAL;


	short gaugeWidth = 8;
	short gaugeHeight = y - DISPLAY_HEIGHT - 8; //8 for font at the top

	//draw numeric voltage
	miniDisp->setFont(GEM_FONT_SMALL);
	char toBePrinted[10];
	sprintf(toBePrinted, "%.2f", volts);
	miniDisp->drawStr(x,y, toBePrinted);

	//testing voltmeter accuracy
	// Serial.print(gPins.voltmeter_read);
	// Serial.print("\t");
	// Serial.print(map_float(gPins.voltmeter_read, 0, ADC_VALUE_REF_2, 0, ADC_VOLTAGE_REF_2));
	// Serial.print("\t");
	// Serial.print(toBePrinted);
	// Serial.print("\t");
	// Serial.println(volts);


	//we need to do a second map here becuase the bar gauge only takes integers
	drawBarGauge(miniDisp, false,
				x + 20 - gaugeWidth, //4 * 5 pixels wide for the numer (4 wide font with 5 digits)
				y + 8, //for font at top
				gaugeWidth,
				gaugeHeight,
				0, 100, (int)map_float(volts, BATTERY_EMPTY, BATTERY_FULL_CHARGE, 0, 100));
}

//draw current selected preset
void menuHandler::drawSelectedPreset(INIT_CLASS *miniDisp, short x, short y, short width, short height,
									const char** labels, unsigned char box_count, unsigned char selected_box)
{
	miniDisp->setFont(GEM_FONT_SMALL);

	short box_width = width / box_count;
	
	for(int i = 0; i < box_count; ++i)
	{
		//miniDisp->getStrWidth(labels[selected_box]);
		miniDisp->drawStr(x + (box_width * i), y + (height / 2 - 3), labels[selected_box]);
		miniDisp->drawFrame(x + (box_width * i), y, box_width, height);

	}

}

