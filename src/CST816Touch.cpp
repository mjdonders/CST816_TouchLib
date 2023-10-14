#include "CST816Touch.h"
#include "TouchSubscriberInterface.h"
#include <limits.h>

//if you like, enable this for debug serial messages
//I tried this from the INO file, but could not get that to work..
//#define CST816_TOUCH_LIB_DEBUG

#ifdef CST816_TOUCH_LIB_DEBUG
#define CST816_TOUCH_DEBUG_PRINT(str) Serial.print(str)
#else
#define CST816_TOUCH_DEBUG_PRINT(str)
#endif

#ifdef CST816_TOUCH_LIB_DEBUG
#define CST816_TOUCH_DEBUG_PRINTLN(str) Serial.println(str)
#else 
#define CST816_TOUCH_DEBUG_PRINTLN(str)
#endif

//avoid the defines, as per https://docs.arduino.cc/learn/contributions/arduino-writing-style-guide#variables
const uint8_t TOUCH_CMD_SLEEP 			= 0x03;
const uint8_t TOUCH_REGISTER_SLEEP		= 0xA5;
const uint8_t TOUCH_REGISTER_WORK		= 0x00;
const uint8_t TOUCH_INDEX_GESTURE		= 0x01;	//defined by reverse engineering..
const uint8_t TOUCH_REGISTER_NUMBER   	= 0x02;

const uint8_t TOUCH_IRQ_EN_TOUCH		= 0x40;	//gives a lot of events, in itself only provide touch (TOUCH_CONTACT) info. also include gesture info
const uint8_t TOUCH_IRQ_EN_CHANGE		= 0x20;	//gives or adds the release (TOUCH_UP) info
const uint8_t TOUCH_IRQ_EN_MOTION		= 0x10;	//seems to add the GESTURE_TOUCH_BUTTON events, add long press-while-still-touched gestures
const uint8_t TOUCH_IRQ_EN_LONGPRESS	= 0x01;	//seems to do nothing..?

const uint8_t MOTION_MASK_CONTINUOUS_LEFT_RIGHT	= 0x04;
const uint8_t MOTION_MASK_CONTINUOUS_UP_DOWN	= 0x02;
const uint8_t MOTION_MASK_DOUBLE_CLICK			= 0x01;	//add hardware based double click

const uint8_t TOUCH_REGISTER_VERSION		= 0x15;
const uint8_t TOUCH_REGISTER_CHIP_ID		= 0xA7;		//chip ID
const uint8_t TOUCH_REGISTER_FW_VERSION		= 0xA9;		//firmware version
const uint8_t TOUCH_REGISTER_MOTION_MASK	= 0xEC;		//motion mask register
const uint8_t TOUCH_REGISTER_IRQ_CTL		= 0xFA;		//interrupt control
const uint8_t TOUCH_REGISTER_AUTOSLEEP		= 0xFE;		//can be used to disable auto sleep, might be quicker (?) but will consume more power

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
 * (where short < half the overflow time, so roughly 25 days)
 */
unsigned long CST816Touch::millisDiff(const unsigned long& ulStart, const unsigned long& ulEnd) {
	unsigned long ulDiff = 0;
	if (ulStart <= ulEnd) {
		//normal easy scenario
		ulDiff = ulEnd - ulStart;
	} else {
		//an overflow has occured, so ulStart > ulEnd
		ulDiff = ulEnd + (ULONG_MAX - ulStart);
	}

	return ulDiff;
}

/**
 * Private: Determines the difference in time between 'now' and the provided start time (which should also be a result from millis), assuming to be 'short'.
 * (where short < half the overflow time, so roughly 25 days)
 */
unsigned long CST816Touch::millisDiff(const unsigned long& ulStart) {
	return millisDiff(ulStart, millis());
}

