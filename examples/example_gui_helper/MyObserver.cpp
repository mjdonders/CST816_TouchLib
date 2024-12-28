#include "MyObserver.h"

#include <CST816_TouchLib.h>

namespace MDO {

/*virtual*/ void MyObserver::guiNotification(const String& strGuiIdName, TouchScreenGuiObserver::gui_touch_mode_t eTouchMode, unsigned int iPageId) {
	Serial.print("GUI notification on page ");
	Serial.print(iPageId);
	Serial.print(" for element: '");
	Serial.print(strGuiIdName);
	Serial.print("' using touch event type: ");
	Serial.println(toString(eTouchMode));
}

/*virtual*/ void MyObserver::gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed) {
	
	if ((iGestureId == (int)TouchScreenController::gesture_t::GESTURE_LONG_PRESS) || 
		(iGestureId == (int)TouchScreenController::gesture_t::GESTURE_DOUBLE_CLICK)) {
		//these gestures are handled by the TouchScreenGuiHelper
		return;
	}
		
	Serial.print("Gesture detected: ");	
	Serial.println(TouchScreenController::gestureIdToString((TouchScreenController::gesture_t)iGestureId));
	
	if (iGestureId == (int)TouchScreenController::gesture_t::GESTURE_LEFT) {
		m_uiCurrentPageId++;
		Serial.print("Active page is now: ");
		Serial.println(m_uiCurrentPageId);
		
	} else if (iGestureId == (int)TouchScreenController::gesture_t::GESTURE_RIGHT) {
		if (m_uiCurrentPageId > 0) {
			m_uiCurrentPageId--;
			Serial.print("Active page is now: ");
			Serial.println(m_uiCurrentPageId);
		}
	}
	
	if (m_pTouchScreenGuiHelper != 0) {
		m_pTouchScreenGuiHelper->setCurrentPage(m_uiCurrentPageId);
	}
}

/*virtual*/ void MyObserver::touchNotification(int x, int y, bool bCurrentlyPressed) {
	//just ignore, since handled by the TouchScreenGuiHelper
    if (!bCurrentlyPressed) {
		//Serial.println(String("Touch ignored (") + x + "," + y + ")");
	}
}

void MyObserver::begin(TouchScreenGuiHelper* pTouchScreenGuiHelper) {
	m_pTouchScreenGuiHelper = pTouchScreenGuiHelper;
	if (m_pTouchScreenGuiHelper != 0) {
		m_pTouchScreenGuiHelper->begin(this);	//ensure the GuiHelper notifies us
	}
}

MyObserver::MyObserver() {
	m_uiCurrentPageId = 0;
	m_pTouchScreenGuiHelper = 0;
}

MyObserver::~MyObserver() {
}

}	//namespace end
