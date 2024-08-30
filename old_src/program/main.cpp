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
inputHandler gPins;
flywheelHandler gFlywheel;
pusherHandler gPusher;
serialHandler gInfo(DEBUG_BAUD_RATE);
buzzerHandler gBuzzer(BUZZER);
menuHandler gMenu;
trigHandler gTrig; //trigonometry
connectomeHandler gConnectome;

//timer inturrupts to handle input buttons (actually just the encoder)
hw_timer_t *timer = NULL;
void IRAM_ATTR handleLoop()
{
    gPins.update_encoder();
    gBuzzer.update();
}


void setup() {
    // put your setup code here, to run once:

    //delay(1000);

    gMenu.start();

    //create inturrupt
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &handleLoop, true);
    timerAlarmWrite(timer, 1000, true); // every 0.01 seconds
    timerAlarmEnable(timer);

    //gInfo.send_message("weee");
    //gBuzzer.beep_multiple(10000, 30000, 10);

}

void loop() {
    // put your main code here, to run repeatedly:

    gPins.update();
    gMenu.update();

    gConnectome.update();

	gFlywheel.update();
	gPusher.update();


}

