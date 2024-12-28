#include "CST816Touch_SWMode.h"

#include "GestureFactory.h"
#include "DoubleClickFactory.h"
#include "ElegantDoubleClickFactory.h"
#include "TouchScreenEventCache.h"

#include "DebugMsg.h"

namespace MDO {

void CST816Touch_SWMode::enableDoubleClickFactory(DoubleClickFactory* pDCF, int iDoubleClickTime_msec) {
	if ((m_pDoubleClickFactory == 0) && //if not already set
		(pDCF != 0)) {					//and usefull to set
		m_pDoubleClickFactory = pDCF;	//store for ownership
		m_pDoubleClickFactory->begin(iDoubleClickTime_msec);
		bool bWithPriority = true;
		registerTouchScreenEventProcessor(m_pDoubleClickFactory, bWithPriority);
	} else {
		CST816_TOUCH_DEBUG_PRINTLN("enableDoubleClickFactory - ignoring this call..");
		if (pDCF != 0) {
			delete pDCF;
		}
	}
}

/**
 * Sets the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
 * Only useful when setNotifyOnMovement has enabled this behavior first
 */
void CST816Touch_SWMode::setMovementInterval(unsigned long ulMovementInterval) {
	CST816Touch::setMovementInterval(ulMovementInterval);
}

/**
 * Enable / disable events during movement.
 * When this method is not used, default is no events during movement
 */
bool CST816Touch_SWMode::setNotifyOnMovement(bool bMovementNotificationsRequired /*= true*/) {
	return CST816Touch::setNotifyOnMovement(bMovementNotificationsRequired);
}

/**
 * get notifications on touch events and release event
 */
bool CST816Touch_SWMode::setNotificationsOnAllEvents() {
	return CST816Touch::setNotificationsOnAllEvents();
}

/**
 * get notifications on release event only - this is the default
 * When enabled, invalidates setNotifyOnMovement(true)
 */
void CST816Touch_SWMode::setNotificationsOnReleaseOnly() {
	CST816Touch::setNotificationsOnReleaseOnly();
}

/**
 * Get our GestureFactory. Note: can be a null pointer..
 */
GestureFactory*	CST816Touch_SWMode::getGestureFactory() {
	return m_pGestureFactory;
}

/*virtual*/ void CST816Touch_SWMode::control() {
	CST816Touch::control();
}

void CST816Touch_SWMode::enableDoubleClickFactory_Elegant(int iDoubleClickTime_msec /*= 500*/) {
	enableDoubleClickFactory(new ElegantDoubleClickFactory(this), iDoubleClickTime_msec);
}

void CST816Touch_SWMode::enableDoubleClickFactory_Quick(int iDoubleClickTime_msec /*= 500*/) {
	enableDoubleClickFactory(new DoubleClickFactory(this), iDoubleClickTime_msec);
}

/**
 * For chip configurations which do not support gestures, use a GestureFactory to make some gestures from touch events
 * This does require that the chip gives us all events (touch & release)
 * Note: 
 *   -no disable methode for this available
 *   -this method only works once
 */
void CST816Touch_SWMode::enableGestureFactory(	int iScreenWidth /*= 0*/, int iScreenHeight /*= 0*/, 
												int iLongPressTime_sec /*= 3*/,
												int iLongPressMaxDistance /*= -1*/, int iHorSwipeMinDistance /*= -1*/, int iVertSwipeMinDistance /*= -1*/) {
	if (m_pGestureFactory == 0) {
		CST816_TOUCH_DEBUG_PRINTLN("enableGestureFactory");
	
		m_pGestureFactory = new GestureFactory(this);
		m_pGestureFactory->begin(iScreenWidth, iScreenHeight, iLongPressTime_sec, iLongPressMaxDistance, iHorSwipeMinDistance, iVertSwipeMinDistance);
		setNotificationsOnAllEvents();	//this must be on for the GestureFactory to work. MDO: for later, see if we can obey the users wishes if that was not the case..
		setNotifyOnMovement();			//same here
		registerTouchScreenEventProcessor(m_pGestureFactory);
	} else {
		CST816_TOUCH_DEBUG_PRINTLN("enableGestureFactory - ignoring this call..");
	}
}

/**
 * begin: this is a mandatory call to be made.
 * Please ensure that Wire has been initialized before this call, using the Wire.begin method
 * For each PIN which this class should not use / change: set to -1
 * For max power efficiency, the interrupt pin is mandatory. If this pin is not set: disables auto sleep.
 * return false on issues
 */
bool CST816Touch_SWMode::begin(		TwoWire& w, 
									TouchScreenObserver* pTouchScreenObserver, 
									int PIN_INTERRUPT 			/* = 16 */, 
									int PIN_RESET 				/* = 21 */, 
									uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/) {
	bool bOk =	CST816Touch::begin(w, pTouchScreenObserver, PIN_INTERRUPT, PIN_RESET, CTS816S_I2C_ADDRESS) &&
				setOperatingModeFast();
			
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
bool CST816Touch_SWMode::begin(		TwoWire& w, 
									int PIN_INTERRUPT 			/* = 16 */, 
									int PIN_RESET 				/* = 21 */, 
									uint8_t CTS816S_I2C_ADDRESS /*= 0x15*/) {
	return begin(w, TouchScreenEventCache::getInstance(), PIN_INTERRUPT, PIN_RESET, CTS816S_I2C_ADDRESS);
}

CST816Touch_SWMode::CST816Touch_SWMode() {
	m_pGestureFactory = 0;
	m_pDoubleClickFactory = 0;
}

CST816Touch_SWMode::~CST816Touch_SWMode() {
	if (m_pGestureFactory != 0) {
		delete m_pGestureFactory;
	}
	m_pGestureFactory = 0;
	
	if (m_pDoubleClickFactory != 0) {
		delete m_pDoubleClickFactory;
	}
	m_pDoubleClickFactory = 0;
}

}	//namespace end