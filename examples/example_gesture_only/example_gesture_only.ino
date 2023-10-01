#include <Wire.h>
#include <CST816_TouchLib.h>

#include "TouchSubscriber.h"

#define PIN_TFT_POWER_ON	15
#define I2C_SDA				18
#define I2C_SCL				17

using namespace MDO;

TouchSubscriber oTouchSubriber;
CST816Touch oTouch;

void setup () {
	Serial.begin(115200);				//so that TouchSubscriber can use that to report

    pinMode(PIN_TFT_POWER_ON, OUTPUT);				//TFT poweron
    digitalWrite(PIN_TFT_POWER_ON, HIGH);	

	Wire.begin(I2C_SDA, I2C_SCL);
	Wire.setClock(400000);	//For reliable communication, it is recommended to use a *maximum* communication rate of 400Kbps	

	if (oTouch.begin(Wire, &oTouchSubriber)) {
		Serial.println("Touch screen initialization done");
	} else {
		Serial.println("Touch screen initialization failed..");
	}
}

void loop () {
	
	unsigned long ulStart = millis();
	while(millis() - ulStart < 20000) {	//Handle gestures for 20 seconds, just for demo
		oTouch.control();
	}
	
	//followed by a deep sleep
	if (oTouch.sleep()) {
		Serial.println("Touch screen is asleep, now for the controller itself");
		Serial.println("Touch the screen to wake up");
		delay(100);
		esp_deep_sleep_start();	//will wake on touch, this being an ESP32 that will result in a setup() again
	} else {
		Serial.println("Initiate a sleep failed");
	}
}
