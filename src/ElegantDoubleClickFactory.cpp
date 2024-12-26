#include "ElegantDoubleClickFactory.h"

#include "Tools.h"
#include "CST816Touch.h"
#include "TouchScreenSubject.h"
#include <Arduino.h>

#include "DebugMsg.h"

namespace MDO {

bool ElegantDoubleClickFactory::hasPendingPressEvent() const {
	return (m_iPrevPressX >= 0) && (m_iPrevPressY >= 0);
}

bool ElegantDoubleClickFactory::hasPendingReleaseEvent() const {
	return (m_iPrevReleaseX >= 0) && (m_iPrevReleaseY >= 0);
}

//will retrieve detected touch events
//bCurrentlyPressed indicates if the screen is currently pressed, or not
//returns true when handled fully (read: processing stops here)
/*virtual*/ bool ElegantDoubleClickFactory::touchNotification(int x, int y, bool bCurrentlyPressed) {
	if (bCurrentlyPressed) {
		//press event
		//cache the press event for now, to keep the sequence of events
		if (m_ulPrevPressTime == 0) {		//only store the first, since there might be many..
			m_ulPrevPressTime = millis();
			m_iPrevPressX = x;
			m_iPrevPressY = y;
		}
		return true;	//yes, this event is now fully handled

	} else {
		//release event
		//also the release event is cached for now, to keep the sequence of events
		if (!hasPendingReleaseEvent()) {
			m_iPrevReleaseX = x;
			m_iPrevReleaseY = y;
			m_ulPrevReleaseTime = millis();
			return true;	//we cache this touch for now, since this was the first, there is no double click yet
		} else {
			if ((m_ulPrevReleaseTime != 0) && 
				(Tools::millisDiff(m_ulPrevReleaseTime) < getDoubleClickTime())) {	//when ::control is called frequent enough, this is always true
				getTouchScreenSubject()->notifyObserversWithoutProcessor(CST816Touch::gesture_t::GESTURE_DOUBLE_CLICK, x, y, false);
				resetState();
				return true;	//since we generated a gesture event: consider this fully handled
			}
		}
	}
	CST816_TOUCH_DEBUG_PRINTLN("ElegantDoubleClickFactory::touchNotification - not handling this");
	return false;	//it's quite unlikely to ever reach this point..
}

/*virtual*/ void ElegantDoubleClickFactory::resetState() {
	m_ulPrevPressTime = 0;
	m_iPrevPressX = -1;
	m_iPrevPressY = -1;
	
	m_ulPrevReleaseTime = 0;
	m_iPrevReleaseX = -1;
	m_iPrevReleaseY = -1;
}

//please call often, needed for all timing based gestures
/*virtual*/ void ElegantDoubleClickFactory::control() {

	if (hasPendingPressEvent()) {
		if (Tools::millisDiff(m_ulPrevPressTime) >= getDoubleClickTime()) {
			//we have a pending touch (press). however too late to become a double click
			//transfer the original event
			//CST816_TOUCH_DEBUG_PRINTLN("Release cached press event");
			getTouchScreenSubject()->notifyObserversWithoutProcessor(m_iPrevPressX, m_iPrevPressY, true, this);	//true, since this is the (delayed) press event
			//and clear state to get ready for the next event
			m_ulPrevPressTime = 0;
			m_iPrevPressX = -1;
			m_iPrevPressY = -1;
		}		
	}
	
	if (hasPendingReleaseEvent()) {
		if (Tools::millisDiff(m_ulPrevReleaseTime) >= getDoubleClickTime()) {
			//we have a pending touch (release). however too late to become a double click
			//transfer the original event
			//CST816_TOUCH_DEBUG_PRINTLN("Release cached release event");
			getTouchScreenSubject()->notifyObserversWithoutProcessor(m_iPrevReleaseX, m_iPrevReleaseY, false, this);	//false, since this is the (delayed) release event
			//and clear state to get ready for the next event
			m_ulPrevReleaseTime = 0;
			m_iPrevReleaseX = -1;
			m_iPrevReleaseY = -1;
		}
	}
	
	
}

void ElegantDoubleClickFactory::begin(int iDoubleClickTime_msec /* = 500 */) {	//the Windows default, as-per wikipedia
	DoubleClickFactory::begin();
}

ElegantDoubleClickFactory::ElegantDoubleClickFactory(TouchScreenSubject* pParent):
DoubleClickFactory(pParent) {
	resetState();
}

ElegantDoubleClickFactory::~ElegantDoubleClickFactory() {
}

}	//namespace end