/**
 * Private: Converts the touch coordinates.
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

	uint8_t iEventFlag = (m_ucBuffer[TOUCH_XH] & 0xC0) >> 6;	//BIT 7 ~ BIT 6: event_flg from TOUCH_XH
	
	if (m_bNotifyMotion) {
		currentlyPressed = (iEventFlag == touch_t::TOUCH_CONTACT) || (iEventFlag == touch_t::TOUCH_DOWN);
	} else {
		currentlyPressed = (iEventFlag == touch_t::TOUCH_DOWN);
	}

#ifdef CST816_TOUCH_LIB_DEBUG	
	//iEventFlag = iEventFlag >> 6; 
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
	
	if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) {
		return (iEventFlag == CST816Touch::touch_t::TOUCH_UP) || (iEventFlag == CST816Touch::touch_t::TOUCH_CONTACT);
	}
	
	if (m_bNotifyMotion) {
		//touch-up is required to ensure we still see normal press events
		//contact is required for the gesture-movements
		return (iEventFlag == CST816Touch::touch_t::TOUCH_UP) || (iEventFlag == CST816Touch::touch_t::TOUCH_CONTACT);
	}
	
	if (m_bNotifyReleaseOnly) {
		return iEventFlag == CST816Touch::touch_t::TOUCH_UP;
	}

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

		if (!bOk) {
			CST816_TOUCH_DEBUG_PRINT("Write command failed, reply: ");
			CST816_TOUCH_DEBUG_PRINTLN(iRet);
			//https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
		}
	}
	return bOk;
}

/**
 * Private: read from a register
 */
