#ifndef _MDO_TouchSubscriber_H
#define _MDO_TouchSubscriber_H

#include "TouchSubscriberInterface.h"

namespace MDO {

/**
 * 
 */ 
class TouchSubscriber: public TouchSubscriberInterface {
	
	private:
	
	private:
	
	public:
		//the pointer is the 'this' from the CST816Touch source 
		virtual void	gestureNotification(CST816Touch* pTouch, int iGestureId);	
		
		//the pointer is the 'this' from the CST816Touch source 
		virtual void	touchNotification(CST816Touch* pTouch, int x, int y);
	
		TouchSubscriber();
		virtual ~TouchSubscriber();
};

}	//namespace end

#endif
