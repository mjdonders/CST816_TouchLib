#ifndef _MDO_MyTouchObserver_H
#define _MDO_MyTouchObserver_H

#include "TouchScreenController.h"
#include "TouchScreenObserver.h"


namespace MDO {

/**
 * 
 */ 
class MyTouchObserver: public TouchScreenObserver {
	
	private:
	
	private:
	
	public:
		virtual void	gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed);
		virtual void	touchNotification(int x, int y, bool bCurrentlyPressed);
	
		MyTouchObserver();
		virtual ~MyTouchObserver();
};

}	//namespace end

#endif
