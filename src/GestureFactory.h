#ifndef _MDO_GestureFactory_H
#define _MDO_GestureFactory_H

#include "TouchScreenEventProcessor.h"

namespace MDO {

/**
 * For a chip which does provide touch events, but does not provide gesture support: provides a software implementation for the gestures.
 * Will block gestures if these do come from the chip, in order to not get stuff too much..
 * The screen width/height is needed to determine the relevant distances
 */ 
class GestureFactory: public TouchScreenEventProcessor {
	
	public:	//types
		enum tGestureFactoryState {
			NOT_INITIALIZED,
			INITIALIZED,
			TOUCHED,
			WAITING_FOR_RELEASE
		};
		
	
	private:
		int							m_iScreenWidth;				//config stuff
		int 						m_iScreenHeight;
		int							m_iLongPressTime_sec;
		int 						m_iHorSwipeMinDistance; 
		int 						m_iVertSwipeMinDistance;
		int 						m_iLongPressMaxDistance;	//end config stuff
		
		unsigned long				m_ulTouchTime;			
		enum tGestureFactoryState	m_eState;
		int							m_iStartX;
		int							m_iStartY;
		bool						m_bMovementSinceTouch;
		
		bool						m_bInvertGestureY;
		
	private:
		bool	checkInitialized() const;
		int		getDiff(int iPos1, int iPos2) const;
		void	invalidateTouchRegistration();
		bool	handleLongPress(int x, int y);
		bool	handleHorizontalGesture(int x, int y);
		bool	handleVerticalGesture(int x, int y);
	
	public:
		//will retreive detected gestures
		//bCurrentlyPressed indicates if the screen is currently being pressed, or not
		//x and y might be valid (/useful). Typically: for long press and double click
		//returns true when handled fully (read: processing stops here)
		virtual bool		gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed);
		
		//will retrieve detected touch events
		//bCurrentlyPressed indicates if the screen is currently being pressed, or not
		//returns true when handled fully (read: processing stops here)
		virtual bool		touchNotification(int x, int y, bool bCurrentlyPressed);
		virtual void		resetState();
		virtual void		control();	//please call often, needed for all timing based gestures
		
		bool				isInitialized() const;	
		void				setInvertVerticalGestures(bool bInvertGestureY = true);
		void				begin(	int iScreenWidth = 0, int iScreenHeight = 0, 
									int iLongPressTime_sec = 3,	int iLongPressMaxDistance = -1, 
									int iHorSwipeMinDistance = -1, int iVertSwipeMinDistance = -1); 
		
		GestureFactory(TouchScreenSubject* pParent);
		virtual ~GestureFactory();
};

}	//namespace end

#endif