bool CST816Touch::readRegister(uint8_t ucRegister, uint8_t iRequestedSize) {
	bool bOk = false;
	
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
	
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
 * Private: Internally used to set the operational mode of the CST816 chip
 */
bool CST816Touch::setOperatingMode(touch_opering_mode_t eOperingMode, bool bNotifyMotion, bool bWakeChipFirst) {
	
	if (bWakeChipFirst) {
		//since we're setting the new state now, there's no point in trying to restore state
		resetChip(false);
	}
	
	uint8_t ucIRQEnable = 0;
	uint8_t ucMotionMask = 0;
	
	//default mode is TOUCH_IRQ_EN_CHANGE & TOUCH_IRQ_EN_TOUCH => 0x60
	bool bOk = false;
	switch (eOperingMode) {
		case CST816Touch::touch_opering_mode_t::TOUCH_MODE_FAST:
			CST816_TOUCH_DEBUG_PRINTLN("Enabling fast operation mode");
			ucIRQEnable = TOUCH_IRQ_EN_CHANGE;
			if (bNotifyMotion) {
				ucIRQEnable |= TOUCH_IRQ_EN_MOTION | TOUCH_IRQ_EN_TOUCH;
			}
			ucMotionMask = 0;
			bOk = true;
			break;
			
		case CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE:
			CST816_TOUCH_DEBUG_PRINTLN("Enabling hardware based operation mode");
			ucIRQEnable = TOUCH_IRQ_EN_MOTION | TOUCH_IRQ_EN_LONGPRESS;
			//if (bNotifyMotion) {	//MDO: not sure if this works.. Check later.
			//	ucIRQEnable |= TOUCH_IRQ_EN_MOTION | TOUCH_IRQ_EN_TOUCH;
			//}
			ucMotionMask = MOTION_MASK_DOUBLE_CLICK;
			bOk = true;
			break;
			
		default:
			break;
	}
	
	if (bOk) {
		bOk = writeRegister(TOUCH_REGISTER_IRQ_CTL, ucIRQEnable);
		if (bOk) {
			//store the operating mode for future reference, but only when this has been ACK-ed by the chip
			m_eOperatingMode = eOperingMode;
		}
	}
	if (bOk) {
		bOk = writeRegister(TOUCH_REGISTER_MOTION_MASK, ucMotionMask);
	}
	
	return bOk;
}

/**
 * Private: just for debug purposes, reads the registers determining the operational mode
 */
bool CST816Touch::printChipOperatingMode() {

	Serial.print("Operating mode: ");
	Serial.print(CST816Touch::operatingModeToString(m_eOperatingMode));
	if (m_bNotifyReleaseOnly) {
		Serial.print(" NotifyReleaseOnly ");
	} else {
		Serial.print(" NotifyAll ");
	}
	
	if (m_bNotifyMotion) {
		Serial.print(" NotifyMotion ");
	}

	bool bOk = readRegister(TOUCH_REGISTER_IRQ_CTL, 1);
	if (bOk) {
		uint8_t ucIRQCtrl = m_ucBuffer[0];
		Serial.print(" IRQ CTRL: 0x");
		Serial.print(ucIRQCtrl, HEX);
		bOk = readRegister(TOUCH_REGISTER_MOTION_MASK, 1);
	}
	if (bOk) {
		uint8_t ucMotionMask = m_ucBuffer[0];
		Serial.print(" Motion mask: 0x");
		Serial.print(ucMotionMask, HEX);
	}
	Serial.println("");
	
	return bOk;
}


/**
 * Private: reads the 'standard' CST816 register
 */
bool CST816Touch::read() { 
	bool bReadSuccess = readRegister(TOUCH_REGISTER_WORK, sizeof(m_ucBuffer));
	bool bSomethingToReport = (bReadSuccess) && (memcmp(m_ucBuffer, m_ucPreviousBuffer, sizeof(m_ucBuffer)) != 0);
	
    return bSomethingToReport;
}

/**
 * Private: did we just had an event?
 * Either handles a interrupt from 'just now', or actively checks this
 */
bool CST816Touch::hadPhysicalEvent() {
	bool bHadPhysicalEvent = false;
	
	if (g_bTouchInterrupt) {
		g_bTouchInterrupt = false;
		bHadPhysicalEvent = read();
	}
	
	return bHadPhysicalEvent;
}

bool CST816Touch::handleGesture(gesture_t eGesture, int x, int y, bool currentlyPressed) {
	if ((eGesture != GESTURE_RIGHT) && (eGesture != GESTURE_LEFT) && 
		(eGesture != GESTURE_DOWN) && (eGesture != GESTURE_UP) && 
		(eGesture != GESTURE_TOUCH_BUTTON) && (eGesture != GESTURE_LONG_PRESS) && (eGesture != GESTURE_DOUBLE_CLICK))  {
#ifdef CST816_TOUCH_LIB_DEBUG
		Serial.print("Unkown gesture: ");
		Serial.println(eGesture, HEX);
		printBuf();
		return false;
#endif
	}

	bool bReportAnEvent = false;
	
	if (eGesture == GESTURE_TOUCH_BUTTON) {
		//also this requires some specifics.. :-(
		//sometimes this is triggered when you're just touching the center of the screen
		//filter that by an additional position check
		if ((x == TOUCH_BUTTON_X) && (y == TOUCH_BUTTON_Y)) {		
			bReportAnEvent = ((m_bNotifyReleaseOnly == false) ||					//if we want to know all event types
							  ((m_bNotifyReleaseOnly) && (!currentlyPressed)));		//or, on a relevant event

		}
	} else {
		//ok, good: we have a supported (and non-special-handling) gesture
		if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) {
			bReportAnEvent = true;
		} else {
			bReportAnEvent =	((m_bNotifyReleaseOnly == false) ||					//if we want to know all event types
								 ((m_bNotifyReleaseOnly) && (!currentlyPressed)) ||	//or, on a relevant event
								 (m_bNotifyMotion));								//or, on all movements
		}
	}
	
	if ((bReportAnEvent)															&&
		(m_bNotifyMotion)															&&
		(m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_FAST)	&& 		
		(millisDiff(m_ulLastGestureTime) < m_ulMovementInterval)) {
		//limit the amount of gesture events in 'report movement mode' to one per 50 msec
		bReportAnEvent = false;
	}
	
	if (bReportAnEvent) {
		m_eLastGesture = eGesture;
		m_iLastGestureX = x;
		m_iLastGestureY = y;

		m_bGestureConsumed = false;
		m_ulLastGestureTime = millis();
		if (m_pTouchSubscriber != 0) {
			m_pTouchSubscriber->gestureNotification(this, m_eLastGesture, !currentlyPressed);
		}
	} else {
		//CST816_TOUCH_DEBUG_PRINTLN("Not handling this");	
	}

	return bReportAnEvent;
	
}

/**
 * Private: handle a gesture
 * returns true when something was (or should be) reported
 */
bool CST816Touch::handleGesture() {
	
	gesture_t eGesture = (gesture_t)m_ucBuffer[TOUCH_INDEX_GESTURE];

	//printBuf(true);

	int x = 0;
	int y = 0;
	bool currentlyPressed = false;
	if (!parseTouchEvent(x, y, currentlyPressed)) {
		//CST816_TOUCH_DEBUG_PRINTLN("        handleGesture - not handling this");
		return false;
	}

	return handleGesture(eGesture, x, y, currentlyPressed);
}

