#include "TouchScreenGuiHelper.h"

#include "TouchScreenGuiObserver.h"
#include "TouchScreenController.h"

#include "DebugMsg.h"

namespace MDO {

/**
 * Returns true if the provided position is in (or on) the GUI element boundaries.
 */
bool TouchScreenGuiHelper::isInGuiElement(int x, int y, const gui_element_t& sGuiElement) const {
	return (x >= sGuiElement.iLeftX) && (x <= sGuiElement.iRightX) && (y <= sGuiElement.iTopY) && (y >= sGuiElement.iLowerY);
}

//input: will retreive detected gestures, this is our input
/*virtual*/ void TouchScreenGuiHelper::gestureNotification(int iGestureId, int x, int y, bool bCurrentlyPressed) {
	//CST816_TOUCH_DEBUG_PRINTLN("TouchScreenGuiHelper::gestureNotification");
	
	if (m_pObserver == 0) {
		CST816_TOUCH_DEBUG_PRINTLN("TouchScreenGuiHelper - nothing to report to");
		return;	//nothing to report to
	}
	if (!((iGestureId == (int)TouchScreenController::gesture_t::GESTURE_DOUBLE_CLICK) ||
		(  iGestureId == (int)TouchScreenController::GESTURE_LONG_PRESS))) {
		return;	//we only care about the two above gestures.
	}

	auto itPage = m_mGuiElements.find(m_uiCurrentPageId);
	if (itPage == m_mGuiElements.end()) {
		CST816_TOUCH_DEBUG_PRINTLN(String("TouchScreenGuiHelper - no GUI elements defined for page") + m_uiCurrentPageId);
		return;	//no GUI elements defined for the current page
	}
	page_gui_elements_t& vCurrentGuiElements = itPage->second;
	for (auto it = vCurrentGuiElements.begin(); it != vCurrentGuiElements.end(); it++) {
		if (isInGuiElement(x, y, *it)) {
			TouchScreenGuiObserver::gui_touch_mode_t eTouchMode = TouchScreenGuiObserver::gui_touch_mode_t::DOUBLE_CLICK;
			if (iGestureId == (int)TouchScreenController::GESTURE_LONG_PRESS) {
				eTouchMode = TouchScreenGuiObserver::gui_touch_mode_t::LONG_PRESS;
			}
			
			m_pObserver->guiNotification(it->strGuiIdName, eTouchMode, m_uiCurrentPageId);
			return;	//we do not support overlay constructions, so we're done
		}
	}
}

//input: will retrieve detected touch events
/*virtual*/ void TouchScreenGuiHelper::touchNotification(int x, int y, bool bCurrentlyPressed) {

	if (m_pObserver == 0) {
		CST816_TOUCH_DEBUG_PRINTLN("TouchScreenGuiHelper - nothing to report to");
		return;
	}
	
	if (bCurrentlyPressed) {	//we only care about release events
		return;
	}
	
	auto itPage = m_mGuiElements.find(m_uiCurrentPageId);
	if (itPage == m_mGuiElements.end()) {
		CST816_TOUCH_DEBUG_PRINTLN(String("TouchScreenGuiHelper - no GUI elements defined for page ") + m_uiCurrentPageId);
		return;	//no GUI elements defined for the current page
	}
	
	page_gui_elements_t& vCurrentGuiElements = itPage->second;
	for (auto it = vCurrentGuiElements.begin(); it != vCurrentGuiElements.end(); it++) {
		if (isInGuiElement(x, y, *it)) {
			//CST816_TOUCH_DEBUG_PRINTLN(String("Notify ") + it->strGuiIdName + " on page " + m_uiCurrentPageId);
			m_pObserver->guiNotification(it->strGuiIdName, TouchScreenGuiObserver::gui_touch_mode_t::CLICK, m_uiCurrentPageId);
			return;	//we do not support overlay constructions, so we're done
		} else {
			//CST816_TOUCH_DEBUG_PRINTLN(String("TouchScreenGuiHelper - no defined GUI region (") + x + "," + y + ")");
		}
	}
}

/**
 * Add a definition for a GUI element on a 'page'.
 * Tries to be graceful in the coordinates, but will check if the area is not 'weirdly small' using uiSmallestGuiElement from *begin*
 * Assuming that the edges provided do not overlap with other elements
 */
bool TouchScreenGuiHelper::defineGuiElement(int iX1, int iX2, int iY1, int iY2, String strGuiIdName, unsigned int uiPageId /*= 0*/) {

	bool bOk =	(iX1 >= 0) && (iX2 >= 0) && 
				(iY1 >= 0) && (iY2 >= 0) &&
				(abs(iX1-iX2) > m_uiSmallestGuiElement)	&&
				(abs(iY1-iY2) > m_uiSmallestGuiElement)	&&
				(strGuiIdName.length() > 0);
			
	if (bOk) {
		//CST816_TOUCH_DEBUG_PRINTLN(String("TouchScreenGuiHelper - adding '") + strGuiIdName + "' for page " + uiPageId);
		gui_element_t sGuiElement;
		sGuiElement.iLeftX	= std::min(iX1, iX2);
		sGuiElement.iRightX	= std::max(iX1, iX2);
		sGuiElement.iLowerY	= std::min(iY1, iY2);
		sGuiElement.iTopY	= std::max(iY1, iY2);
		sGuiElement.strGuiIdName = strGuiIdName;		
		
		m_mGuiElements[uiPageId].push_back(sGuiElement);
	} else  {
		CST816_TOUCH_DEBUG_PRINTLN(String("TouchScreenGuiHelper::defineGuiElement - ignoring element '") + strGuiIdName + "' for page " + uiPageId);
	}

	return bOk;
}

void TouchScreenGuiHelper::setCurrentPage(unsigned int uiPageId) {
	m_uiCurrentPageId = uiPageId;
}

bool TouchScreenGuiHelper::begin(TouchScreenGuiObserver* pObserver, unsigned int uiSmallestGuiElement /*= 5*/) {
	m_pObserver = pObserver;
	m_uiSmallestGuiElement = uiSmallestGuiElement;
	return m_pObserver != 0;
}

TouchScreenGuiHelper::TouchScreenGuiHelper() {
	m_pObserver = 0;
	m_uiCurrentPageId = 0;
}

TouchScreenGuiHelper::~TouchScreenGuiHelper() {
}

}	//namespace end