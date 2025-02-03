#ifndef _MDO_TouchScreenGuiHelper_H
#define _MDO_TouchScreenGuiHelper_H

#include <Arduino.h>
#include <vector>
#include <map>

#include "TouchScreenObserver.h"

namespace MDO {

class TouchScreenGuiObserver;

/**
 * A helper for touch screen GUI handling.
 * The idea is to define GUI elements here and let this class parse touch screen events to see with which GUI elements there was interaction.
 */ 
class TouchScreenGuiHelper: public TouchScreenObserver {
	
	public:	//types		
		struct gui_element_t {
			int iLeftX;
			int iRightX;
			int iLowerY;
			int iTopY; 
			String strGuiIdName;
		};
		
	private:	//types
		typedef std::vector<gui_element_t>					page_gui_elements_t;	//all elements in a page
		typedef std::map<unsigned int, page_gui_elements_t>	gui_elements_t;			//all elements grouped per page
	
	private:
		TouchScreenGuiObserver*	m_pObserver;
		gui_elements_t			m_mGuiElements;
		unsigned int 			m_uiCurrentPageId;
		unsigned int 			m_uiSmallestGuiElement;
		
	private:
		bool										isInGuiElement(int x, int y, const gui_element_t& sGuiElement) const;
		int /*TouchScreenGuiObserver::gui_touch_mode_t*/	toTouchMode(TouchScreenController::gesture_t eGesture) const;
	
	public:
		//input: will retreive detected gestures, this is our input
		virtual void	gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed);	
		
		//input: will retrieve detected touch events
		virtual void	touchNotification(int x, int y, bool bCurrentlyPressed);

		//add a definition for a GUI element on a 'page'
		bool			defineGuiElement(int iX1, int iX2, int iY1, int iY2, String strGuiIdName, unsigned int uiPageId = 0);
		void			setCurrentPage(unsigned int uiPageId);
	
		bool			begin(TouchScreenGuiObserver* pObserver, unsigned int uiSmallestGuiElement = 5);
	
		TouchScreenGuiHelper();
		virtual ~TouchScreenGuiHelper();
};

}	//namespace end

#endif