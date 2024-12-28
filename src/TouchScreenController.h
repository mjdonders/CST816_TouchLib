#ifndef _MDO_TouchScreenController_H
#define _MDO_TouchScreenController_H

#include <Arduino.h>

#include "TouchScreenSubject.h"

namespace MDO {

/**
 * Abstract base class for all touch screen controllers.
 * The idea to have this class is mainly to have a unified interface as much as possible to allow for 
 * adding touch screen controllers later without a big interface change. Hopefully.
 */ 
class TouchScreenController: public TouchScreenSubject {
	
	public:	//types
		
		enum touch_t {						//'iEventFlag', from TOUCH_REGISTER_TOUCH_XH
			TOUCH_DOWN      = 0,			//Touch press
			TOUCH_UP        = 1,			//Touch release
			TOUCH_CONTACT   = 2				//Touch contact
		};

		enum gesture_t {					//when updating, don't forget the GestureIdToString method..
			GESTURE_NONE = 0,
			GESTURE_RIGHT,					//officially: up, but I think that's an orientation thingy
			GESTURE_LEFT,					//officiallly: down, again: orientation
			GESTURE_DOWN,					//officially: left, again: orientation
			GESTURE_UP,						//officially: right, again: orientation
			GESTURE_TOUCH_BUTTON = 5,		// or 'single click'	//officially: click
			GESTURE_DOUBLE_CLICK = 0x0B,	//depending on the operational mode, this might be software determined
			GESTURE_LONG_PRESS = 0x0C
		};
		
		enum device_type_t {
			DEVICE_UNKNOWN,
			DEVICE_CST716	= 0x20,
			DEVICE_CST816S	= 0xB4,
			DEVICE_CST816T	= 0xB5,
			DEVICE_CST816D	= 0xB6
		};	
	
	protected:
		bool			m_bSwapXY;
		bool			m_bInvertY;
		int				m_iVerticalResolution;	
	
	protected:
		virtual void	notifyObservers(int iGestureId, int x, int y, bool bCurrentlyPressed) const;	//gesture
		virtual void	notifyObservers(int x, int y, bool bCurrentlyPressed) const;					//touch
		
	public:
		static String	deviceTypeToString(device_type_t eDeviceType);
		static String	gestureIdToString(gesture_t eGesture);
		
		void			setSwapXY(bool bSwapXY = true);		//default is T-Display which needs swapping the XY coordinates
		bool			setInvertY(bool bInvertY, int iVerticalResolution);		

		virtual void	control();	//please call in loop()
	
	protected:
		TouchScreenController();
	public:
		virtual ~TouchScreenController();
};

}	//namespace end

#endif