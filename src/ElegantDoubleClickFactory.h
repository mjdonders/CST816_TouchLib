#ifndef _MDO_ElegantDoubleClickFactory_H
#define _MDO_ElegantDoubleClickFactory_H

#include "DoubleClickFactory.h"
#include "TouchScreenEventProcessor.h"

namespace MDO {

/**
 * Generates double click events from touchNotification events, might be useful in the case that the chip does not support this.
 * This implementation is the elegant 'slow one', buffering clicks to ensure two quick enough touches form one double-click.
 * If a click is not followed quick enough by a new click to form a double click, will re-issue the original touch event (with a delay of iDoubleClickTime_msec).
 */ 
class ElegantDoubleClickFactory: public DoubleClickFactory {
	
	private:
		int				m_iDoubleClickTime_msec;
		unsigned long	m_ulPrevReleaseTime;	//the release is used for the double click check
		int				m_iPrevReleaseX;
		int				m_iPrevReleaseY;		
		unsigned long	m_ulPrevPressTime;		//the press is also buffered, to ensure the sequence of events
		int				m_iPrevPressX;			//please note that this does not work (gracefully) with setNotifyOnMovement(true)..
		int				m_iPrevPressY;
		
	private:
		bool				hasPendingPressEvent() const;
		bool				hasPendingReleaseEvent() const;
	
	public:
		
		//will retrieve detected touch events, this is our input
		virtual bool		touchNotification(int x, int y, bool bCurrentlyPressed);
		
		virtual void		resetState();
		virtual void		control();	//please call often, needed for all timing based gestures

		void				begin(int iDoubleClickTime_msec = 500);
		
		ElegantDoubleClickFactory(TouchScreenSubject* pParent);
		virtual ~ElegantDoubleClickFactory();
};

}	//namespace end

#endif