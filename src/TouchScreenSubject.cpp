#include "TouchScreenSubject.h"


#include <algorithm>

#include "TouchScreenObserver.h"
#include "TouchScreenEventProcessor.h"

#include "DebugMsg.h"

namespace MDO {

//private:
//actual internal's

//gesture
/*virtual*/ void TouchScreenSubject::processTouchEvent(int iGestureId, int x, int y, bool bCurrentlyPressed) const {
	bool bFullyHandled = false;
	
	for (auto cit = m_pEventProcessors.begin(); cit != m_pEventProcessors.end(); cit++) {
		if (!bFullyHandled) {
			bFullyHandled = (*cit)->gestureNotification(iGestureId, x, y, bCurrentlyPressed);
		}
	}
	
	if (bFullyHandled) {
		//since someone handled the whole thing, ensure that nobody thinks we're half-way some touch action
		for (auto cit = m_pEventProcessors.begin(); cit != m_pEventProcessors.end(); cit++) {
			(*cit)->resetState();
		}
	}
	
	if (!bFullyHandled) {
		notifyObserversWithoutProcessor(iGestureId, x, y, bCurrentlyPressed);
	}
}

//touch	
/*virtual*/ void TouchScreenSubject::processTouchEvent(int x, int y, bool bCurrentlyPressed) const {
	bool bFullyHandled = false;
	
	for (auto cit = m_pEventProcessors.begin(); cit != m_pEventProcessors.end(); cit++) {
		if (!bFullyHandled) {
			bFullyHandled = (*cit)->touchNotification(x, y, bCurrentlyPressed);
		}
	}
	
	if (!bFullyHandled) {
		notifyObserversWithoutProcessor(x, y, bCurrentlyPressed);
	}	
}	

//interface for TouchScreenProcessors, gesture
/*virtual*/ void TouchScreenSubject::notifyObserversWithoutProcessor(int iGestureId, int x, int y, bool bCurrentlyPressed) const {
	for (auto cit = m_vpObservers.begin(); cit != m_vpObservers.end(); cit++) {
		(*cit)->gestureNotification(iGestureId, x, y, bCurrentlyPressed);
	}	
}

/**
 * Notify all, except the provided EventProcessor, that will be excluded
 */
/*virtual*/ void TouchScreenSubject::notifyObserversWithoutProcessor(int x, int y, bool bCurrentlyPressed, TouchScreenEventProcessor* pExcludedEventProcessor) const {
	
	bool bFullyHandled = false;
	
	for (auto cit = m_pEventProcessors.begin(); cit != m_pEventProcessors.end(); cit++) {
		TouchScreenEventProcessor* pCurrent = *cit;
		if ((!bFullyHandled) &&
			(pCurrent != 0) && 
			(pCurrent != pExcludedEventProcessor)) {
			bFullyHandled = pCurrent->touchNotification(x, y, bCurrentlyPressed);
		}
	}
	
	if (!bFullyHandled) {
		notifyObserversWithoutProcessor(x, y, bCurrentlyPressed);
	}
}

//interface for TouchScreenProcessors, touch
/*virtual*/ void TouchScreenSubject::notifyObserversWithoutProcessor(int x, int y, bool bCurrentlyPressed) const {
	for (auto cit = m_vpObservers.begin(); cit != m_vpObservers.end(); cit++) {
		(*cit)->touchNotification(x, y, bCurrentlyPressed);
	}
}	

//protected:

/**
 * Notify all Observers for this gesture event, if the TouchScreenProcessors allow
 */
/*virtual*/ void TouchScreenSubject::notifyObservers(int iGestureId, int x, int y, bool bCurrentlyPressed) const {
	processTouchEvent(iGestureId, x, y, bCurrentlyPressed);
}

/**
 * Notify all Observers for this touch event, if the TouchScreenProcessors allow
 */
/*virtual*/ void TouchScreenSubject::notifyObservers(int x, int y, bool bCurrentlyPressed) const {
	processTouchEvent(x, y, bCurrentlyPressed);
}

void TouchScreenSubject::registerTouchScreenEventProcessor(TouchScreenEventProcessor* pTouchScreenEventProcessor, bool bWithPriority /*= false*/) {
	if (pTouchScreenEventProcessor != 0) {
		CST816_TOUCH_DEBUG_PRINTLN("registerTouchScreenEventProcessor");
		if (bWithPriority) {
			m_pEventProcessors.insert(m_pEventProcessors.begin(), pTouchScreenEventProcessor);
		} else {
			m_pEventProcessors.push_back(pTouchScreenEventProcessor);
		}
	}
}

void TouchScreenSubject::unregisterTouchScreenEventProcessor(TouchScreenEventProcessor* pTouchScreenEventProcessor) {
	if ((pTouchScreenEventProcessor != 0) && (!m_pEventProcessors.empty())) {
		auto position = std::find(m_pEventProcessors.begin(), m_pEventProcessors.end(), pTouchScreenEventProcessor);
		if (position != m_pEventProcessors.end()) {
			m_pEventProcessors.erase(position);
		}
	}	
}

//public:

/**
 * Registers an Obeserver. At least one call to this method is mandatory
 */
void TouchScreenSubject::registerObserver(TouchScreenObserver* pObserver) {
	if (pObserver != 0) {
		CST816_TOUCH_DEBUG_PRINTLN("registerObserver");
		m_vpObservers.push_back(pObserver);
	}
}

/**
 * Removes an Observer from the list. 
 */
void TouchScreenSubject::unregisterObserver(TouchScreenObserver* pObserver) {
	if ((pObserver != 0) && (!m_vpObservers.empty())) {
		auto position = std::find(m_vpObservers.begin(), m_vpObservers.end(), pObserver);
		if (position != m_vpObservers.end()) {
			m_vpObservers.erase(position);
		}
	}
}

/**
 * Give all the TouchScreenEventProcessor's some processing time, if needed
 */
/*virtual*/ void TouchScreenSubject::control() {
	for (auto cit = m_pEventProcessors.begin(); cit != m_pEventProcessors.end(); cit++) {
		(*cit)->control();
	}	
}

TouchScreenSubject::TouchScreenSubject() {
}

TouchScreenSubject::~TouchScreenSubject() {
}

}	//namespace end