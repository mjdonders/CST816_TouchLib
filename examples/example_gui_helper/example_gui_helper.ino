#include <Wire.h>
#include <CST816_TouchLib.h>

#include "MyObserver.h"

#define USE_T_DISPLAY_S3_AMOLED		//please uncomment this for the T-Display S3 AMOLED
//#define USE_T_DISPLAY_S3			//please uncomment this for the T-Display S3

#ifdef USE_T_DISPLAY_S3
#warning "Using T-Display S3 configuration"
#define PIN_TFT_POWER_ON	15
#define PIN_I2C_SDA			18
#define PIN_I2C_SCL			17
#define TFT_WIDTH			320
#define TFT_HEIGHT			170
#endif

#ifdef USE_T_DISPLAY_S3_AMOLED
#warning "Using T-Display S3 AMOLED configuration"
#define PIN_TFT_POWER_ON	38
#define PIN_I2C_SDA			3
#define PIN_I2C_SCL			2
#define PIN_TOUCH_INT		21
#define TFT_WIDTH			536
#define TFT_HEIGHT			240
#endif

using namespace MDO;

MyObserver oMyObserver;						    	//local class from example directory
TouchScreenGuiHelper oTouchScreenGuiHelper;			//to help us process GUI events
CST816Touch_SWMode oTouchController;				//the main touch screen controller class from the library used by this example, the hardware based one

void setup () {
	Serial.begin(115200);
	while (!Serial) {
		delay(5);
	}
	Serial.println("");
	Serial.println("Starting");
	
    pinMode(PIN_TFT_POWER_ON, OUTPUT);				//TFT poweron
    digitalWrite(PIN_TFT_POWER_ON, HIGH);	

	Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
	Wire.setClock(400000);	//For reliable communication, it is recommended to use a *maximum* communication rate of 400Kbps	

#ifdef USE_T_DISPLAY_S3
	if (!oTouchController.begin(Wire, &oMyObserver)) {	//this is the ::begin version where we provide the Observer instance
		Serial.println("Touch screen initialization failed..");
		while(true){
			delay(100);
		}
	}
#endif
#ifdef USE_T_DISPLAY_S3_AMOLED
	if (!oTouchController.begin(Wire, &oMyObserver, PIN_TOUCH_INT, -1)) {	//this is the ::begin version where we provide the Observer instance
		Serial.println("Touch screen initialization failed..");
		while(true){
			delay(100);
		}
	}
	oTouchController.setTouchButtonCoordinates(600, 120);			//the T-Display S3 AMOLED touch button coordinates
	oTouchController.setSwapXY(false);
	oTouchController.setInvertY(true, TFT_HEIGHT);					//I prefer lower Y values to be on the lower part of the screen, just like the T-Display S3
	oTouchController.enableGestureFactory(TFT_WIDTH, TFT_HEIGHT);
	oTouchController.enableDoubleClickFactory_Elegant();
	
	//can be used in case the gestures are vertically not the way you like it (see the above setInvertY-call as well)
	//GestureFactory* pGF = oTouchController.getGestureFactory();
	//if (pGF != 0) {
	//	pGF->setInvertVerticalGestures();
	//}
	
#endif
	
	//ensure our GUI helper gets all the required input from the oTouchController
	oTouchController.registerObserver(&oTouchScreenGuiHelper);

	//ensure our Observer is able to update the TouchScreenGuiHelper with the current page
	oMyObserver.begin(&oTouchScreenGuiHelper);

	
	//moved the following to MyObserver::begin
	//oTouchScreenGuiHelper.begin(&oMyObserver);	//ensures that the GuiHelper knows who to notify
	
	//now define our 'user interface elements' (as example)
	oTouchScreenGuiHelper.defineGuiElement(0, TFT_WIDTH/2, 0, TFT_HEIGHT, "Left button", 0);				//page 0	//just two buttons spaced horizontally
	oTouchScreenGuiHelper.defineGuiElement(TFT_WIDTH/2+1, TFT_WIDTH, 0, TFT_HEIGHT, "Right button", 0);		//page 0
	oTouchScreenGuiHelper.defineGuiElement(0, TFT_WIDTH, TFT_HEIGHT/2+1, TFT_HEIGHT, "Upper button", 1);	//page 1	//just two buttons spaced vertically
	oTouchScreenGuiHelper.defineGuiElement(0, TFT_WIDTH, 0, TFT_HEIGHT/2, "Lower button", 1);				//page 1	
	
	Serial.println("Touch screen initialization done");
}

void loop () {
	
	oTouchController.control();

}
