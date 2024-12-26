#include "TouchScreenController.h"




namespace MDO {

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

/*virtual*/ void TouchScreenController::control() {
	TouchScreenSubject::control();
}

TouchScreenController::TouchScreenController() {
}

TouchScreenController::~TouchScreenController() {
}

}	//namespace end