#include "CST816Touch.h"
#include "Tools.h"
#include "TouchScreenObserver.h"

#include "DebugMsg.h"

//avoid the defines, as per https://docs.arduino.cc/learn/contributions/arduino-writing-style-guide#variables
const uint8_t CTS816_I2C_ADDRESS				= 0x15;
const uint8_t TOUCH_REGISTER_SLEEP				= 0xA5;
const uint8_t TOUCH_CMD_SLEEP 					= 0x03;

const uint8_t TOUCH_REGISTER_WORK				= 0x00;
const uint8_t TOUCH_REGISTER_GESTURE			= 0x01;
const uint8_t TOUCH_REGISTER_FINGER_NUMBER  	= 0x02;		//the finger number: either 0 (no finger) or 1 (a finger)
const uint8_t TOUCH_REGISTER_TOUCH_XH			= 0x03;
const uint8_t TOUCH_REGISTER_TOUCH_XL			= 0x04;
const uint8_t TOUCH_REGISTER_TOUCH_YH			= 0x05;
const uint8_t TOUCH_REGISTER_TOUCH_YL			= 0x06;

const uint8_t TOUCH_REGISTER_BPC0H				= 0xB0;	//not sure what this is for though..	BPC0[15:8]
const uint8_t TOUCH_REGISTER_BPC0L				= 0xB1;	//not sure what this is for though..	BPC0[7:0]
const uint8_t TOUCH_REGISTER_BPC1H				= 0xB2;	//not sure what this is for though..	BPC1[15:8]
const uint8_t TOUCH_REGISTER_BPC1L				= 0xB3;	//not sure what this is for though..	BPC1[7:0]

//const uint8_t TOUCH_REGISTER_VERSION			= 0x15;		//not sure where I got this from, but seems to be nothing
const uint8_t TOUCH_REGISTER_CHIP_ID			= 0xA7;		//chip ID (/model) (1 byte)
const uint8_t TOUCH_REGISTER_PROJ_ID			= 0xA8;		//project number (1 byte)
const uint8_t TOUCH_REGISTER_FW_VERSION			= 0xA9;		//firmware version

const uint8_t TOUCH_REGISTER_MOTION_MASK		= 0xEC;		//motion mask register
const uint8_t MOTION_MASK_CONTINUOUS_LEFT_RIGHT	= 0x04;	//Enables continuous left and right sliding actions
const uint8_t MOTION_MASK_CONTINUOUS_UP_DOWN	= 0x02;	//Enable continuous up and down sliding actions
const uint8_t MOTION_MASK_DOUBLE_CLICK			= 0x01;	//add hardware based double click

//Interrupt low pulse output width.
//Unit: 0.1ms, optional value: 1~200. The default value is 10.
const uint8_t TOUCH_REGISTER_IRQ_PULSE_WIDTH	= 0xED;

//Normal rapid testing cycle.
//This value affects LpAutoWakeTime and AutoSleepTime.
//Unit: 10ms, optional values: 1~30. The default value is 1.
const uint8_t TOUCH_REGISTER_NORMAL_SCAN_PERIOD	= 0xEE;

//Gesture detection sliding partition angle control. Angle=tan(c)*10
//c is the angle based on the positive direction of the x-axis.
const uint8_t TOUCH_REGISTER_ANGLE_CONTROL		= 0xEF;

//Low power consumption scans the upper 8 bits of the reference value of channel 1
const uint8_t TOUCH_REGISTER_LpScanRaw1H		= 0xF0;
const uint8_t TOUCH_REGISTER_LpScanRaw1L		= 0xF1;

//Low power consumption scans the upper 8 bits of the reference value of channel 2
const uint8_t TOUCH_REGISTER_LpScanRaw2H		= 0xF2;
const uint8_t TOUCH_REGISTER_LpScanRaw2L		= 0xF3;

//Automatic recalibration cycle during low power consumption.
//The unit is 1 minute, optional values: 1~5. The default value is 5
const uint8_t TOUCH_REGISTER_LpAutoWakeTime		= 0xF4;

