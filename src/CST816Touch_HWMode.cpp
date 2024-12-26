#include "CST816Touch_HWMode.h"
#include "TouchScreenObserver.h"
#include "TouchScreenEventCache.h"

#include "DebugMsg.h"

namespace MDO {

bool CST816Touch_HWMode::setLongPressTime(uint8_t uiLongPressTime) {
	return setHardwareLongPressTime(uiLongPressTime);
}

/*virtual*/ void CST816Touch_HWMode::control() {
	CST816Touch::control();
}

/**
 * begin: this is a mandatory call to be made.
 * Please ensure that Wire has been initialized before this call, using the Wire.begin method
 * For each PIN which this class should not use / change: set to -1
 * For max power efficiency, the interrupt pin is mandatory. If this pin is not set: disables auto sleep.
 * return false on issues
 */
bool CST816Touch_HWMode::begin(		TwoWire& w, 
									TouchScreenObserver* pTouchScreenObserver, 
									int PIN_INTERRUPT 			/* = 16 */, 
									int PIN_RESET 				/* = 21 */, 
									uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/) {
	bool bOk =	CST816Touch::begin(w, pTouchScreenObserver, PIN_INTERRUPT, PIN_RESET, CTS816S_I2C_ADDRESS) &&
				setOperatingModeHardwareBased();
				
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

/**
 * begin: this is a mandatory call to be made.
 * This is the begin which does not have a Observer provided. Assuming the TouchScreenEventCache is required.
 * Please ensure that Wire has been initialized before this call, using the Wire.begin method
 * For each pin which this class should not use / change: set to -1
 * For max power efficiency, the interrupt pin is mandatory. If this pin is not set: disables auto sleep.
 * return false on issues
 */
bool CST816Touch_HWMode::begin(		TwoWire& w, 
									int PIN_INTERRUPT 			/* = 16 */, 
									int PIN_RESET 				/* = 21 */, 
									uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/) {
	return begin(w, TouchScreenEventCache::getInstance(), PIN_INTERRUPT, PIN_RESET, CTS816S_I2C_ADDRESS);
}

CST816Touch_HWMode::CST816Touch_HWMode() {
}

CST816Touch_HWMode::~CST816Touch_HWMode() {
}

}	//namespace end