/**
 * Private: Handle a touch event
 */
bool CST816Touch::handleTouch() {
	
	bool bReportAnEvent = false;
	
	int x=0;
	int y=0;
	bool currentlyPressed = false;
	if (!parseTouchEvent(x, y, currentlyPressed)) {
		//CST816_TOUCH_DEBUG_PRINTLN("        handleTouch - not handling this");
		return false;
	}		
		
	if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) {
		bReportAnEvent = true;
	} else {
		bReportAnEvent =	((m_bNotifyReleaseOnly == false) ||				//if we want to know all event types
							((m_bNotifyReleaseOnly) && (!currentlyPressed)));	//or, on a relevant event
	}

	if ((x == TOUCH_BUTTON_X) && (y == TOUCH_BUTTON_Y)) {
		//sometimes a touch on the touch-button is not registered as such.. Handle this here for a more consitent interfase.			
		m_eLastGesture = GESTURE_TOUCH_BUTTON;

		if (bReportAnEvent) {
			bReportAnEvent = handleGesture(GESTURE_TOUCH_BUTTON, x, y, currentlyPressed);
		}
	} else {
		//not a touch_button, but a normal touch
		m_iLastTouchX = x;
		m_iLastTouchY = y;
		if (bReportAnEvent) {
			m_bTouchConsumed = false;
			if (m_pTouchSubscriber != 0) {
				m_pTouchSubscriber->touchNotification(this, m_iLastTouchX, m_iLastTouchY, !currentlyPressed);
			}
		}
		
		if ((!currentlyPressed) && (m_eOperatingMode != touch_opering_mode_t::TOUCH_MODE_HARDWARE)){
			//check if we can register this as a double click, but not in the mode where the hardware already does this..
			if (millisDiff(m_ulLastTouchTime) < 500) {				
				//CST816_TOUCH_DEBUG_PRINTLN("Creating a software based double click event");
				bReportAnEvent = handleGesture(GESTURE_DOUBLE_CLICK, x, y, currentlyPressed);
			}				
			m_ulLastTouchTime = millis();
		}
	}
	
	return bReportAnEvent;
}

/*static*/ String CST816Touch::deviceTypeToString(device_type_t eDeviceType) {
	
	switch (eDeviceType) {
		case DEVICE_CST716:		return "CST716";
		case DEVICE_CST816S:	return "CST816S";
		case DEVICE_CST816T:	return "CST816T";
		case DEVICE_CST816D:	return "CST816D";
		case DEVICE_UNKNOWN:
		default:
			//revert to the default behaviour
			break;
	}
	
	return String("Unknow device type: ") + String((int)eDeviceType);
}

/**
 * Convert a gesture id (gesture_t) to a String
 */
/*static*/ String CST816Touch::gestureIdToString(int iGestureId) {
	if (iGestureId > 0) {
		gesture_t eGesture((gesture_t)iGestureId);
		switch (eGesture) {
			case GESTURE_NONE:			return "NONE";
			case GESTURE_LEFT:			return "LEFT";
			case GESTURE_RIGHT:			return "RIGHT";
			case GESTURE_UP:			return "UP";
			case GESTURE_DOWN:			return "DOWN";
			case GESTURE_TOUCH_BUTTON:	return "TOUCH_BUTTON";
			case GESTURE_DOUBLE_CLICK:	return "DOUBLE_CLICK";
			case GESTURE_LONG_PRESS:	return "LONG_PRESS";
			default:
				//revert to the default behaviour
				break;			
		}
	}
	return String("UNKNOWN: ") + String(iGestureId);
}

/*static*/ String CST816Touch::operatingModeToString(touch_opering_mode_t eOperatingMode) {
	switch (eOperatingMode) {
		case TOUCH_MODE_DEFAULT:	return "DEFAULT";
		case TOUCH_MODE_FAST:		return "FAST";
		case TOUCH_MODE_HARDWARE:	return "HARDWARE";
		default:
			//revert to the default behaviour
			break;		
	}
	return String("Unknow operating mode: ") + String((int)eOperatingMode);
}

/**
 * Get the position of the last touch.
 * Note: 'consumes the last touch', meaning it will reset this info, to minimize the chance of a dual processing of a single event
 * When x=y=0, please assume no touch ever happened.
 */
