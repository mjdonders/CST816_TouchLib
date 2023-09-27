#ifndef _MDO_DummyTouchSubscriber_H
#define _MDO_DummyTouchSubscriber_H

#include "TouchSubscriberInterface.h"

namespace MDO {

/**
 * A dummy implementation of the TouchSubscriberInterface.
 * It shows how that works, by just printing the relevant events Serial.print statements
 */ 
class DummyTouchSubscriber: public TouchSubscriberInterface {
	
	private:
	
	private:
	
	public:
		//the pointer is the 'this' from the CST816Touch source 
		virtual void	gestureNotification(CST816Touch* pTouch, int iGestureId, bool bReleasedScreen);
		
		//the pointer is the 'this' from the CST816Touch source 
		virtual void	touchNotification(CST816Touch* pTouch, int x, int y, bool bReleasedScreen);
	
		DummyTouchSubscriber();
		virtual ~DummyTouchSubscriber();
};

}	//namespace end

#endif
