#include "CST816Touch.h"
#include "TouchSubscriberInterface.h"

//if you like, enable this for debug serial messages
//I tried this from the INO file, but could not get that to work..
//#define CST816_TOUCH_LIB_DEBUG
//#include <mdomisc.h>

//avoid the defines, as per https://docs.arduino.cc/learn/contributions/arduino-writing-style-guide#variables
const uint8_t TOUCH_CMD_SLEEP 			= 0x03;
const uint8_t TOUCH_REGISTER_SLEEP		= 0xA5;
const uint8_t TOUCH_REGISTER_WORK		= 0x00;
const uint8_t TOUCH_REGISTER_GESTURE	= 0x01;	//defined by reverse engineering..
const uint8_t TOUCH_REGISTER_NUMBER   	= 0x02;

const uint8_t TOUCH_REGISTER_FW_VERSION_LOW		= 0xA6;
const uint8_t TOUCH_REGISTER_FW_VERSION_HIGH	= 0xA7;

const uint8_t  I2C_OK = 0;

const int TOUCH_BUTTON_X = 360;
const int TOUCH_BUTTON_Y =  85;

#ifdef CST816_TOUCH_LIB_DEBUG
#warning Just a message to note that CST816Touch debug messages are enabled
#endif

namespace MDO {
	
bool	g_bTouchInterrupt = false;

/**
 * Private: Determines the difference in time between two millis calls, assuming to be 'short'.
 * (where short < half the overflow time, so roughly 25 days
 */
unsigned long CST816Touch::millisDiff(const unsigned long& ulStart, const unsigned long& ulEnd) {
	unsigned long ulDiff = 0;
	if (ulStart <= ulEnd) {
		//normal easy scenario
		ulDiff = ulEnd - ulStart;
	} else {
		//an overflow has occured
		ulDiff = ulEnd + (ULONG_MAX - ulStart);
	}

	return ulDiff;
}

/**
 * Private: Determines the difference in time between two millis calls, assuming to be 'short'.
 * (where short < half the overflow time, so roughly 25 days
 */
unsigned long CST816Touch::millisDiff(const unsigned long& ulStart) {
	return millisDiff(ulStart, millis());
}

/**
 * Private Converts the touch coordinates.
 * Note that this assumes the USB of the LILYGO T-Display ESP32-S3 to be on the right.
 * Assuming that, x=y=0 == lower left
 * note2: the touch-button is (TOUCH_BUTTON_X,TOUCH_BUTTON_Y) in this case
 */
bool CST816Touch::parseTouchEvent(int& x, int& y, bool& currentlyPressed) {
	
	const uint8_t TOUCH_XH = 3;		//X_position[11:8]		BIT 7 ~ BIT 6: event_flg
	const uint8_t TOUCH_XL = 4;		//X_position[7:0]
	
	x = (m_ucBuffer[TOUCH_XH] & 0x0F) << 8;
	x += m_ucBuffer[TOUCH_XL];
	
	const uint8_t TOUCH_YH = 5;		//Y_position[11:8]		BIT 7 ~ BIT 4: touch_ID[3:0]
	const uint8_t TOUCH_YL = 6;		//Y_position[7:0]
	
	y = (m_ucBuffer[TOUCH_YH] & 0x0F) << 8;
	y += m_ucBuffer[TOUCH_YL];

	int iEventFlag = (m_ucBuffer[TOUCH_XH] & 0xC0) >> 6;	//BIT 7 ~ BIT 6: event_flg from TOUCH_XH
	currentlyPressed = iEventFlag == 2;

#ifdef CST816_TOUCH_LIB_DEBUG	
	//Serial.print("Event flag: ");
	//Serial.println(iEventFlag);
	
	int iTouchId = (m_ucBuffer[TOUCH_YH] & 0xF0) >> 4;
	if (iTouchId != 0) {				//seems to always be 0
		Serial.print("Touch ID: ");
		Serial.println(iTouchId);
	}
#endif
	
	int swapToBeUseful = x;	//...
	x = y;
	y = swapToBeUseful;
	
	return true;
}

/**
 * Private: write to a register
 */
bool CST816Touch::writeRegister(uint8_t ucRegister, uint8_t ucByteToWrite) {
	bool bOk = false;
	
	if (m_pI2C != 0) {
		m_pI2C->beginTransmission(m_ucAddress);
		m_pI2C->write(ucRegister);
		m_pI2C->write(ucByteToWrite);
		
		int iRet = m_pI2C->endTransmission();
		bOk = iRet == I2C_OK;

#ifdef CST816_TOUCH_LIB_DEBUG
		if (!bOk) {
			Serial.print("Write command failed, reply: ");
			Serial.println(iRet);
		}
#endif	

	}
	return bOk;
}

/**
 * Private: read from a register
 */
bool CST816Touch::readRegister(uint8_t ucRegister, uint8_t iRequestedSize) {
	bool bOk = false;
	
	m_ucBuffer[0] = 0;
	
	if ((iRequestedSize <= sizeof(m_ucBuffer)) && (m_pI2C != 0)) {
		m_pI2C->beginTransmission(m_ucAddress);
		m_pI2C->write(ucRegister);

		bOk =	(m_pI2C->endTransmission() == I2C_OK) && 
				(m_pI2C->requestFrom(m_ucAddress, iRequestedSize) > 0);
		
		if (bOk) {
			bOk = m_pI2C->readBytes(&m_ucBuffer[0], iRequestedSize) >= iRequestedSize;
		}
	}
	
	return bOk;
}

/**
 * Private: reads the 'standard' CST816 register
 */
bool CST816Touch::read() { 
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
    return 	(readRegister((uint8_t)TOUCH_REGISTER_WORK, sizeof(m_ucBuffer))) &&
			(m_ucBuffer[TOUCH_REGISTER_NUMBER] > 0);	//in reality, this will always be 1 at a proper read
}

/**
 * Private: did we just had a touch event?
 * Either handles a interrupt from 'just now', or actively checks this
 */
bool CST816Touch::hadPhysicalEvent() {
	bool bTouchHappened = false;
	
	if (g_bTouchInterrupt) {
		g_bTouchInterrupt = false;
		bTouchHappened = read();	
	}
	
	if (m_iPIN_INTERRUPT == -1) {
		//non-interrupt mode..
		bTouchHappened = read();
	}
	return bTouchHappened;
}

/**
 * Private: handle a gesture
 */
void CST816Touch::handleGesture() {
	
	gesture_t eGesture = (gesture_t)m_ucBuffer[TOUCH_REGISTER_GESTURE];
	const int iGestureFilterDelayTime = 100;

	int x = 0;
	int y = 0;
	bool currentlyPressed = false;
	bool bCoordinatesAreValid = parseTouchEvent(x, y, currentlyPressed);
	if (bCoordinatesAreValid) {
		m_iLastGestureX = x;
		m_iLastGestureY = y;
	}

	
	//specific handling for the long press..
	if (eGesture == GESTURE_LONG_PRESS) {
		m_ulLastLongPress = millis();
		
	} else if (eGesture == GESTURE_TOUCH_BUTTON) {
		//also this requires some specifics.. :-(
		//sometimes this is triggered when you're just touching the center of the screen
		//filter that by an additional position check
		if (bCoordinatesAreValid) {
			if ((x == TOUCH_BUTTON_X) && (y == TOUCH_BUTTON_Y)) {
				
				if (millisDiff(m_ulLastGesture) > iGestureFilterDelayTime) {
					m_eLastGesture = eGesture;
					if (m_pTouchSubscriber != 0) {
						m_pTouchSubscriber->gestureNotification(this, m_eLastGesture);
					}
				}
				m_ulLastGesture = millis();
			}
		}
	} else {
#ifdef CST816_TOUCH_LIB_DEBUG
		if ((eGesture != GESTURE_RIGHT) && (eGesture != GESTURE_LEFT) && (eGesture != GESTURE_DOWN) && (eGesture != GESTURE_UP) && (eGesture != GESTURE_TOUCH_BUTTON) && (eGesture != GESTURE_LONG_PRESS))  {
			Serial.println("Unkown gesture: ");
			printBuf();
		}
#endif
		if (millisDiff(m_ulLastGesture) > iGestureFilterDelayTime) {
			m_eLastGesture = eGesture;
			if (m_pTouchSubscriber != 0) {
				m_pTouchSubscriber->gestureNotification(this, m_eLastGesture);
			}
			m_ulLastGesture = millis();
		}
	}
}

/**
 * Private: Handle a touch event
 */
void CST816Touch::handleTouch() {
	
	if (millisDiff(m_ulLastTouch) > 50) {
		int x=0;
		int y=0;
		bool currentlyPressed = false;
		if (parseTouchEvent(x, y, currentlyPressed)) {
			if (!currentlyPressed) {
				//notify on release only
								
				if ((x == TOUCH_BUTTON_X) && (y == TOUCH_BUTTON_Y)) {
					m_eLastGesture = GESTURE_TOUCH_BUTTON;
					m_iLastGestureX = x;
					m_iLastGestureY = y;

					//sometime a touch on the touch-button is not registered as such.. Handle this here for a more consitent interfase.
					if (m_pTouchSubscriber != 0) {
						m_pTouchSubscriber->gestureNotification(this, m_eLastGesture);
					}
				} else {
					m_iLastTouchX = x;
					m_iLastTouchY = y;
					if (m_pTouchSubscriber != 0) {
						m_pTouchSubscriber->touchNotification(this, m_iLastTouchX, m_iLastTouchY);
					}
				}
			}
		}
		m_ulLastTouch = millis();
	}
}

/**
 * Convert a gesture id (gesture_t) to a String
 */
/*static*/ String CST816Touch::gestureIdToString(int iGestureId) {
	if (iGestureId > 0) {
		gesture_t eGesture((gesture_t)iGestureId);
		switch (eGesture) {
			case GESTURE_LEFT:			return "LEFT";
			case GESTURE_RIGHT:			return "RIGHT";
			case GESTURE_UP:			return "UP";
			case GESTURE_DOWN:			return "DOWN";
			case GESTURE_TOUCH_BUTTON:	return "TOUCH_BUTTON";
			case GESTURE_LONG_PRESS:	return "LONG_PRESS";
		}
	}
	return String("UNKNOWN: ") + String(iGestureId);
}

/**
 * Get the position of the last touch.
 * Note: 'consumes the last touch', meaning it will reset this info, to minimize the chance of a dual processing of a single event
 * When x=y=0, please assume no touch ever happened.
 */
void CST816Touch::getLastTouchPosition(int& x, int& y) {
	x = m_iLastTouchX;
	y = m_iLastTouchY;
	
	m_iLastTouchX = 0;
	m_iLastTouchY = 0;
}

/**
 * Get the position of the last gesture.
 * Note: 'consumes the last gesture', meaning it will reset this info, to minimize the chance of a dual processing of a single event
 * When x=y=0, please assume no gesture ever happened.
 */
void CST816Touch::getLastGesture(gesture_t& gesture, int& x, int& y) {
	x = m_iLastGestureX;
	y = m_iLastGestureY;
	gesture = m_eLastGesture;
	
	m_eLastGesture	= GESTURE_NONE;
	m_iLastGestureX = 0;
	m_iLastGestureY = 0;
}

bool CST816Touch::hadTouch() const {
	return (m_iLastTouchX != 0) && (m_iLastTouchY != 0);
}

bool CST816Touch::hadGesture() const {
	return m_eLastGesture != GESTURE_NONE;
}

/**
 * Gets the firmware version.
 * Can be used to check CST816 availability
 */
bool CST816Touch::getFirmwareVersion(unsigned int& uiFW) {
	bool bOk = false;
	uiFW = 0;
	
	setChipInDynamicMode();
	
#ifdef CST816_TOUCH_LIB_DEBUG
	Serial.println("Checking touch firmware..");
#endif	
	if (readRegister(TOUCH_REGISTER_FW_VERSION_HIGH, 1)) {
		uiFW = m_ucBuffer[0];
		uiFW <<= 8;
		
		if (readRegister(TOUCH_REGISTER_FW_VERSION_LOW, 1)) { 
			uiFW |= m_ucBuffer[0];
			bOk = true;
		}
	} else {
#ifdef CST816_TOUCH_LIB_DEBUG
		Serial.println("Checking touch firmware failed");
#endif
	}
	
	return bOk;
}

/**
 * Set the CST816 chip in 'dynamic mode', meaning the mode in which it listens to our commands.
 * Internally used in Sleep and GetFirmwareVersion, so should not be needed to externally, typically.
 */
void CST816Touch::setChipInDynamicMode() {
	if (m_iPIN_RESET != -1) {
		digitalWrite(m_iPIN_RESET, LOW);	//this is the actual reset
		delay(20);							//arbitrary-but-tested value..
		digitalWrite(m_iPIN_RESET, HIGH);	
		delay(100);							//arbitrary-but-tested value..
	}
}

/**
 * Set the CST816 chip in sleep mode
 */
bool CST816Touch::sleep() {

	//ensure that the chip listenes to our command first..
	//feels a bit contradictionay though.
	setChipInDynamicMode();
	
	bool bOk = writeRegister((uint8_t)TOUCH_REGISTER_SLEEP, (uint8_t)TOUCH_CMD_SLEEP);

#ifdef CST816_TOUCH_LIB_DEBUG
	if (!bOk) {
		Serial.println("Sleep request failed..");
	}
#endif
	
	if (m_iPIN_RESET != -1) {
		/*
		//from https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
		However, on ESP32/S2/C3/S3/C2, this function cannot be used to hold the state of a digital GPIO during Deep-sleep. 
		Even if this function is enabled, the digital GPIO will be reset to its default state when the chip wakes up from Deep-sleep. 
		If you want to hold the state of a digital GPIO during Deep-sleep, please call gpio_deep_sleep_hold_en.
		*/
		//gpio_hold_en((gpio_num_t)m_iPIN_RESET);
		gpio_deep_sleep_hold_en();
	}
	if (m_iPIN_INTERRUPT != -1) {
		esp_sleep_enable_ext0_wakeup((gpio_num_t)m_iPIN_INTERRUPT, 0);
	}
	
	return bOk;
}

/**
 * Debug method only: print internal buffer.
 * Is useful only after a Read, typically.
 */
void CST816Touch::printBuf() {
	for (int i=0; i< sizeof(m_ucBuffer); i++) {
		if (i !=0) {
			Serial.print(", ");
		}
		uint8_t c = m_ucBuffer[i];
		Serial.print("0x");
		Serial.print(c < 16 ? "0" : "");
		Serial.print(c, HEX);
	}
	Serial.println();
}

/**
 * De-initialize this classs.
 */
void CST816Touch::deInit() {
	if (m_pI2C != 0) {
		m_pI2C->end();
	}
	m_pI2C = 0;
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
	m_pTouchSubscriber = 0;
}

/**
 * Periodically call this, from *loop* typically.
 * The return value *can* optionally be checked if a touch/gesture has happened, also see hadTouch & hadGesture for 'the same'.
 * All of this return value checking, hadTouch & hadGesture are not needed when the callback mechanism is used.
 */
bool CST816Touch::control() {
	bool bTouchHappened = hadPhysicalEvent();
		
	if (bTouchHappened) {
#ifdef CST816_TOUCH_LIB_DEBUG
		if (m_pTouchSubscriber == 0) {
			Serial.println("Touch event");
		}
		//PrintBuf();
#endif
				
		//check gesture first
		if (m_ucBuffer[TOUCH_REGISTER_GESTURE] != 0x00) {
			handleGesture();
		} else {
			handleTouch();
		}
		//ensure we never re-parse the same
		memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
	} else {
		//no touch happened just now
		//
		//however, a long press is only indicate by the press, not the release..
		//check if that was the case
		if ((m_ulLastLongPress != 0) && (millisDiff(m_ulLastLongPress) > 50)) {
			m_eLastGesture = GESTURE_LONG_PRESS;
			if (m_pTouchSubscriber != 0) {
				m_pTouchSubscriber->gestureNotification(this, m_eLastGesture);
			}
			m_ulLastLongPress = 0;
		}	
	}
	
	return bTouchHappened;
}

/**
 * Initialize: this is a mandatory call to be made 
 * For each PIN which this class should not use / change: set to -1
 * return false on issues
 */
bool CST816Touch::init(TwoWire& w, TouchSubscriberInterface* pTouchSubscriber /*= 0*/, 
							int PIN_INTERRUPT /*= 16*/, 
							int I2C_SDA /*= 18*/, 
							int I2C_SCL /*= 17*/, 
							uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/, 							
							int PIN_RESET /*= 21*/) {	//note that this is a mandatory PIN, so don't set this to -1
	m_pI2C = &w;
	bool bOk = false;
	if (m_pI2C != 0) {
		m_pI2C->begin(I2C_SDA, I2C_SCL);
		m_pI2C->setClock(400000);	//For reliable communication, it is recommended to use a maximum communication rate of 400Kbps
		bOk = true;		
	}
		
	m_ucAddress = CTS816S_I2C_ADDRESS;
	m_iPIN_RESET = PIN_RESET;
	m_iPIN_INTERRUPT = PIN_INTERRUPT;
	m_pTouchSubscriber = pTouchSubscriber;
	
	if (bOk) {
		bOk = m_iPIN_RESET != -1;
	}
	
	if (bOk) {
#ifdef CST816_TOUCH_LIB_DEBUG
		Serial.println("Ensure touch screen is properly reset");
#endif			
		//init PGIO, if required
		//see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
		gpio_hold_dis((gpio_num_t)m_iPIN_RESET);
		pinMode(m_iPIN_RESET, OUTPUT);
		digitalWrite(m_iPIN_RESET, LOW);
		delay(200);
		digitalWrite(m_iPIN_RESET, HIGH);	
		delay(200);
	}
	
	if (bOk) {
		unsigned int uiFW = 0;
		bOk = getFirmwareVersion(uiFW);
#ifdef CST816_TOUCH_LIB_DEBUG		
		Serial.print("Firmware: ");
		Serial.println(uiFW, HEX);
#endif
	}
	
	
	if (m_iPIN_INTERRUPT != -1) {
#ifdef CST816_TOUCH_LIB_DEBUG
		Serial.println("Configuring touch interrupt");
#endif		
	    attachInterrupt(
			m_iPIN_INTERRUPT, [] { g_bTouchInterrupt = true; }, FALLING);
	}

	return bOk;
}
		

CST816Touch::CST816Touch() {
	m_pI2C = 0;
	m_iPIN_RESET = -1;
	m_iPIN_INTERRUPT = -1;
	m_uiCTS816S_I2C_ADDRESS = 0;
	
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
	m_pTouchSubscriber = 0;
	
	m_ulLastLongPress = 0;
	
	m_ulLastGesture = 0;
	m_eLastGesture	= GESTURE_NONE;
	m_iLastGestureX = 0;
	m_iLastGestureY = 0;
	
	m_ulLastTouch = 0;
	m_iLastTouchX = 0;
	m_iLastTouchY = 0;
}

CST816Touch::~CST816Touch() {
	deInit();
}

}	//namespace end