#ifndef _MDO_TouchScreenSubject_H
#define _MDO_TouchScreenSubject_H

#include <vector>

//#include "TouchScreenEventProcessor.h"

namespace MDO {

class TouchScreenObserver;
class TouchScreenEventProcessor;

/**
 * interface class meant as 'Subject'. 
 * See here: https://en.wikipedia.org/wiki/Observer_pattern
 */ 
class TouchScreenSubject {
	
	friend class TouchScreenEventProcessor;
	friend class DoubleClickFactory;
	friend class ElegantDoubleClickFactory;
	friend class GestureFactory;
	
	private:
		std::vector<TouchScreenEventProcessor*>	m_pEventProcessors;
		std::vector<TouchScreenObserver*>		m_vpObservers;
	
	private:
		//actual internal's
		virtual void	processTouchEvent(int iGestureId, int x, int y, bool bCurrentlyPressed) const;	//gesture
		virtual void	processTouchEvent(int x, int y, bool bCurrentlyPressed) const;					//touch		

		//interface for TouchScreenProcessors
		virtual void	notifyObserversWithoutProcessor(int iGestureId, int x, int y, bool bCurrentlyPressed) const;										//gesture
		virtual void	notifyObserversWithoutProcessor(int x, int y, bool bCurrentlyPressed, TouchScreenEventProcessor* pExcludedEventProcessor) const;	//touch		
		virtual void	notifyObserversWithoutProcessor(int x, int y, bool bCurrentlyPressed) const;														//touch		
		
	protected:
		//interface for TouchController (CST816 and the likes of that)
		virtual void	notifyObservers(int iGestureId, int x, int y, bool bCurrentlyPressed) const;	//gesture
		virtual void	notifyObservers(int x, int y, bool bCurrentlyPressed) const;					//touch
		
		void			registerTouchScreenEventProcessor(TouchScreenEventProcessor* pTouchScreenEventProcessor, bool bWithPriority = false);
		void			unregisterTouchScreenEventProcessor(TouchScreenEventProcessor* pTouchScreenEventProcessor);		
	
	public:
		void			registerObserver(TouchScreenObserver* pObserver);
		void			unregisterObserver(TouchScreenObserver* pObserver);
	
		virtual void	control();	//might be used my TouchScreenEventProcessor's time-based 'own processing'	
	
		TouchScreenSubject();
		virtual ~TouchScreenSubject();
};

}	//namespace end

#endif