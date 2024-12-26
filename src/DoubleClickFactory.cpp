#include "DoubleClickFactory.h"

#include "Tools.h"
#include "CST816Touch.h"
#include "TouchScreenSubject.h"
#include <Arduino.h>

#include "DebugMsg.h"

namespace MDO {

int DoubleClickFactory::getDoubleClickTime() const {
	return m_iDoubleClickTime_msec;
}

//will retrieve detected touch events
//bCurrentlyPressed indicates if the screen is currently being pressed, or not
//returns true when handled fully (read: processing stops here)
/*virtual*/ bool DoubleClickFactory::touchNotification(int x, int y, bool bCurrentlyPressed) {
	
	if (!bCurrentlyPressed) {
		if (Tools::millisDiff(m_ulClickTime) < m_iDoubleClickTime_msec) {				
			//CST816_TOUCH_DEBUG_PRINTLN("Creating a software based double click event");
			getTouchScreenSubject()->notifyObserversWithoutProcessor(CST816Touch::gesture_t::GESTURE_DOUBLE_CLICK, x, y, bCurrentlyPressed);
			
			//the following return does assume that the second click of this double click does not count for a new double click
			//I think that that would make sense..
			return true;
		}				
		m_ulClickTime = millis();		
	}
	return false;
}

/*virtual*/ void DoubleClickFactory::resetState() {
	m_ulClickTime = 0;
}

void DoubleClickFactory::begin(int iDoubleClickTime_msec /* = 500 */) {	//the Windows default, according to wikipedia
	if ((iDoubleClickTime_msec > 100) && (iDoubleClickTime_msec < 2000)) {	//some basic 'sanity checks'
		m_iDoubleClickTime_msec = iDoubleClickTime_msec;
	}
}

DoubleClickFactory::DoubleClickFactory(TouchScreenSubject* pParent):
TouchScreenEventProcessor(pParent) {
	m_iDoubleClickTime_msec = 500;
	resetState();
}

DoubleClickFactory::~DoubleClickFactory() {
}

}	//namespace end