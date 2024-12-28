#include "GestureFactory.h"
#include "Tools.h"
#include "CST816Touch.h"
#include "TouchScreenSubject.h"

#include <stdlib.h>
#include <algorithm>

#include "DebugMsg.h"

namespace MDO {

bool GestureFactory::checkInitialized() const {
	return	(m_iHorSwipeMinDistance > 0) &&
			(m_iVertSwipeMinDistance > 0);
}
int GestureFactory::getDiff(int iPos1, int iPos2) const {
	return abs(iPos1 - iPos2);
}

void GestureFactory::invalidateTouchRegistration() {
	m_iStartX = 0;		//invalidate, since handled
	m_iStartY = 0;
	m_ulTouchTime = 0;
	m_eState = tGestureFactoryState::INITIALIZED;
}

/**
 * Handles a long press. Returns true when handled.
 */
bool GestureFactory::handleLongPress(int x, int y) {
	bool bWasLongPress =	(!m_bMovementSinceTouch)											&&
							(Tools::millisDiff(m_ulTouchTime) > (m_iLongPressTime_sec * 1000))	&&
							(getDiff(x, m_iStartX) <= m_iLongPressMaxDistance)					&&
							(getDiff(y, m_iStartY) <= m_iLongPressMaxDistance);
	
	if (bWasLongPress) {
		getTouchScreenSubject()->notifyObserversWithoutProcessor(CST816Touch::gesture_t::GESTURE_LONG_PRESS, x, y, true);
		invalidateTouchRegistration();
	}
	
	return bWasLongPress;
}

bool GestureFactory::handleHorizontalGesture(int x, int y) {
	bool bWasHorizontalGesture =	(getDiff(x, m_iStartX) >= m_iHorSwipeMinDistance) &&
									(getDiff(y, m_iStartY) < m_iVertSwipeMinDistance);
	//don't care about the time in this case.
	
	if (bWasHorizontalGesture) {
		bool bLeft = x < m_iStartX;
		if (bLeft) {
			getTouchScreenSubject()->notifyObserversWithoutProcessor(CST816Touch::gesture_t::GESTURE_LEFT, x, y, false);
			invalidateTouchRegistration();		
		} else {
			getTouchScreenSubject()->notifyObserversWithoutProcessor(CST816Touch::gesture_t::GESTURE_RIGHT, x, y, false);
			invalidateTouchRegistration();		
		}
	}
	
	return bWasHorizontalGesture;
}

bool GestureFactory::handleVerticalGesture(int x, int y) {
	bool bWasVerticalGesture =	(getDiff(y, m_iStartY) >= m_iVertSwipeMinDistance) &&
								(getDiff(x, m_iStartX) < m_iHorSwipeMinDistance);
	//don't care about the time in this case.
	
	if (bWasVerticalGesture) {
		bool bUp = m_bInvertGestureY ? (y < m_iStartY) : (y > m_iStartY);
		if (bUp) {
			getTouchScreenSubject()->notifyObserversWithoutProcessor(CST816Touch::gesture_t::GESTURE_UP, x, y, false);
			invalidateTouchRegistration();		
		} else {
			getTouchScreenSubject()->notifyObserversWithoutProcessor(CST816Touch::gesture_t::GESTURE_DOWN, x, y, false);
			invalidateTouchRegistration();		
		}
	} else {
		//in case vertical swipe tweaking is needed, enable the following
		//specifically for high resolution (and small) screens, this might be relevant
		//CST816_TOUCH_DEBUG_PRINTLN(String("GestureFactory - vertical diff: ") + getDiff(y, m_iStartY));
	}
	
	return bWasVerticalGesture;
}

//will retreive detected gestures
//bCurrentlyPressed indicates if the screen is currently pressed, or not
/*virtual*/ bool GestureFactory::gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed) {
	
	//this should never be called, as per the goal of this class..
	//revert to assuming this to be a touch, as workaround.
	//since a hardware based gesture might 'hide' events, this will give unpredictable behaviour
	if (iGestureId != (int)TouchScreenController::gesture_t::GESTURE_TOUCH_BUTTON) {
		//the touch_button being the only exception.. ;-)
		//since that might come from CST816Touch::handleTouch
		CST816_TOUCH_DEBUG_PRINTLN("Hardware gesture detected in GestureFactory.. If might be better to not use this class in this setup");
	}
	return touchNotification(x, y, bCurrentlyPressed);
}

