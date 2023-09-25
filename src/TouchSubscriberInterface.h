#ifndef _MDO_TouchSubscriberInterface_H
#define _MDO_TouchSubscriberInterface_H


namespace MDO {

class CST816Touch;

/**
 * An interface class which is used to retrieve touch notifications.
 */ 
class TouchSubscriberInterface {
	
	private:
	
	private:
	
	public:
		//will retreive detected gestures
		virtual void	gestureNotification(CST816Touch* pTouch, int iGestureId) = 0;	
		
		//will retrieve detected touch events
		virtual void	touchNotification(CST816Touch* pTouch, int x, int y) = 0;
};

}	//namespace end

#endif
