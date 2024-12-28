#include "TouchScreenController.h"

#include "DebugMsg.h"


namespace MDO {

/*virtual*/ void TouchScreenController::notifyObservers(int iGestureId, int x, int y, bool bCurrentlyPressed) const {
	if (m_bInvertY) {
		y = m_iVerticalResolution - y;
	}
	if (y < 0) {
		CST816_TOUCH_DEBUG_PRINTLN("Invalid y position detected. Please check provided vertical resolution");
	}
	TouchScreenSubject::notifyObservers(iGestureId, x, y, bCurrentlyPressed);
}

/*virtual*/ void TouchScreenController::notifyObservers(int x, int y, bool bCurrentlyPressed) const {
	if (m_bInvertY) {
		y = m_iVerticalResolution - y;
	}
	if (y < 0) {
		CST816_TOUCH_DEBUG_PRINTLN("Invalid y position detected. Please check provided vertical resolution");
	}
	TouchScreenSubject::notifyObservers(x, y, bCurrentlyPressed);
}

/*static*/ String TouchScreenController::deviceTypeToString(TouchScreenController::device_type_t eDeviceType) {
	
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
/*static*/ String TouchScreenController::gestureIdToString(gesture_t eGestureId) {

	switch (eGestureId) {
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
	
	return String("UNKNOWN: ") + String((int)eGestureId);
}

/**
 * default is T-Display which needs swapping the XY coordinates
 */
void TouchScreenController::setSwapXY(bool bSwapXY /*= true*/) {
	m_bSwapXY = bSwapXY;
}

/**
 * Can be used to invert vertical coordinates.
 * When set to true, iVerticalResolution is required.
 */
bool TouchScreenController::setInvertY(bool bInvertY, int iVerticalResolution) {

	if (bInvertY) {
		if (iVerticalResolution > 50) {	//some simple check..
			m_iVerticalResolution = iVerticalResolution;
			m_bInvertY = true;
		}
	} else {
		m_bInvertY = false;
	}
	
	return m_bInvertY == bInvertY;
}

/*virtual*/ void TouchScreenController::control() {
	TouchScreenSubject::control();
}

TouchScreenController::TouchScreenController() {
	m_bSwapXY = true;	
	m_bInvertY = false;
	m_iVerticalResolution = 0;
}

TouchScreenController::~TouchScreenController() {
}

}	//namespace end