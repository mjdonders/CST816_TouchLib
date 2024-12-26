#ifndef _MDO_DoubleClickFactory_H
#define _MDO_DoubleClickFactory_H

#include "TouchScreenEventProcessor.h"

namespace MDO {

/**
 * Generates double click events from touchNotification events, might be useful in the case that the chip does not support this.
 * This implementation is the fast one. It just passes through each touch, and if quick enough it will *also* result in a double click.
 * For the more elegant (but slower) option, see ElegantDoubleClickFactory
 */ 
class DoubleClickFactory: public TouchScreenEventProcessor {
	
	private:
		int				m_iDoubleClickTime_msec;
		unsigned long	m_ulClickTime;			//the release is used for the double click check
		
	protected:
		int				getDoubleClickTime() const;	//in msec
	
	public:
		
		//will retrieve detected touch events, this is our input
		virtual bool		touchNotification(int x, int y, bool bCurrentlyPressed);
		
		virtual void		resetState();

		void				begin(int iDoubleClickTime_msec = 500);
		
		DoubleClickFactory(TouchScreenSubject* pParent);
		virtual ~DoubleClickFactory();
};

}	//namespace end

#endif