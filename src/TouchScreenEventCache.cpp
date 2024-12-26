#include "TouchScreenEventCache.h"




namespace MDO {

TouchScreenEventCache* TouchScreenEventCache::m_pInstance = 0;

/**
 * Get the position of the last touch.
 * Note: 'consumes the last touch', meaning it will reset this info, to minimize the chance of a dual processing of a single event
 * When x=y=0, please assume no touch ever happened.
 */
void TouchScreenEventCache::getLastTouchPosition(int& x, int& y) {
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
void TouchScreenEventCache::getLastGesture(TouchScreenController::gesture_t& gesture, int& x, int& y, bool& bCurrentlyPressed) {
	if (!m_bGestureConsumed) {
		m_bGestureConsumed = true;
		x = m_iLastGestureX;
		y = m_iLastGestureY;
		gesture = m_eLastGesture;
		bCurrentlyPressed = m_bLastGesturePressedScreen;
	} else {
		gesture	= TouchScreenController::gesture_t::GESTURE_NONE;
		x = 0;
		y = 0;
		bCurrentlyPressed = false;
	}
}

/**
 * Get the position of the last gesture.
 * Note: 'consumes the last gesture', meaning it will reset this info, to minimize the chance of a dual processing of a single event
 * When x=y=0, please assume no gesture ever happened.
 * 
 * We also have m_bLastGestureReleasedScreen, but I don't think anyone will care..
 * this is implicit, based on the gesture type: for long press it's false, for all others it's true
 */
void TouchScreenEventCache::getLastGesture(TouchScreenController::gesture_t& gesture, int& x, int& y) {
	
	bool bDummy = false;
	getLastGesture(gesture, x, y, bDummy);
}

bool TouchScreenEventCache::hadTouch() const {
	return (m_iLastTouchX >= 0) && (m_iLastTouchY >= 0) && (!m_bTouchConsumed);
}

bool TouchScreenEventCache::hadGesture() const {
	return (m_eLastGesture != CST816Touch::gesture_t::GESTURE_NONE) && (!m_bGestureConsumed);
}

/*virtual*/ void TouchScreenEventCache::gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed) {
	
	m_eLastGesture = (TouchScreenController::gesture_t)iGestureId;
	m_iLastGestureX = x;
	m_iLastGestureY = y;
	m_bLastGesturePressedScreen = bCurrentlyPressed;	//note that for a long press, we detect based on time only, not when released. Therefore this is required
	m_bGestureConsumed = false;
}

/*virtual*/ void TouchScreenEventCache::touchNotification(int x, int y, bool bCurrentlyPressed) {
	if (!bCurrentlyPressed) {
		m_iLastTouchX = x;
		m_iLastTouchY = y;
		m_bTouchConsumed = false;
	}
}

/*static*/ TouchScreenEventCache* TouchScreenEventCache::getInstance() {
	if (m_pInstance == 0) {
		m_pInstance = new TouchScreenEventCache();
	}
	return m_pInstance;
}

//protected contructor
TouchScreenEventCache::TouchScreenEventCache() {
	m_bTouchConsumed = true;
	m_bGestureConsumed = true;
	m_eLastGesture = CST816Touch::gesture_t::GESTURE_NONE;
	m_iLastGestureX = -1;
	m_iLastGestureY = -1;
	
	m_iLastTouchX = -1;
	m_iLastTouchY = -1;
}

TouchScreenEventCache::~TouchScreenEventCache() {
}

}	//namespace end