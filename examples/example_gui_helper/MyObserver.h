#ifndef _MDO_MyObserver_H
#define _MDO_MyObserver_H

//#include "TouchScreenController.h"
//#include "TouchScreenObserver.h"
//#include "TouchScreenGuiObserver.h"
#include <CST816_TouchLib.h>


namespace MDO {

class TouchScreenGuiHelper;

/**
 * MyObserver example class, deriving from:
 * 		-TouchScreenObserver: for touches & gestures
 * 		-TouchScreenGuiObserver: for specific GUI elements
 */ 
class MyObserver:   public TouchScreenObserver, public TouchScreenGuiObserver { 
	
	private:
		unsigned int 			m_uiCurrentPageId;
		TouchScreenGuiHelper*	m_pTouchScreenGuiHelper;
	
	private:
	
	public:
		//parsed GUI events
		virtual void	guiNotification(const String& strGuiIdName, TouchScreenGuiObserver::gui_touch_mode_t eTouchMode, unsigned int iPageId);
	
		//all gestures
		virtual void	gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed);
		
		//all touch events
		virtual void	touchNotification(int x, int y, bool bCurrentlyPressed);
		
		void			begin(TouchScreenGuiHelper* pTouchScreenGuiHelper);
	
		MyObserver();
		virtual ~MyObserver();
};

}	//namespace end

#endif
