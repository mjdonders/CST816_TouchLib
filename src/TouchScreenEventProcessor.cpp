#include "TouchScreenEventProcessor.h"




namespace MDO {

/**
 * get's our 'parent' TouchScreenSubject
 * protected.
 */
TouchScreenSubject* TouchScreenEventProcessor::getTouchScreenSubject() {
	return m_pParent;
}

//returns true when handled fully (read: processing stops here)
/*virtual*/ bool TouchScreenEventProcessor::gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed) {
	return false;
}

/*virtual*/ void TouchScreenEventProcessor::resetState() {
	//default implementation..
}

/**
 * can be used to do 'own processing'
 */
/*virtual*/ void TouchScreenEventProcessor::control() {
	//default implementation..
}

TouchScreenEventProcessor::TouchScreenEventProcessor(TouchScreenSubject* pParent) {
	m_pParent = pParent;
}

TouchScreenEventProcessor::~TouchScreenEventProcessor() {
}

}	//namespace end