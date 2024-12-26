#ifndef _MDO_TouchScreenEventCache_H
#define _MDO_TouchScreenEventCache_H

#include "CST816Touch.h"
#include "TouchScreenObserver.h"

namespace MDO {

/**
 * A cache of TouchScreen events, used for full events only (so when the screen has been released by now)
 * Used in case the call back mechanism the Observer pattern requires is not desirable
 */ 
class TouchScreenEventCache: public TouchScreenObserver {
	
	private:
		static TouchScreenEventCache* m_pInstance;
		bool						m_bTouchConsumed;
		bool						m_bGestureConsumed;

		CST816Touch::gesture_t		m_eLastGesture;					//gesture:  what
		int							m_iLastGestureX;				//          where
		int							m_iLastGestureY;				//          where
		bool						m_bLastGesturePressedScreen;	//          how

		int							m_iLastTouchX;					//touch:    where
		int							m_iLastTouchY;					//          where
		//bool						m_bLastTouchReleasedScreen;		//          how		//might be useful later, we'll see
	
	private:
	
	public:
		void			getLastTouchPosition(int& x, int& y);
		void			getLastGesture(TouchScreenController::gesture_t& gesture, int& x, int& y, bool& bCurrentlyPressed);
		void			getLastGesture(TouchScreenController::gesture_t& gesture, int& x, int& y);	//this is (most likely) the one you need, see comments
		
		bool			hadTouch() const;
		bool			hadGesture() const;
		
		virtual void	gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed);	
		virtual void	touchNotification(int x, int y, bool bCurrentlyPressed);
	
		static TouchScreenEventCache*	getInstance();
	protected:
		TouchScreenEventCache();
	public:
		virtual ~TouchScreenEventCache();
};

}	//namespace end

#endif