//Low power scan wake-up threshold. The smaller it is, the more sensitive it is.
//Optional values: 1~255. The default value is 48
const uint8_t TOUCH_REGISTER_LpScanTH			= 0xF5;

//Low power scanning range. The bigger it is, the more sensitive it is and the higher its power consumption.
//Optional values: 0, 1, 2, 3. The default value is 3.
const uint8_t TOUCH_REGISTER_LpScanWin			= 0xF6;

//Low power scanning frequency. The smaller it is, the more sensitive it is.
//Optional values: 1~255. The default value is 7
const uint8_t TOUCH_REGISTER_LpScanFreq			= 0xF7;

//Low power scan current. The smaller it is, the more sensitive it is.
//Optional values: 1~255
const uint8_t TOUCH_REGISTER_LpScanIdac			= 0xF8;

//When there is no touch within x seconds, it automatically enters low power consumption mode.
//The unit is 1sec, and the default value is 2sec.
const uint8_t TOUCH_REGISTER_AutoSleepTime		= 0xF9;

const uint8_t TOUCH_REGISTER_IRQ_CTL			= 0xFA;		//interrupt control
const uint8_t TOUCH_IRQ_EN_IRQ_TEST				= 0x80;		//Interrupt pin test, automatically send out low pulse periodically after enabled
const uint8_t TOUCH_IRQ_EN_TOUCH				= 0x40;	//Periodically pulses low when a touch is detected	//gives a lot of events, in itself only provide touch (TOUCH_CONTACT) info. also include gesture info
const uint8_t TOUCH_IRQ_EN_CHANGE				= 0x20;	//When a change in touch status is detected, a low pulse is emitted	//gives or adds the release (TOUCH_UP) info
const uint8_t TOUCH_IRQ_EN_MOTION				= 0x10;	//When a gesture is detected, pulse low	//seems to add the GESTURE_TOUCH_BUTTON events, add long press-while-still-touched gestures
const uint8_t TOUCH_IRQ_EN_LONGPRESS			= 0x01;	//The long press gesture only emits a low pulse signal	//seems to do nothing..?	//note: document inconsist

//When there is a touch but no valid gesture within x seconds, it will automatically reset.
//The unit is 1sec. When it is 0, this function is not enabled. Default is 5sec
const uint8_t TOUCH_REGISTER_AutoReset			= 0xFB;

//Automatically reset after long pressing for x seconds.
//The unit is 1sec. When it is 0, this function is not enabled. Default is 10
const uint8_t TOUCH_REGISTER_LongPressMaxTime	= 0xFC;

const uint8_t TOUCH_REGISTER_IO_CTL				= 0xFD;
const uint8_t TOUCH_IO_SOFT_RST					= 0x04;	//The main control implements the soft reset function of the touch control by pulling the IRQ pin low. 0= Soft reset is disabled, 1= Enable soft reset.
const uint8_t TOUCH_IO_IIC_0D					= 0x02;	//IIC pin drive mode, the default is resistor pull-up. 0= Resistor pull-up, 1=OD
const uint8_t TOUCH_IO_LEVEL_SELECTION			= 0x01;	//IIC and IRQ pin level selection, the default is VDD level. 0=VDD, 1=1.8V

//can be used to disable auto sleep, might be quicker (?) but will consume more power
//The default is 0, which enables automatic entry into low power mode.
//When the value is non-0, automatic entry into low power consumption mode is prohibited.
const uint8_t TOUCH_REGISTER_AUTOSLEEP			= 0xFE;

const uint8_t  I2C_OK = 0;

const int TOUCH_BUTTON_X = 360;	//T-Display default
const int TOUCH_BUTTON_Y =  85;	//T-Display default

#ifdef CST816_TOUCH_LIB_DEBUG
#warning Just a message to note that CST816Touch debug messages are enabled
#endif

