#include <Arduino.h>

#include "main.h"
#include "configuration.h"
#include "settings.h"
#include "input.h"
#include "utilities.h"
#include "flywheels.h"
#include "pusher.h"
#include "display.h"
#include "connectome.h"

all_settings_t gSettings;
sharedSettingsHandler gSettingsShare;
inputHandler gPins;
flywheelHandler gFlywheel;
pusherHandler gPusher;
buzzerHandler gBuzzer(BUZZER);
trigHandler gTrig; //trigonometry
connectomeHandler gConnectome;

//serialHandler gInfo(DEBUG_BAUD_RATE);
//chronyHandler gChronograph;

//timer inturrupts to handle input buttons (actually just the encoder)
hw_timer_t *timer = NULL;
void IRAM_ATTR handleLoop()
{
	//Serial.println("inter");
    gPins.update_encoder();
    gBuzzer.update();
}



//handle the main processes
void backend_task(void * parameters)
{
	TickType_t xLastWakeTime;
    const TickType_t xFrequency = 1 / portTICK_PERIOD_MS;
    BaseType_t xWasDelayed;


    xLastWakeTime = xTaskGetTickCount();
    for( ;; )
    {
		//we could also use vTaskDelay here if we don't care about the return value
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, xFrequency );

		//unique stuff:
		//Serial.printf("task ran at %d\n", xLastWakeTime);

		gPins.update();

		gConnectome.update();

		gFlywheel.update();
		gPusher.update();

    }
}

//handle drawing the screen
void screen_task(void *parameters)
{
	TickType_t xLastWakeTime;
    const TickType_t xFrequency = 1 / portTICK_PERIOD_MS;
    BaseType_t xWasDelayed;

	menuHandler tMenu;
	tMenu.start();

    xLastWakeTime = xTaskGetTickCount();
    for( ;; )
    {
		//we could also use vTaskDelay here if we don't care about the return value
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, xFrequency );

		//unique stuff:
		tMenu.update();

    }
}


all_settings_t theseSettings;
all_settings_t outSettings;

void setup() {
    // put your setup code here, to run once:


	//gInfo.send_message("weee");

	gSettingsShare.start();

    //create inturrupt for encoder
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &handleLoop, true);
    timerAlarmWrite(timer, 1000, true); // every 0.01 seconds = 1000
    timerAlarmEnable(timer);

	//start freeRTOS tasks
	{
		TaskHandle_t backend_task_handle;
		xTaskCreate(
			backend_task,    // Function that should be called
			"Backend Task", // Name of the task (for debugging)
			10000,     // Stack size (bytes)
			NULL,     // Parameter to pass
			2,        // Task priority
			&backend_task_handle      // Task handle
			);
		TaskHandle_t screen_task_handle;
		xTaskCreate(
			screen_task,    // Function that should be called
			"Screen Task", // Name of the task (for debugging)
			10000,     // Stack size (bytes)
			NULL,     // Parameter to pass
			1,        // Task priority
			&screen_task_handle      // Task handle
			);

		
	}

    //gInfo.send_message("weee");
    //gBuzzer.beep_multiple(10000, 30000, 10);

	//test
	Serial.begin(DEBUG_BAUD_RATE);

	



}

void loop() {
    // put your main code here, to run repeatedly:

    // gPins.update();
    // gMenu.update();

    // gConnectome.update();

	// gFlywheel.update();
	// gPusher.update();



	//test
	//I came head-first with this error. The fix was a single line, as mentioned in the link.
	//note https://github.com/espressif/esp-idf/issues/11867
	//we're getting occasional buffer overruns from bidirectional RMT.
	//see this: https://discord.com/channels/727038380054937610/1050619108007350343/1168263597172981810

	// unsigned long spd;
	// unsigned int cnt;

	// static unsigned long bggst_spd = 99999999;

	// gChronograph.get_darts(&spd, &cnt);

	// if(spd == 0)
	// 	spd = 1;
	
	// if(spd != 1 && spd < bggst_spd)
	// 	bggst_spd = spd;


	// float speed_fps = DART_LENGTH / (float)(spd) / 25.4 / 12 * 1E+6;               // feet per second
    // float speed_mps = DART_LENGTH / (float)(spd) / 1000 * 1E+6;                    // meter per second
    // float speed_mph = DART_LENGTH / (float)(spd) / 25.4 / 12 * 1E+6 / 5280 * 3600; // miles per hour  

	// Serial.printf("DC: %d || SPD: %.2f || SPD_MAX %d\n", cnt, speed_fps, bggst_spd);

	sleep(1);


	//this is a test
	// if (millis() % 1000 == 0)
	// {
	// 	theseSettings.preset_hold_time += 1;
	// 	gSettingsShare.sendSettings(&theseSettings);
	// 	theseSettings.preset_hold_time += 1;

	// 	gSettingsShare.getSettings(&outSettings);
	// 	Serial.printf("in: %d || out: %d\n",theseSettings.preset_hold_time, outSettings.preset_hold_time);

	// }




}

