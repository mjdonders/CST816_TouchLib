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

	if (oTouch.begin(Wire)) {
		Serial.println("Touch screen initialization done");
	} else {
		Serial.println("Touch screen initialization failed..");
	}
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