void CST816Touch::getLastTouchPosition(int& x, int& y) {
	if (!m_bTouchConsumed) {
		m_bTouchConsumed = true;
		x = m_iLastTouchX;
		y = m_iLastTouchY;
	} else {
		x = 0;
		y = 0;
	}
	
}

/**
 * Get the position of the last gesture.
 * Note: 'consumes the last gesture', meaning it will reset this info, to minimize the chance of a dual processing of a single event
 * When x=y=0, please assume no gesture ever happened.
 */
void CST816Touch::getLastGesture(gesture_t& gesture, int& x, int& y) {
	if (!m_bGestureConsumed) {
		m_bGestureConsumed = true;
		x = m_iLastGestureX;
		y = m_iLastGestureY;
		gesture = m_eLastGesture;
	} else {
		gesture	= GESTURE_NONE;
		x = 0;
		y = 0;
	}
}

bool CST816Touch::hadTouch() const {
	return (m_iLastTouchX != 0) && (m_iLastTouchY != 0) && (!m_bTouchConsumed);
}

bool CST816Touch::hadGesture() const {
	return (m_eLastGesture != GESTURE_NONE) && (!m_bGestureConsumed);
}

/**
 * Sets the auto sleep mode, note that the default is: 'auto sleep mode on'
 * In 'dynamic mode' it is documented to consume < 2.5mA, while in the other two sleep modes it consumes <10uA and <5uA respectively.
 * returns true on success
 */
bool CST816Touch::setAutoSleep(bool bEnable) {
	uint8_t ucAutoSleep = 0;
	if (!bEnable) {
		ucAutoSleep = 0xFF;	//disable auto sleep
	}
	
	return writeRegister(TOUCH_REGISTER_AUTOSLEEP, ucAutoSleep);
}


bool CST816Touch::getDeviceType(device_type_t& eDeviceType) {

	bool bOk = readRegister(TOUCH_REGISTER_CHIP_ID, 1);
	if (bOk) {
		eDeviceType = (device_type_t)m_ucBuffer[0];
	}
	
	return bOk;
}

/**
 * Gets the firmware version.
 * Can be used to check CST816 availability
 */
bool CST816Touch::getFirmwareVersion(unsigned char& ucFW) {
	bool bOk = false;
	ucFW = 0;
	
	CST816_TOUCH_DEBUG_PRINTLN("Checking touch firmware..");	

	bOk = readRegister(TOUCH_REGISTER_FW_VERSION, 1);
	if (bOk) {
		ucFW = m_ucBuffer[0];
	}

	if (!bOk) {
		CST816_TOUCH_DEBUG_PRINTLN("Checking touch firmware failed");
	}
	
	return bOk;
}

/**
 * Set the CST816 chip in sleep mode, and prepares the ESP32 for a deep sleep with touch screen as a wakeup source
 */
bool CST816Touch::sleep() {
	
	bool bOk = writeRegister((uint8_t)TOUCH_REGISTER_SLEEP, (uint8_t)TOUCH_CMD_SLEEP);

	if (!bOk) {
		CST816_TOUCH_DEBUG_PRINTLN("Sleep request failed..");
	}
	
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
		//ensure we'll wake when ESP32 deep sleep is started
		esp_sleep_enable_ext0_wakeup((gpio_num_t)m_iPIN_INTERRUPT, 0);
	}
	
	return bOk;
}

/**
 * Resets the CST816 chip. Putting it back in 'dynamic mode', meaning the mode in which it listens to our commands.
 * Note that unless bRestoreState is set to true, it will revert back to the *chip-default* behaviour
 * Internally used in setOperatingMode, should not be needed to externally, typically.
 */
bool CST816Touch::resetChip(bool bRestoreState) {
	bool bOk = false;
	
	touch_opering_mode_t ePreviousOperatingMode = m_eOperatingMode;
	
	if (m_iPIN_RESET != -1) {
		CST816_TOUCH_DEBUG_PRINTLN("Resetting the CST816");
		digitalWrite(m_iPIN_RESET, LOW);	//this is the actual reset
		delay(20);							//arbitrary-but-tested value..
		digitalWrite(m_iPIN_RESET, HIGH);	
		delay(100);							//arbitrary-but-tested value.. (100 seems to work as well)
		
		//ensure we know the reset happened, to ensure we don't get into limbo
		m_eOperatingMode = CST816Touch::touch_opering_mode_t::TOUCH_MODE_DEFAULT;
		
		if (bRestoreState) {
			//and, since requested: try to restore state
			bOk = setOperatingMode(ePreviousOperatingMode, m_bNotifyMotion, false);
		} else {
			bOk = true;
		}
	}
	return bOk;
}

