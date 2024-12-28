#ifndef _MDO_TouchScreenObserver_H
#define _MDO_TouchScreenObserver_H

#include "TouchScreenController.h"

namespace MDO {

/**
 * Abstract class meant as interface for the outside world (receiving touch & gesture events).
 */ 
class TouchScreenObserver {
	
	public:	
	
	private:
	
	public:
		//will retreive detected gestures
		//bCurrentlyPressed indicates if the screen is currently being pressed, or not
		//x and y might be valid (/useful). Typically: for long press and double click
		virtual void	gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed) = 0;	
		
		//will retrieve detected touch events
		//bCurrentlyPressed indicates if the screen is currently being pressed, or not
		virtual void	touchNotification(int x, int y, bool bCurrentlyPressed) = 0;

	
		TouchScreenObserver();
		virtual ~TouchScreenObserver();
};

}	//namespace end

#endif