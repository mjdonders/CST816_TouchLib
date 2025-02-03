#ifndef _MDO_TouchScreenGuiObserver_H
#define _MDO_TouchScreenGuiObserver_H

#include <Arduino.h>

namespace MDO {

/**
 * Abstract interface class meant to receive GUI notifications
 */ 
class TouchScreenGuiObserver {
	
	public:	//types
		enum gui_touch_mode_t {
			CLICK,
			DOUBLE_CLICK,
			LONG_PRESS,
			GESTURE_LEFT,
			GESTURE_RIGHT,
			GESTURE_UP,
			GESTURE_DOWN			
		};
	
	private:
	
	public:
		static String	toString(const gui_touch_mode_t& eTouchMode);

		virtual void	guiNotification(const String& strGuiIdName, gui_touch_mode_t eTouchMode, unsigned int iPageId) = 0;
	
		TouchScreenGuiObserver();
		virtual ~TouchScreenGuiObserver();
};

}	//namespace end

#endif