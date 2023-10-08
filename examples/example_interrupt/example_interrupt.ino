#include <Wire.h>
#include <CST816_TouchLib.h>

#define PIN_TFT_POWER_ON	15
#define I2C_SDA				18
#define I2C_SCL				17

using namespace MDO;

DummyTouchSubscriber oTouchSubriber;
CST816Touch oTouch;

void setup () {
	Serial.begin(115200);				//so that DummyTouchSubscriber can use that to report

    pinMode(PIN_TFT_POWER_ON, OUTPUT);				//TFT poweron
    digitalWrite(PIN_TFT_POWER_ON, HIGH);
	
	Wire.begin(I2C_SDA, I2C_SCL);
	Wire.setClock(400000);	//For reliable communication, it is recommended to use a *maximum* communication rate of 400Kbps	
	
	if (!oTouch.begin(Wire, &oTouchSubriber)) {
		Serial.println("Touch screen initialization failed..");
		while(true){
			delay(100);
		}
	}
	
//	if (!oTouch.setOperatingModeHardwareBased()) {
//		Serial.println("Set full hardware operational mode failed");
//	}

//	if (oTouch.setNotifyOnMovement()) {
//		oTouch.setMovementInterval(100);	//as example: limit to 10 per second (so 100 msec as interval)
//	} else {
//		//only available in 'fast'-mode
//		Serial.println("Set notify on movement failed");
//	}
	
//	if (!oTouch.setNotificationsOnAllEvents()) {
//		//only available in 'fast'-mode, provides events on touch and release of the screen
//		Serial.println("Set notify on touch-and-release failed");
//	}

	CST816Touch::device_type_t eDeviceType;
	if (oTouch.getDeviceType(eDeviceType)) {
		Serial.print("Device is of type: ");
		Serial.println(CST816Touch::deviceTypeToString(eDeviceType));
	}
		
	Serial.println("Touch screen initialization done");
}

void loop () {
	oTouch.control();
}