/**
 * Debug method only: print internal buffer.
 * Is useful only after a Read, typically.
 */
void CST816Touch::printBuf(bool bWhenChangedOnly /* = false*/) {
	
	static uint8_t ucOldBuffer[sizeof(m_ucBuffer)];
	
	if ((!bWhenChangedOnly) || (memcmp(m_ucBuffer, ucOldBuffer, sizeof(m_ucBuffer)) != 0)) {
		Serial.print("    ");	//in order to ensure we can still see the rest of the log entries
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
	
	memcpy(ucOldBuffer, m_ucBuffer, sizeof(m_ucBuffer));
}

/**
 * De-initialize this classs.
 */
void CST816Touch::deInit() {
	stop();
}

/**
 * Counterpart of begin
 */
void CST816Touch::stop() {
	if ((m_pI2C != 0) && (m_bWeInitializedI2C)) {
		m_pI2C->end();
	}
	m_pI2C = 0;
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
	memset(m_ucPreviousBuffer, 0, sizeof(m_ucPreviousBuffer));
	m_pTouchSubscriber = 0;
	m_bTouchConsumed = true;
	m_bGestureConsumed = true;
}

/**
 * Periodically call this, from *loop* typically.
 * The return value *can* optionally be checked if a touch/gesture has happened, also see hadTouch & hadGesture for 'the same'.
 * All of this return value checking, hadTouch & hadGesture are not needed when the callback mechanism is used.
 */
bool CST816Touch::control() {
	bool bHadPhysicalEvent = hadPhysicalEvent();
	bool bReportAnEvent = false;
	
	if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_DEFAULT) {
		//none of the two main operating modes has been set yet.
		//ignore input based on that
		return false;
	}
	
	if (bHadPhysicalEvent) {
#ifdef CST816_TOUCH_LIB_DEBUG
		//printBuf(true);
#endif
		
		gesture_t eGesture = (gesture_t)m_ucBuffer[TOUCH_INDEX_GESTURE];
		bool bGesture = eGesture != 0x00;
		//if needed, the following will convert a 'touch_button'-gesture to a normal touch
		if ((bGesture) && (eGesture == GESTURE_TOUCH_BUTTON) && (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE)) {
			bGesture = false;				//handle as normal touch
			m_ulLastGestureTime = millis();	//but don't forget to register as it actually was, in order to clean the previous buffer stuff
		}
		
		//check gesture first
		if (bGesture) {
			bReportAnEvent = handleGesture();
		} else {
			bReportAnEvent = handleTouch();
		}
		//ensure we never re-parse the same
		memcpy(m_ucPreviousBuffer, m_ucBuffer, sizeof(m_ucBuffer));		
	}
	
	if ((m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) &&
		(m_ulLastGestureTime != 0) &&
		(millisDiff(m_ulLastGestureTime) > 100)) {
		//ok, so in TOUCH_MODE_HARDWARE mode, we only get relevant events (as-in, only the 'release', not the 'press')
		//in that scenario, when clicking on the EXACT SAME position twice, 
		//we will not get the second touch, which is caused by it being filtered using the m_ucPreviousBuffer (being equal).
		//It sounds like that will never happen, but this is exactly what happens with the touch-button at the right side of the screen
		//..
		//so reset the buffer to ensure we 'forget', assuming 100 msec as a safe timeout
		memset(m_ucPreviousBuffer, 0, sizeof(m_ucPreviousBuffer));
		m_ulLastGestureTime = 0;
	}
	
	return bReportAnEvent;
}

/**
 * Sets the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
 * In only relevant when setNotifyOnMovement(true) has been called
 */
void CST816Touch::setMovementInterval(unsigned long ulMovementInterval) {
	m_ulMovementInterval = ulMovementInterval;
}
/**
 * Enables / disables notifications on movement. Will only work in fast operating mode (setOperatingModeFast)
 * When enabled, invalidates setNotificationsOnReleaseOnly
 */
