#ifndef _MDO_TouchScreenEventProcessor_H
#define _MDO_TouchScreenEventProcessor_H

#include "TouchScreenController.h"

namespace MDO {

class TouchScreenSubject;

/**
 * Processes a TouchScreen event.
 * Gets the same events as a TouchScreenObserver, but for a TouchScreenObserver these events are read-only. That's not the case here.
 */ 
class TouchScreenEventProcessor {
	
	private:
		TouchScreenSubject*	m_pParent;
	
	protected:
		TouchScreenSubject*	getTouchScreenSubject();
	
	public:
		//will retreive detected gestures
		//bCurrentlyPressed indicates if the screen is currently being pressed, or not
		//x and y might be valid (/useful). Typically: for long press and double click
		//returns true when handled fully (read: processing stops here)
		virtual bool		gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed);	
		
		//will retrieve detected touch events
		//bCurrentlyPressed indicates if the screen is currently being pressed, or not
		//returns true when handled fully (read: processing stops here)
		virtual bool		touchNotification(int x, int y, bool bCurrentlyPressed) = 0;
		virtual void		resetState();
		virtual void		control();	//can be used to do 'own processing'	
	
		TouchScreenEventProcessor(TouchScreenSubject* pParent);
		virtual ~TouchScreenEventProcessor();
};

}	//namespace end

#endif