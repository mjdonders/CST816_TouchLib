#include <arduino.h>
#include <Wire.h>

#include <CST816_TouchLib.h>

#define PIN_TFT_POWER_ON                 15

using namespace MDO;

DummyTouchSubscriber oTouchSubriber;
CST816Touch oTouch;

void setup () {
	Serial.begin(115200);				//so that DummyTouchSubscriber can use that to report

    pinMode(PIN_TFT_POWER_ON, OUTPUT);				//TFT poweron
    digitalWrite(PIN_TFT_POWER_ON, HIGH);	
	
	if (oTouch.init(Wire, &oTouchSubriber)) {
		Serial.println("Touch screen initialization done");
	} else {
		Serial.println("Touch screen initialization failed..");
	}
}

void loop () {
	oTouch.control();
}