//will retrieve detected touch events
//bCurrentlyPressed indicates if the screen is currently pressed, or not
/*virtual*/ bool GestureFactory::touchNotification(int x, int y, bool bCurrentlyPressed) {
	if (!isInitialized()) {
		CST816_TOUCH_DEBUG_PRINTLN("GestureFactory::touchNotification - not initialized..");
		return false;
	}
	//CST816_TOUCH_DEBUG_PRINTLN(String("touchNotification: ") + (bCurrentlyPressed?"pressed":"released"));
	
	if (m_eState == tGestureFactoryState::INITIALIZED) {
		if (bCurrentlyPressed) {
			m_iStartX = x;
			m_iStartY = y;
			m_ulTouchTime = millis();
			m_bMovementSinceTouch = false;
			m_eState = tGestureFactoryState::TOUCHED;	//the screen is being touched

			//CST816_TOUCH_DEBUG_PRINTLN("GestureFactory - touch detected");			
		}
	}
	else if (m_eState == tGestureFactoryState::TOUCHED) {
		if (!bCurrentlyPressed) {
			//CST816_TOUCH_DEBUG_PRINTLN("GestureFactory -> back to INITIALIZED");
			//ready with action. check if we think that that was a gesture.
			m_eState = tGestureFactoryState::INITIALIZED;	//the screen has just been released

			if (handleHorizontalGesture(x, y)) {
				//Serial.println("Horizontal gesture");
				return true;	//we handled it. Fully.
			}
			if (handleVerticalGesture(x, y)) {
				//Serial.println("Vertical gesture");
				return true;	//we handled it. Fully.
			}
		} else {	//still holding the screen
			if ((getDiff(x, m_iStartX) > m_iLongPressMaxDistance) || 
				(getDiff(y, m_iStartY) > m_iLongPressMaxDistance)) {
				//Movement detected, this should not be considered a long press anymore..
				m_bMovementSinceTouch = true;
			}
			if (handleLongPress(x, y)) {						//when changing, see simular code in ::control
				Serial.println("GestureFactory - long press handled - event based");
				m_eState = tGestureFactoryState::WAITING_FOR_RELEASE;
				//CST816_TOUCH_DEBUG_PRINTLN("GestureFactory -> WAITING_FOR_RELEASE");
				return true;	//we handled it. Fully.
			}
		}
	}
	else if (m_eState == tGestureFactoryState::WAITING_FOR_RELEASE) {	//this is to ensure that after a long press, we don't get more unintended stuff..
		if (!bCurrentlyPressed) {
			//CST816_TOUCH_DEBUG_PRINTLN("GestureFactory screen released -> back to INITIALIZED");
			m_eState = tGestureFactoryState::INITIALIZED;
			return true;
		}
	}
	return false;
}

/*virtual*/ void GestureFactory::resetState() {
	if (isInitialized()) {
		m_eState = tGestureFactoryState::INITIALIZED;		
	}
}

void GestureFactory::control() {
	if (m_eState == tGestureFactoryState::TOUCHED) {
		//check for long press
		//check this here, for the case when we do not get informed (in case the user keeps his/her finger really still)
		if (handleLongPress(m_iStartX, m_iStartY)) {			//when changing, see simular code in ::touchNotification
			//CST816_TOUCH_DEBUG_PRINTLN("GestureFactory - long press handled - time based");
			m_eState = tGestureFactoryState::WAITING_FOR_RELEASE;
			//CST816_TOUCH_DEBUG_PRINTLN("GestureFactory -> WAITING_FOR_RELEASE");
		}
	}
}

bool GestureFactory::isInitialized() const {
	return m_eState != tGestureFactoryState::NOT_INITIALIZED;	//anything other than not-initialized is initialized.. :-)
}

void GestureFactory::setInvertVerticalGestures(bool bInvertGestureY /*= true*/) {
	m_bInvertGestureY = bInvertGestureY;
}

void GestureFactory::begin(	int iScreenWidth          /* = 0*/, 	//screen width. only needed when *not* providing iHorSwipeMinDistance
							int iScreenHeight         /* = 0*/, 	//screen height. only needed when *not* providing iVertSwipeMinDistance
							int iLongPressTime_sec    /* = 3 */,	//long press time in seconds (to keep the interface inline with the hardware capabilities
							int iLongPressMaxDistance /* = -1 */,	//the long press maximum distance (in order to still call it a long press)
							int iHorSwipeMinDistance  /* = -1 */, 	//horizontal swipe minimum distance, leave as-is for auto-determined
							int iVertSwipeMinDistance /* = -1 */) {	//vertical swipe minimum distance, leave as-is for auto-determined

	m_iScreenWidth = iScreenWidth;
	m_iScreenHeight = iScreenHeight;
	m_iHorSwipeMinDistance = iHorSwipeMinDistance;
	m_iVertSwipeMinDistance = iVertSwipeMinDistance;
	m_iLongPressMaxDistance = iLongPressMaxDistance;
	
	if ((iLongPressTime_sec > 0) && (iLongPressTime_sec < 100)) {		//just to ensure that this in not by mistaken 'set in msec'
		m_iLongPressTime_sec = iLongPressTime_sec;
	}
	
	if (m_iLongPressMaxDistance < 0) {
		m_iLongPressMaxDistance = 10;
	}
	
	const int iMinDistance = 100;
	if (m_iHorSwipeMinDistance <= 0) {
		if (m_iScreenWidth == 0) {
			m_iHorSwipeMinDistance = iMinDistance;
		} else {
			m_iHorSwipeMinDistance = std::min(m_iScreenWidth / 3, iMinDistance);	//with 536x240 => minimum 178
		}
	}
	if (m_iVertSwipeMinDistance <= 0) {
		if (m_iScreenHeight == 0) {
			m_iVertSwipeMinDistance = iMinDistance;
		} else {
			m_iVertSwipeMinDistance = std::min(m_iScreenHeight / 4, iMinDistance);	//with 536x240 => minimum 60
		}
	}
	
	if (checkInitialized()) {
		m_eState = tGestureFactoryState::INITIALIZED;
	}
}

GestureFactory::GestureFactory(TouchScreenSubject* pParent):
TouchScreenEventProcessor(pParent) {
	m_iScreenWidth = 0;
	m_iScreenHeight = 0;
	m_iLongPressTime_sec = 3;
	m_iHorSwipeMinDistance = 0;
	m_iVertSwipeMinDistance = 0;
	m_eState = tGestureFactoryState::NOT_INITIALIZED;
	m_iStartX = 0;
	m_iStartY = 0;
	m_ulTouchTime = 0;
	m_bMovementSinceTouch = false;
	m_bInvertGestureY = false;
}

GestureFactory::~GestureFactory() {
}

}	//namespace end