namespace MDO {
	
bool	g_bTouchInterrupt = false;

/**
 * Private: Converts the touch coordinates.
 * Note that this assumes the USB of the LILYGO T-Display ESP32-S3 to be on the right.
 * Assuming that, x=y=0 == lower left
 * note2: the touch-button is (m_iTouchButtonX,m_iTouchButtonY) in this case
 * returns true when this TouchController events needs to be handled based on the current settings
 */
bool CST816Touch::parseTouchControllerEvent(int& x, int& y, bool& currentlyPressed) {

	//TOUCH_REGISTER_TOUCH_XH = 3 
	//TOUCH_REGISTER_TOUCH_XL = 4	
	x = (m_ucBuffer[TOUCH_REGISTER_TOUCH_XH] & 0x0F) << 8;	//X_position high: [11:8]		BIT 7 ~ BIT 6: event_flg
	x += m_ucBuffer[TOUCH_REGISTER_TOUCH_XL];				//X_position low:  [7:0]
	//TOUCH_REGISTER_TOUCH_YH = 5
	//TOUCH_REGISTER_TOUCH_YL = 6		
	y = (m_ucBuffer[TOUCH_REGISTER_TOUCH_YH] & 0x0F) << 8;	//Y_position high: [11:8]		BIT 7 ~ BIT 4: touch_ID[3:0]
	y += m_ucBuffer[TOUCH_REGISTER_TOUCH_YL];				//Y_position low:  [7:0]

	uint8_t iEventFlag = (m_ucBuffer[TOUCH_REGISTER_TOUCH_XH] & 0xC0) >> 6;	//BIT 7 ~ BIT 6: event_flg from TOUCH_REGISTER_TOUCH_XH
	
	if (m_bNotifyMotion) {
		currentlyPressed = (iEventFlag == TouchScreenController::touch_t::TOUCH_CONTACT) || (iEventFlag == TouchScreenController::touch_t::TOUCH_DOWN);
	} else {
		currentlyPressed = (iEventFlag == TouchScreenController::touch_t::TOUCH_DOWN);
	}

	if (m_bSwapXY) {
		int swapToBeUseful = x;	//the orientation for the T-Display S3 does not match the documentation one
		x = y;					//so swap x and y
		y = swapToBeUseful;
	}
	
#ifdef CST816_TOUCH_LIB_DEBUG
	//Serial.print("x: "); Serial.print(x);
	//Serial.print(", y: "); Serial.print(y);
	//Serial.print(", pressed: "); Serial.println(currentlyPressed);
	
//	Serial.print("                                                                   Event flag: ");
//	Serial.println(iEventFlag);
	
	int iTouchId = (m_ucBuffer[TOUCH_REGISTER_TOUCH_YH] & 0xF0) >> 4;
	if (iTouchId != 0) {				//seems to always be 0
		Serial.print("Touch ID: ");
		Serial.println(iTouchId);
	}
#endif
	
	if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) {
		return (iEventFlag == TouchScreenController::touch_t::TOUCH_UP) || (iEventFlag == TouchScreenController::touch_t::TOUCH_CONTACT);
	}
	
	if (m_bNotifyMotion) {
		//touch-up is required to ensure we still see normal press events
		//contact is required for the gesture-movements
		return (iEventFlag == TouchScreenController::touch_t::TOUCH_UP) || (iEventFlag == TouchScreenController::touch_t::TOUCH_CONTACT);
	}
	
	if (m_bNotifyReleaseOnly) {
		return iEventFlag == TouchScreenController::touch_t::TOUCH_UP;
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
			CST816_TOUCH_DEBUG_PRINT("    Write command failed, reply: ");
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
		//CST816_TOUCH_DEBUG_PRINTLN("    Request data");
		m_pI2C->beginTransmission(m_ucAddress);
		m_pI2C->write(ucRegister);

		int iRet = m_pI2C->endTransmission();
		bOk =	(iRet == I2C_OK) && 
				(m_pI2C->requestFrom(m_ucAddress, iRequestedSize) > 0);
		
		if (bOk) {
			//CST816_TOUCH_DEBUG_PRINTLN("    Reading");
			bOk = m_pI2C->readBytes(&m_ucBuffer[0], iRequestedSize) >= iRequestedSize;
		} else {
			CST816_TOUCH_DEBUG_PRINT("    Read command failed, reply: ");
			CST816_TOUCH_DEBUG_PRINTLN(iRet);			
		}
	} else {
		CST816_TOUCH_DEBUG_PRINTLN("CST816Touch::readRegister - error");
	}
	
