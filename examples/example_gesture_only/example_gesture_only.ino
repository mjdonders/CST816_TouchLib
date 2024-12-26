#include <Wire.h>
#include <CST816_TouchLib.h>

#include "MyTouchObserver.h"

//#define USE_T_DISPLAY_S3_AMOLED	//please uncomment this for the T-Display S3 AMOLED
#define USE_T_DISPLAY_S3			//please uncomment this for the T-Display S3

#ifdef USE_T_DISPLAY_S3
#warning "Using T-Display S3 configuration"
#define PIN_TFT_POWER_ON	15
#define PIN_I2C_SDA			18
#define PIN_I2C_SCL			17
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

MyTouchObserver oTouchObserver;						//local class from example directory
CST816Touch_HWMode oTouchController;				//the main touch screen controller class from the library used by this example, the hardware based one

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
	if (!oTouchController.begin(Wire, &oTouchObserver)) {	//this is the ::begin version where we provide the Observer instance
		Serial.println("Touch screen initialization failed..");
		while(true){
			delay(100);
		}
	}
#endif
#ifdef USE_T_DISPLAY_S3_AMOLED
	Serial.println("**Note that this does not work in my setup..**");				//please revert to the CST816Touch_SWMode implementation
	if (!oTouchController.begin(Wire, &oTouchObserver, PIN_TOUCH_INT, -1)) {	//this is the ::begin version where we provide the Observer instance
		Serial.println("Touch screen initialization failed..");
		while(true){
			delay(100);
		}
	}
	oTouchController.setTouchButtonCoordinates(600, 120);			//the T-Display S3 AMOLED touch button coordinates
	oTouchController.setSwapXY(false);	
	
#endif

	TouchScreenController::device_type_t eDeviceType = TouchScreenController::device_type_t::DEVICE_UNKNOWN;
	if (oTouchController.getDeviceType(eDeviceType)) {
		Serial.print("Device is of type: ");
		Serial.println(oTouchController.deviceTypeToString(eDeviceType));
	}
	
	Serial.println("Touch screen initialization done");
}

bool sleepScreenAndESP32() {
	if (!oTouchController.sleep()) {
		return false;
	}
	Serial.println("Touch screen is asleep, now for the controller itself");
	Serial.println("Touch the screen to wake up");
	delay(100);
	esp_deep_sleep_start();	//will wake on touch, this being an ESP32 that will result in a setup() again
	
	return true;	//note that this line will never-ever be executed
}

void loop () {
	
	unsigned long ulStart = millis();
	while(millis() - ulStart < 60000) {	//Handle gestures for 60 seconds, just for demo
		oTouchController.control();
	}
	
	//followed by a deep sleep
	if (!sleepScreenAndESP32()) {
		Serial.println("Initiate a sleep failed, the touch chip was most likely sleeping");
		if (oTouchController.resetChip(true)) {
			Serial.println("Yes, most likely. The chip is awake and state is restored by now");
			if (!sleepScreenAndESP32()) {
				Serial.println("Initiate a sleep failed permanently..");
			}
		}
	}
}
