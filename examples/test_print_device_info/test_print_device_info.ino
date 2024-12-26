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
//#define TFT_WIDTH			536
//#define TFT_HEIGHT			240
#endif

using namespace MDO;

CST816Touch_SWMode oTouchController;

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

	if (!oTouchController.begin(Wire)) {
		Serial.println("Touch screen initialization failed..");
		//nevertheless, continue for now..
	}
			
	TouchScreenController::device_type_t eDeviceType = TouchScreenController::device_type_t::DEVICE_UNKNOWN;
	oTouchController.getDeviceType(eDeviceType);
	Serial.print("Device is of type: ");
	Serial.println(oTouchController.deviceTypeToString(eDeviceType));
	
	unsigned char ucFW = 0;
	if (oTouchController.getFirmwareVersion(ucFW)) {
		Serial.print("Firmware: ");
		Serial.println((int)ucFW);
	}
	
	Serial.println("");
	
	Serial.println("Halting program.");
	while (true) {
		delay(1000);
	}
}


void loop () {
}
