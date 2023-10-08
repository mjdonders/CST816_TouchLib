#include <Wire.h>
#include <CST816_TouchLib.h>

#define PIN_TFT_POWER_ON	15
#define I2C_SDA				18
#define I2C_SCL				17

using namespace MDO;

CST816Touch oTouch;

void setup () {
	Serial.begin(115200);

	pinMode(PIN_TFT_POWER_ON, OUTPUT);				//TFT poweron
	digitalWrite(PIN_TFT_POWER_ON, HIGH);	

	Wire.begin(I2C_SDA, I2C_SCL);
	Wire.setClock(400000);	//For reliable communication, it is recommended to use a *maximum* communication rate of 400Kbps

	if (!oTouch.begin(Wire)) {
		Serial.println("Touch screen initialization failed..");
		while(true){
			delay(100);
		}
	}

	if (!oTouch.setOperatingModeHardwareBased()) {
		Serial.println("Set full hardware operational mode failed");
	}

//	if (oTouch.setNotifyOnMovement()) {
//		oTouch.setMovementInterval(100);	//as example: limit to 10 per second (so 100 msec as interval)
//	} else {
//		//only available in 'fast'-mode
//		Serial.println("Set notify on movement failed");
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
	
	if (oTouch.hadTouch()) {
		int x = 0;
		int y = 0;
		oTouch.getLastTouchPosition(x, y);
		
		Serial.print("Touch received at: (");
		Serial.print(x);
		Serial.print(",");
		Serial.print(y);
		Serial.println(")");
	}
	
	if (oTouch.hadGesture()) {	//note that a gesture typically starts with a touch. Both will be provided here.
		CST816Touch::gesture_t eGesture;
		int x = 0;
		int y = 0;
		oTouch.getLastGesture(eGesture, x, y);
		
		Serial.print("Gesture (");
		Serial.print(CST816Touch::gestureIdToString(eGesture));
		Serial.print(") received at: (");
		Serial.print(x);
		Serial.print(",");
		Serial.print(y);
		Serial.println(")");
	}	
}
