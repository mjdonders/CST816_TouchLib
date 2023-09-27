#include "TouchSubscriber.h"
#include "CST816Touch.h"

//#define DEBUG
//#include <mdomisc.h>



namespace MDO {


//the pointer is the 'this' from the CST816Touch source 
/*virtual*/ void TouchSubscriber::gestureNotification(CST816Touch* pTouch, int iGestureId, bool bReleasedScreen) {
	Serial.print("Gesture");
	if (bReleasedScreen) {
		Serial.print(" release");
	}
	Serial.print(" detected: ");	
	Serial.print(CST816Touch::gestureIdToString(iGestureId));
	
	if ((pTouch != 0) && ((iGestureId == (int)CST816Touch::gesture_t::GESTURE_LONG_PRESS) || 
						  (iGestureId == (int)CST816Touch::gesture_t::GESTURE_DOUBLE_CLICK))) {
		int x = 0;
		int y = 0;
		CST816Touch::gesture_t eGesture;
		
		pTouch->getLastGesture(eGesture, x, y);
		
		Serial.print(" at: (");
		Serial.print(x);
		Serial.print(",");
		Serial.print(y);
		Serial.print(")");
	}
	
	Serial.println();
}

//the pointer is the 'this' from the CST816Touch source 
/*virtual*/ void TouchSubscriber::touchNotification(CST816Touch* pTouch, int x, int y, bool bReleasedScreen) {
	//just ignore..
}

TouchSubscriber::TouchSubscriber() {
}

TouchSubscriber::~TouchSubscriber() {
}

}	//namespace end
