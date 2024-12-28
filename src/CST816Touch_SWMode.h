#ifndef _MDO_CST816Touch_SWMode_H
#define _MDO_CST816Touch_SWMode_H

#include "CST816Touch.h"

namespace MDO {

class GestureFactory;
class DoubleClickFactory;

/**
 * Mainly software based touch controller implementation.
 * Has more options and flexibility compared to the hardware mode implementation.
 * Depending on the [configuration available] and [implementation wishes] please check the TouchScreenEventProcessor derived classes.
 */ 
class CST816Touch_SWMode: public CST816Touch {
	
	private:
		GestureFactory*		m_pGestureFactory;
		DoubleClickFactory*	m_pDoubleClickFactory;
	
	private:
		void			enableDoubleClickFactory(DoubleClickFactory* pDCF, int iDoubleClickTime_msec);
	
	protected:
		//I'll keep the following block protected for now, since this no longer works like before
		//the only drawback I can see is that this blocks you from making a 'MS Paint'-like application..
		void			setMovementInterval(unsigned long ulMovementInterval);	//sets the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
		bool			setNotifyOnMovement(bool bMovementNotificationsRequired = true);	//for events during movement.
		bool			setNotificationsOnAllEvents();		//get notifications on touch events and release event
		void			setNotificationsOnReleaseOnly();	//get notifications on release event only - this is the *default*		
	public:
		GestureFactory*	getGestureFactory();
		
		virtual void	control();	//please call in loop()
	
		void			enableDoubleClickFactory_Elegant(int iDoubleClickTime_msec = 500);
		void			enableDoubleClickFactory_Quick(int iDoubleClickTime_msec = 500);
		void			enableGestureFactory(	int iScreenWidth = 0, int iScreenHeight = 0, 
												int iLongPressTime_sec = 3,
												int iLongPressMaxDistance = -1, int iHorSwipeMinDistance = -1, int iVertSwipeMinDistance = -1);
												
		bool			begin(TwoWire& w, TouchScreenObserver* pTouchScreenObserver,	int PIN_INTERRUPT = 16, int PIN_RESET = 21, uint8_t CTS816S_I2C_ADDRESS = 0x15);
		bool			begin(TwoWire& w, 												int PIN_INTERRUPT = 16, int PIN_RESET = 21, uint8_t CTS816S_I2C_ADDRESS = 0x15);
	
		CST816Touch_SWMode();
		virtual ~CST816Touch_SWMode();
};

}	//namespace end

#endif