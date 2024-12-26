#include "MyTouchObserver.h"

namespace MDO {


/*virtual*/ void MyTouchObserver::gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed) {
	Serial.print("Gesture ");
//	if (!bCurrentlyPressed) {
//		Serial.print("release");
//	}
	Serial.print("detected: ");	
	Serial.print(TouchScreenController::gestureIdToString((TouchScreenController::gesture_t)iGestureId));
	
	if ((iGestureId == (int)TouchScreenController::gesture_t::GESTURE_LONG_PRESS) || 
		(iGestureId == (int)TouchScreenController::gesture_t::GESTURE_DOUBLE_CLICK)) {
		
		Serial.print(" at: (");
		Serial.print(x);
		Serial.print(",");
		Serial.print(y);
		Serial.print(")");
	}
	
	Serial.println();
}

/*virtual*/ void MyTouchObserver::touchNotification(int x, int y, bool bCurrentlyPressed) {
	//just ignore..
    //Serial.println("Touch ignored");
}

MyTouchObserver::MyTouchObserver() {
}

MyTouchObserver::~MyTouchObserver() {
}

}	//namespace end
