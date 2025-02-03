#include "TouchScreenGuiObserver.h"


namespace MDO {

/*static*/ String TouchScreenGuiObserver::toString(const gui_touch_mode_t& eTouchMode) {
	switch (eTouchMode) {
		case CLICK:			return "click";
		case DOUBLE_CLICK:	return "double click";
		case LONG_PRESS:	return "long press";
		case GESTURE_LEFT:	return "gesture left";
		case GESTURE_RIGHT:	return "gesture right";
		case GESTURE_UP:	return "gesture up";
		case GESTURE_DOWN:	return "gesture down";
	}
	return "";
}

TouchScreenGuiObserver::TouchScreenGuiObserver() {
}

TouchScreenGuiObserver::~TouchScreenGuiObserver() {
}

}	//namespace end