	return bOk;
}

/**
 * Private: reads the 'standard' CST816 register
 * Returns true when [read was successful ] && [the read buffer was changed compared to the previous read]
 */
bool CST816Touch::read() { 
	bool bReadSuccess = readRegister(TOUCH_REGISTER_WORK, sizeof(m_ucBuffer));
	bool bSomethingToReport = (bReadSuccess) && (memcmp(m_ucBuffer, m_ucPreviousBuffer, sizeof(m_ucBuffer)) != 0);
	
    return bSomethingToReport;
}

/**
 * Private: did we just had an event?
 * Handles a interrupt from 'just now', while actively checking if we care (the relevant TouchController buffer needs to be altered)
 */
bool CST816Touch::hadPhysicalEvent() {
	bool bHadPhysicalEvent = false;
	
	if (g_bTouchInterrupt) {
		g_bTouchInterrupt = false;	//reset the interrupt flag as soon as possible
		bHadPhysicalEvent = read();
	}
	
	return bHadPhysicalEvent;
}

void CST816Touch::handleGesture(TouchScreenController::gesture_t eGesture, int x, int y, bool currentlyPressed) {
	if ((eGesture != GESTURE_RIGHT) && (eGesture != GESTURE_LEFT) && 
		(eGesture != GESTURE_DOWN) && (eGesture != GESTURE_UP) && 
		(eGesture != GESTURE_TOUCH_BUTTON) && (eGesture != GESTURE_LONG_PRESS) && (eGesture != GESTURE_DOUBLE_CLICK))  {
#ifdef CST816_TOUCH_LIB_DEBUG
		Serial.print("Unkown gesture: ");
		Serial.println(eGesture, HEX);
		printBuf();
#endif
	}

	bool bReportAnEvent = false;
	
	if (eGesture == GESTURE_TOUCH_BUTTON) {
		//also this requires some specifics.. :-(
		//sometimes this is triggered when you're just touching the center of the screen
		//filter that by an additional position check
		if ((x == m_iTouchButtonX) && (y == m_iTouchButtonY)) {		
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
		((currentlyPressed) && (Tools::millisDiff(m_ulLastGestureTime) < m_ulMovementInterval))) {	//only check the timing then being pressed, thanks for the tip @rhueppin
		//limit the amount of gesture events in 'report movement mode' to one per 50 msec
		bReportAnEvent = false;
	}
	
	if (bReportAnEvent) {
		m_ulLastGestureTime = millis();
		notifyObservers(eGesture, x, y, currentlyPressed);
	} else {
		//CST816_TOUCH_DEBUG_PRINTLN("Not handling this");	
	}	
}

/**
 * Private: handle a gesture
 * returns true when something was (or should be) reported
 */
void CST816Touch::handleGesture() {
	
	TouchScreenController::gesture_t eGesture = (TouchScreenController::gesture_t)m_ucBuffer[TOUCH_REGISTER_GESTURE];
#ifdef CST816_TOUCH_LIB_DEBUG
	//printBuf(true);
#endif

	int x = 0;
	int y = 0;
	bool currentlyPressed = false;
	if (!parseTouchControllerEvent(x, y, currentlyPressed)) {
		//CST816_TOUCH_DEBUG_PRINTLN("        handleGesture - not handling this");
		return;
	}

	handleGesture(eGesture, x, y, currentlyPressed);
}

/**
 * Private: Handle a touch event
 */
void CST816Touch::handleTouch() {
	
	bool bReportAnEvent = false;
	
	int x=0;
	int y=0;
	bool currentlyPressed = false;
	if (!parseTouchControllerEvent(x, y, currentlyPressed)) {
		//CST816_TOUCH_DEBUG_PRINTLN("        handleTouch - not handling this");
		return;
	}		
		
	if (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) {
		bReportAnEvent = true;
	} else {
		bReportAnEvent =	((m_bNotifyReleaseOnly == false) ||					//if we want to know all event types
							((m_bNotifyReleaseOnly) && (!currentlyPressed)));	//or, on a relevant event
	}

	if ((x == m_iTouchButtonX) && (y == m_iTouchButtonY)) {
		//sometimes a touch on the touch-button is not registered as such.. Handle this here for a more consistent interfase
		if (bReportAnEvent) {
			if (!currentlyPressed) {	//Just like for the other gestures: assuming no 'touch' is required here, only the 'release'.
				handleGesture(GESTURE_TOUCH_BUTTON, x, y, currentlyPressed);
			}
		}
	} else {
		//not a touch_button, but a normal touch
		if (bReportAnEvent) {
			notifyObservers(x, y, currentlyPressed);
		}
	}
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
			CST816_TOUCH_DEBUG_PRINTLN(String("Enabling fast operation mode") + (bNotifyMotion?" - notify on motion":""));
			ucIRQEnable = TOUCH_IRQ_EN_CHANGE;
			//ucIRQEnable |= TOUCH_IRQ_EN_MOTION;	//MDO!, als test voor nu
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
		//MDO!, als test voor nu
		//ucMotionMask |= MOTION_MASK_CONTINUOUS_LEFT_RIGHT;
		//ucMotionMask |=  MOTION_MASK_CONTINUOUS_UP_DOWN;
		//end
		bOk = writeRegister(TOUCH_REGISTER_MOTION_MASK, ucMotionMask);
	}
	
	return bOk;
}