bool CST816Touch::setNotifyOnMovement(bool bMovementNotificationsRequired /*= true*/) {
	
	bool bOk = false;
	
	if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_FAST) {
		bool bModeChanged = bMovementNotificationsRequired != m_bNotifyMotion;
		m_bNotifyMotion = bMovementNotificationsRequired;		
		
		if (bModeChanged) {
			bOk = 	setOperatingMode(m_eOperatingMode, m_bNotifyMotion, false) ||	//try without a wake first..
					setOperatingMode(m_eOperatingMode, m_bNotifyMotion, true);		//if not, awake it
#ifdef CST816_TOUCH_LIB_DEBUG
			printChipOperatingMode();
#endif
			
		} else {
			bOk = true;
		}
	} else {
		m_bNotifyMotion = false;
		if (bMovementNotificationsRequired) {
			//since this seems to remove the double-click capability detection from the chip
			if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) {
				CST816_TOUCH_DEBUG_PRINTLN("Movement notifications do not seem to be possible in hardware mode..");
			} else {	//TOUCH_MODE_DEFAULT
				CST816_TOUCH_DEBUG_PRINTLN("Please set an operational mode first");
			}
		} else {
			bOk = true;
		}
	}
	
	return bOk;
}

/**
 * get notifications on touch events and release event
 * Note that this is only supported when:
 *   - a TouchSubscriberInterface has been provided AND 
 *   - touch operating mode is TOUCH_MODE_FAST
 */
bool CST816Touch::setNotificationsOnAllEvents() {

	bool bOk = (m_pTouchSubscriber != 0) && (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_FAST);
	
	if (bOk) {
		m_bNotifyReleaseOnly = false;
	} else {
#ifdef CST816_TOUCH_LIB_DEBUG
		if (m_pTouchSubscriber != 0) {
			CST816_TOUCH_DEBUG_PRINTLN("Setting notifications on touch-events and release-events only works in the callback enabled scenario");
		}
		if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) {
			CST816_TOUCH_DEBUG_PRINTLN("Setting notifications on touch-events and release-events does not work in TOUCH_MODE_HARDWARE mode");
		} else if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_DEFAULT) {
			CST816_TOUCH_DEBUG_PRINTLN("Please set an operational mode first");
		}
#endif
	}

	
	return m_bNotifyReleaseOnly == false;
}

/**
 * get notifications on release event only - this is the default
 * When enabled, invalidates setNotifyOnMovement(true)
 */
void CST816Touch::setNotificationsOnReleaseOnly() {
	m_bNotifyReleaseOnly = true;	
	if (m_bNotifyMotion) {
		setNotifyOnMovement(false);
	}
}

/**
 * Set the quickest operating mode.
 * Double click and long press are software based, double click triggers 2 touch events AND a gesture event
 */
bool CST816Touch::setOperatingModeFast() {
	return	setOperatingMode(TOUCH_MODE_FAST, m_bNotifyMotion, false) ||	//try without waking the chip first
			setOperatingMode(TOUCH_MODE_FAST, m_bNotifyMotion, true);
}

/**
 * Set full hardware based operating mode.
 * Resets NotificationsOnReleaseOnly-state and NotifyOnMovement-state (both: off)
 * Long press takes a lot longer, double click however only gives a single gesture event 
 */
bool CST816Touch::setOperatingModeHardwareBased() {
	m_bNotifyReleaseOnly = true;
	m_bNotifyMotion = false;

	bool bOk =	setOperatingMode(TOUCH_MODE_HARDWARE, m_bNotifyMotion, false) ||	//try without waking the chip first
				setOperatingMode(TOUCH_MODE_HARDWARE, m_bNotifyMotion, true);

#ifdef CST816_TOUCH_LIB_DEBUG	
	printChipOperatingMode();
#endif
	
	return bOk;
}


/**
 * Initialize: this is a mandatory call to be made
 * @deprecated, since this initializes Wire and it's not the Ardunio recommended name
 * For each PIN which this class should not use / change: set to -1
 * The interrupt pin is mandatory for proper usage.
 * return false on issues
 */
