

// #include "configuration.h"
// #include "DispTest.h"
// #include "main.h"

// //INIT_CLASS u8g2(U8G2_R0, U8X8_PIN_NONE , I2C_SCL_PIN, I2C_SDA_PIN);


// short sinTbl[0x100];

// void InitTrigTbl()
// {
//     for (int i = 0; i < 0x100; ++i)
//             sinTbl[i] = (short)(sin(i * TWO_PI / 256.0) * 512.0);
// }

// short GetSin(unsigned char deg)
// {
// 	return sinTbl[deg];
// }

// short GetCos(unsigned char deg)
// {
// 	deg += 0x40;//90 degree offset
// 	return sinTbl[deg];
// }


// //draw a gauge centered on this coordinate
// void drawDialGauge(INIT_CLASS *miniDisp, short x, short y, short radius, int min, int max, int val)
// {

// 	unsigned char degree;
// 	short x1, y1, x2, y2, tx, ty;
// 	int i;


// 	//draw the edge ticks
// 	short tick_count = 7;
// 	for(i = 0; i < tick_count; ++i)
// 	{
// 		//192 because we want only 3/4 of the gauge to have ticks, +96 for orientation
// 		degree = 192/(tick_count - 1) * i + 96;

// 		//do the initial calculation for offset vars
// 		tx = (radius * GetCos(degree)) / 512;
// 		ty = (radius * GetSin(degree)) / 512;

// 		//apply offsets to coordinates
// 		x1 = x + tx*3/4;
// 		y1 = y + ty*3/4;
// 		x2 = x + tx;
// 		y2 = y + ty;

// 		miniDisp->drawLine(x1, y1, x2, y2);
// 	}
// 	//draw face
// 	miniDisp->drawCircle(x, y, radius, U8G2_DRAW_ALL);

// 	//draw knob
// 	degree = map(val, min, max, 0, 192) + 96;
// 	x2 = x + (radius * GetCos(degree)) / 512;
// 	y2 = y + (radius * GetSin(degree)) / 512;
// 	miniDisp->drawLine(x, y, x2, y2);



// }

// //draw a bar gauge with corner at this coordinate
// void drawBarGauge(INIT_CLASS *miniDisp, bool isHorizontal, short x, short y, short width, short height, int min, int max, int val)
// {
	
// 	miniDisp->drawFrame(x, y, width, height);

// 	//how tall the "filled" box should be
// 	int fillHeight = map(val, min, max, 0, isHorizontal ? width : height);

// 	if(isHorizontal)
// 		miniDisp->drawBox(x, y, fillHeight, height);
// 	else
// 		miniDisp->drawBox(x, y + height - fillHeight, width, fillHeight);
// }

// /*
// void setup2()
// {
//     InitTrigTbl();
//     u8g2.begin();
// }

// void loop2()
// {

//     int value = 47;
//     u8g2.firstPage();
// 	do
// 	{
// 		//u8g2.setFont(u8g2_font_helvB12_tr);
// 		//u8g2.drawStr(0,24,"Hello World!");
// 		drawDialGauge(&u8g2, DISPLAY_HEIGHT/2, DISPLAY_HEIGHT/2, 24, 0, 100, value);
// 		drawBarGauge(&u8g2, false, DISPLAY_WIDTH - 32, 4, 16, DISPLAY_HEIGHT - 8, 0, 100, value);
// 	} while (u8g2.nextPage());
// }
// */


// menuHandler::menuHandler(): u8g2(U8G2_R0, U8X8_PIN_NONE , I2C_SCL, I2C_SDA){ }

// void menuHandler::start()
// {
//     InitTrigTbl();
//     u8g2.begin();
// }

// void menuHandler::update()
// {
//     static int value = 47;
//     static int lastValue = 0;


//     value += gPins.encoder_val;
//     if(lastValue != value)
//     {
//         lastValue = value;
//         drawing_screen = true;
//         first_page = true;
//         Serial.println(value);
//     }

//     if(drawing_screen == false)
//         return;



//     if(first_page)
//     {
//         first_page = false;
//         u8g2.firstPage();
//         screenDrawLoop(value);
//     }
//     else if(u8g2.nextPage())
//         screenDrawLoop(value);
//     else
//         drawing_screen = false;


// }

// //do all of the actual screen drawing stuff in here
// void menuHandler::screenDrawLoop(int value)
// {
//     drawDialGauge(&u8g2, DISPLAY_HEIGHT/2, DISPLAY_HEIGHT/2, 24, 0, 100, value/4);
// 	drawBarGauge(&u8g2, false, DISPLAY_WIDTH - 32, 4, 16, DISPLAY_HEIGHT - 8, 0, 100, value/4);
// }