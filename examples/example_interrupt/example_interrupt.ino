#include <Wire.h>
#include <CST816_TouchLib.h>

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

CST816Touch_SWMode oTouchController;				//the main touch screen controller class from the library used by this example.

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
	Wire.setClock(400000);							//For reliable communication, it is recommended to use a *maximum* communication rate of 400Kbps	

#ifdef USE_T_DISPLAY_S3
	//since this controller was the original base for this library
	//the majority of the defaults are already good
	if (!oTouchController.begin(Wire)) {					//this will initialize & use the TouchScreenEventCache
		Serial.println("Touch screen initialization failed..");
		while(true) {
			delay(100);
		}
	}
	oTouchController.setSwapXY(true);
	//oTouchController.enableDoubleClickFactory_Quick();			//quickest option (no delay), for a double click however it will give a Touch event and a Gesture
	oTouchController.enableDoubleClickFactory_Elegant();			//most elegant option, however this does buffer (read: delay) touch events a bit
	
	//note that this setup uses the chip based gestures.
	//the long press is therefore provided only on release
	//unlike the GestureFactory approach, which is only based on time (so the event is provided when the screen is still touched)
	
#endif
#ifdef USE_T_DISPLAY_S3_AMOLED
	if (!oTouchController.begin(Wire, PIN_TOUCH_INT, -1)) {	//this will initialize & use the TouchScreenEventCache, note: no reset pin for touch screen in T-Display S3 AMOLED..
			Serial.println("Touch screen initialization failed..");
		while(true) {
			delay(100);
		}
	}
	oTouchController.setTouchButtonCoordinates(600, 120);			//the T-Display S3 AMOLED touch button coordinates
	oTouchController.setSwapXY(false);	
	oTouchController.enableGestureFactory(TFT_WIDTH, TFT_HEIGHT);	//enable software based gestures (left, right, up, down, long press)
	//oTouchController.enableDoubleClickFactory_Quick();			//quickest option (no delay), for a double click however it will give a Touch event and a Gesture
	oTouchController.enableDoubleClickFactory_Elegant();			//most elegant option, however this does buffer (read: delay) touch events a bit
#endif

    MDO::CST816Touch::device_type_t eDeviceType;
    if (oTouchController.getDeviceType(eDeviceType)) {
        Serial.print("Found device of type: ");
		Serial.println(CST816Touch::deviceTypeToString(eDeviceType));
    }
		
	Serial.println("Touch screen initialization done");
}

void loop () {
	oTouchController.control();
	
	TouchScreenEventCache* pTouchCache = TouchScreenEventCache::getInstance();
	
	if (pTouchCache->hadTouch()) {
		int x = 0;
		int y = 0;
		pTouchCache->getLastTouchPosition(x, y);	//this 'consumes' the touch from the event cache
		Serial.print("Touch: (");
		Serial.print(x);
		Serial.print(", ");
		Serial.print(y);
		Serial.println(")");
	}
	if (pTouchCache->hadGesture()) {
		TouchScreenController::gesture_t gesture = TouchScreenController::gesture_t::GESTURE_NONE;
		int x = 0;
		int y = 0;
		pTouchCache->getLastGesture(gesture, x, y);	//this 'consumes' the gesture from the event cache
		Serial.print("Gesture: ");
		Serial.print(TouchScreenController::gestureIdToString(gesture));
		if ((gesture == TouchScreenController::gesture_t::GESTURE_DOUBLE_CLICK) || (gesture == TouchScreenController::gesture_t::GESTURE_LONG_PRESS)) {
			Serial.print(", at position (");
			Serial.print(x);
			Serial.print(", ");
			Serial.print(y);
			Serial.print(")");
		}
		Serial.println("");
	}
}