bool CST816Touch::init(TwoWire& w, TouchSubscriberInterface* pTouchSubscriber /*= 0*/, 
							int PIN_INTERRUPT /*= 16*/, 
							int I2C_SDA /*= 18*/, 
							int I2C_SCL /*= 17*/, 
							uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/, 							
							int PIN_RESET /*= 21*/) {	//note that this is a mandatory PIN, so don't set this to -1
	
	bool bOk = begin(w, pTouchSubscriber, PIN_INTERRUPT, CTS816S_I2C_ADDRESS, PIN_RESET);

	if ((bOk) && (m_pI2C != 0)) {
		m_pI2C->begin(I2C_SDA, I2C_SCL);
		m_pI2C->setClock(400000);	//For reliable communication, it is recommended to use a maximum communication rate of 400Kbps	
		m_bWeInitializedI2C = true;
	}

	return bOk;
}

/**
 * begin: this is a mandatory call to be made.
 * Please ensure that Wire has been initialized before this call, using the Wire.begin method
 * For each PIN which this class should not use / change: set to -1
 * The interrupt pin is mandatory for proper usage.
 * return false on issues
 */
bool CST816Touch::begin(TwoWire& w, TouchSubscriberInterface* pTouchSubscriber /*= 0*/, 
							int PIN_INTERRUPT 			/*= 16 */, 
							uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/, 							
							int PIN_RESET 				/*= 21*/) {	//note that this is a mandatory PIN, so don't set this to -1
	m_pI2C = &w;
	bool bOk = m_pI2C != 0;
	m_bWeInitializedI2C = false;
	
	m_ucAddress = CTS816S_I2C_ADDRESS;
	m_iPIN_RESET = PIN_RESET;
	m_iPIN_INTERRUPT = PIN_INTERRUPT;
	m_pTouchSubscriber = pTouchSubscriber;
	
	if (bOk) {
		bOk = m_iPIN_RESET != -1;
	}
	
	if (bOk) {
		CST816_TOUCH_DEBUG_PRINTLN("Ensure touch screen is properly reset");
		
		//init PGIO, if required
		//see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
		gpio_hold_dis((gpio_num_t)m_iPIN_RESET);
		pinMode(m_iPIN_RESET, OUTPUT);
		resetChip(false);
	}
	
	if (bOk) {
		//get the firmware just to see if we're able to communicate with the chip
		unsigned char ucFW = 0;
		bOk = getFirmwareVersion(ucFW);
#ifdef CST816_TOUCH_LIB_DEBUG	
		if (bOk) {
			Serial.print("    Firmware: ");
			Serial.println(ucFW, HEX);
		}
#endif
	}
	
	
	if (m_iPIN_INTERRUPT != -1) {
		CST816_TOUCH_DEBUG_PRINTLN("Configuring touch interrupt");		
	    attachInterrupt(
			m_iPIN_INTERRUPT, [] { g_bTouchInterrupt = true; }, FALLING);
	}
	
	if (bOk) {
		//set our default: the quickest one
		bOk = setOperatingModeFast();
	}
	
#ifdef CST816_TOUCH_LIB_DEBUG	
	if (bOk) {
		printChipOperatingMode();
		CST816_TOUCH_DEBUG_PRINTLN("Touch screen configured");
	} else {
		CST816_TOUCH_DEBUG_PRINTLN("Touch screen configuration failed..");
	}
#endif	

	return bOk;
}

CST816Touch::CST816Touch() {
	m_pI2C = 0;
	m_bWeInitializedI2C = false;
	m_iPIN_RESET = -1;
	m_iPIN_INTERRUPT = -1;
	m_uiCTS816S_I2C_ADDRESS = 0;
	
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
	m_pTouchSubscriber = 0;
	
	m_eLastGesture	= GESTURE_NONE;
	m_iLastGestureX = 0;
	m_iLastGestureY = 0;
	
	m_ulLastTouchTime = 0;
	m_iLastTouchX = 0;
	m_iLastTouchY = 0;
	
	
	m_bTouchConsumed = true;
	m_bGestureConsumed = true;
	m_eOperatingMode = CST816Touch::touch_opering_mode_t::TOUCH_MODE_DEFAULT;	//set to the default from the chip itself
	m_bNotifyReleaseOnly = true;
	m_bNotifyMotion = false;
	
	m_ulLastGestureTime = 0;
	m_ulMovementInterval = 50;
}

CST816Touch::~CST816Touch() {
	deInit();
}

}	//namespace end
