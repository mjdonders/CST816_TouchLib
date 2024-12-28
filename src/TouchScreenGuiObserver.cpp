#include "TouchScreenGuiObserver.h"


namespace MDO {

/*static*/ String TouchScreenGuiObserver::toString(const gui_touch_mode_t& eTouchMode) {
	switch (eTouchMode) {
		case CLICK:			return "click";
		case DOUBLE_CLICK:	return "double click";
		case LONG_PRESS:	return "long press";
	}
	return "";
}

TouchScreenGuiObserver::TouchScreenGuiObserver() {
}

TouchScreenGuiObserver::~TouchScreenGuiObserver() {
}

}	//namespace end