/**
 * Protected: just for debug purposes, reads the registers determining the operational mode
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
 * Protected: Set the quickest operating mode.
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
	
	return bOk;
}

/**
 * Sets the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
 * In only relevant when setNotifyOnMovement(true) has been called
 */
void CST816Touch::setMovementInterval(unsigned long ulMovementInterval) {
	m_ulMovementInterval = ulMovementInterval;
}

/**
 * get notifications on touch events and release event
 * Note that this is only supported when touch operating mode is TOUCH_MODE_FAST
 */
bool CST816Touch::setNotificationsOnAllEvents() {

	bool bOk = (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_FAST);
	
	if (bOk) {
		m_bNotifyReleaseOnly = false;
	} else {
#ifdef CST816_TOUCH_LIB_DEBUG
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

/*static*/ uint8_t CST816Touch::getDefaultI2Caddress() {
	return CTS816_I2C_ADDRESS;
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
 * Sets the auto sleep mode, note that the default is: 'auto sleep mode on'
 * In 'dynamic mode' it is documented to consume < 2.5mA, while in the other two sleep modes it consumes <10uA and <5uA respectively.
 * returns true on success
 */
bool CST816Touch::setAutoSleep(bool bEnable) {
	uint8_t ucAutoSleep = 0;
	if (!bEnable) {
		ucAutoSleep = 0xFF;	//disable auto sleep
	}
	CST816_TOUCH_DEBUG_PRINT("CST816Touch::setAutoSleep ");
	CST816_TOUCH_DEBUG_PRINTLN(bEnable);
	return writeRegister(TOUCH_REGISTER_AUTOSLEEP, ucAutoSleep);
}


bool CST816Touch::getDeviceType(TouchScreenController::device_type_t& eDeviceType) {

	bool bOk = readRegister(TOUCH_REGISTER_CHIP_ID, 1);
	if (bOk) {
		eDeviceType = (TouchScreenController::device_type_t)m_ucBuffer[0];
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
	
	//CST816_TOUCH_DEBUG_PRINTLN("Checking touch controller firmware..");	

	bOk = readRegister(TOUCH_REGISTER_FW_VERSION, 1);
	if (bOk) {
		ucFW = m_ucBuffer[0];
	}

	if (!bOk) {
		CST816_TOUCH_DEBUG_PRINT("Checking touch firmware failed. Typically: device not found. Buffer: ");
		CST816_TOUCH_DEBUG_PRINTLN((uint8_t)m_ucBuffer[0]);
	}
	
	return bOk;
}

/**
 * Set the CST816 chip in sleep mode, and prepares the ESP32 for a deep sleep with touch screen as a wakeup source
 */
bool CST816Touch::sleep() {
	
	bool bOk = writeRegister((uint8_t)TOUCH_REGISTER_SLEEP, (uint8_t)TOUCH_CMD_SLEEP);

	if (!bOk) {
		CST816_TOUCH_DEBUG_PRINTLN("Sleep request failed. Chip already asleep?");
		//since the chip is already asleep, no need to see this as an error
		//so just continue with setting the proper interrupt pin for the ESP32
		bOk = true;
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
		CST816_TOUCH_DEBUG_PRINTLN("Resetting the touch controller chip");
		digitalWrite(m_iPIN_RESET, LOW);	//this is the actual reset
		delay(20);							//arbitrary-but-tested value..
		digitalWrite(m_iPIN_RESET, HIGH);	
		delay(100);							//arbitrary-but-tested value.. (100 seems to work)
		
		//ensure we know the reset happened, to ensure we don't get into limbo
		m_eOperatingMode = CST816Touch::touch_opering_mode_t::TOUCH_MODE_DEFAULT;
		
		if (bRestoreState) {
			//and, since requested: try to restore state
			bOk = setOperatingMode(ePreviousOperatingMode, m_bNotifyMotion, false);
		} else {
			bOk = true;
		}
	} else {
		bOk = true;	//we're not able to reset.. So indicate 'all good'.. ?
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
 * Counterpart of begin
 */
void CST816Touch::stop() {
	m_pI2C = 0;
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));
	memset(m_ucPreviousBuffer, 0, sizeof(m_ucPreviousBuffer));
}

/**
 * Periodically call this, from *loop* typically.
 * The return value *can* optionally be checked if a touch/gesture has happened, also see hadTouch & hadGesture for 'the same'.
 * All of this return value checking, hadTouch & hadGesture are not needed when the callback mechanism is used.
 */
/*virtual*/ void CST816Touch::control() {
	bool bHadPhysicalEvent = hadPhysicalEvent();
	
	if (bHadPhysicalEvent) {
#ifdef CST816_TOUCH_LIB_DEBUG
		//printBuf(true);
#endif
		
		TouchScreenController::gesture_t eGesture = (TouchScreenController::gesture_t)m_ucBuffer[TOUCH_REGISTER_GESTURE];
		bool bGesture = eGesture != TouchScreenController::gesture_t::GESTURE_NONE;
		//if needed, the following will convert a 'touch_button'-gesture to a normal touch
		if ((bGesture) && (eGesture == GESTURE_TOUCH_BUTTON) && (m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE)) {
			bGesture = false;				//handle as normal touch
			m_ulLastGestureTime = millis();	//but don't forget to register as it actually was, in order to clean the previous buffer stuff
		}
		
		//check gesture first
		if (bGesture) {
			handleGesture();
		} else {
			handleTouch();
		}
		//ensure we never re-parse the same, so remember the current buffer state
		memcpy(m_ucPreviousBuffer, m_ucBuffer, sizeof(m_ucBuffer));		
	}
	
	if ((m_eOperatingMode == CST816Touch::touch_opering_mode_t::TOUCH_MODE_HARDWARE) &&
		(m_ulLastGestureTime != 0) &&
		(Tools::millisDiff(m_ulLastGestureTime) > 100)) {
		//ok, so in TOUCH_MODE_HARDWARE mode, we only get relevant events (as-in, only the 'release', not the 'press')
		//in that scenario, when clicking on the EXACT SAME position twice, 
		//we will not get the second touch, which is caused by it being filtered using the m_ucPreviousBuffer (being equal).
		//It sounded like that will never happen, but this is exactly what happens with the touch-button at the right side of the screen
		//..
		//so reset the buffer to ensure we 'forget', assuming 100 msec as a safe timeout
		memset(m_ucPreviousBuffer, 0, sizeof(m_ucPreviousBuffer));
		m_ulLastGestureTime = 0;
	}
	
	TouchScreenController::control();
}

/**
 * set the coordinates of the touch button.
 * T-Display: 360, 85
 * T-Display S3: 600, 120
 */
void CST816Touch::setTouchButtonCoordinates(int iTouchButtonX, int iTouchButtonY) {
	m_iTouchButtonX = iTouchButtonX;
	m_iTouchButtonY = iTouchButtonY;
}

/**
 * default is T-Display which needs swapping the XY coordinates
 */
void CST816Touch::setSwapXY(bool bSwapXY /*= true*/) {
	m_bSwapXY = bSwapXY;
}

/**
 * setHardwareLongPressTime => 0 is disable, default is 10
 */
bool CST816Touch::setHardwareLongPressTime(uint8_t uiLongPressTime) {
	bool bOk = writeRegister((uint8_t)TOUCH_REGISTER_LongPressMaxTime, uiLongPressTime);
#ifdef CST816_TOUCH_LIB_DEBUG
	if (!bOk) {
		Serial.println("setHardwareLongPressTime failed");
	}
#endif	
	return bOk;
}

/**
 * begin: this is a mandatory call to be made.
 * Please ensure that Wire has been initialized before this call, using the Wire.begin method
 * For each PIN which this class should not use / change: set to -1
 * For max power efficiency, the interrupt pin is mandatory. If this pin is not set: disables auto sleep.
 * return false on issues
 */
bool CST816Touch::begin(	TwoWire& w,
							TouchScreenObserver* pTouchScreenObserver,
							int PIN_INTERRUPT 			/*= 16 */, 
							int PIN_RESET 				/*= 21*/,
							uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/) {
	m_pI2C = &w;
	bool bOk = m_pI2C != 0;
	
	m_ucAddress = CTS816S_I2C_ADDRESS;
	m_iPIN_RESET = PIN_RESET;
	m_iPIN_INTERRUPT = PIN_INTERRUPT;
	registerObserver(pTouchScreenObserver);
	
//	if (bOk) {
//		bOk = m_iPIN_RESET != -1;
//	}
	
	if (m_iPIN_RESET == -1) {
		//CST816_TOUCH_DEBUG_PRINTLN("No reset PIN available, disable auto-sleep");
		//setAutoSleep(false);	//since we're not able to reset.. Ensure the chip does not auto sleep (so we keep able to communicate to the touch controller)
	} else {
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

	if (m_iPIN_INTERRUPT == -1) {
		CST816_TOUCH_DEBUG_PRINTLN("CST816Touch::begin - no interrupt pin defined");
	} else {
		if (bOk) {
			CST816_TOUCH_DEBUG_PRINTLN("Configuring touch interrupt");
			pinMode(m_iPIN_INTERRUPT, INPUT_PULLUP);
			attachInterrupt(
				m_iPIN_INTERRUPT, [] { g_bTouchInterrupt = true; }, FALLING);
		}
	}

	return bOk;
}


CST816Touch::CST816Touch() {
	m_pI2C = 0;
	m_iPIN_RESET = -1;
	m_iPIN_INTERRUPT = -1;
	m_uiCTS816S_I2C_ADDRESS = 0;
	
	memset(m_ucBuffer, 0, sizeof(m_ucBuffer));

	m_ulLastTouchTime = 0;
	
	m_iTouchButtonX = TOUCH_BUTTON_X;	//init to T-Display default
	m_iTouchButtonY = TOUCH_BUTTON_Y;	//init to T-Display default
	
	m_bSwapXY = true;	
	m_eOperatingMode = CST816Touch::touch_opering_mode_t::TOUCH_MODE_DEFAULT;	//set to the default from the chip itself
	m_bNotifyReleaseOnly = true;
	m_bNotifyMotion = false;
	
	m_ulLastGestureTime = 0;
	m_ulMovementInterval = 50;
}

CST816Touch::~CST816Touch() {
	stop();
}

}	//namespace end
