#ifndef __CST816_TOUCH_LIB_
#define __CST816_TOUCH_LIB_

#include "TouchScreenController.h"		//abstract base class for 'all' supported touch screen controller chips
#include "CST816Touch.h"				//main touch controller chip interface, cannot be used directly
#include "CST816Touch_HWMode.h"			//main touch controller chip interface, hardware mode
#include "CST816Touch_SWMode.h"			//main touch controller chip interface, software mode

#include "TouchScreenSubject.h"			//interface class meant as 'Subject'
#include "TouchScreenObserver.h"		//interface class meant as interface for the outside world (receiving touch & gesture events)
#include "TouchScreenEventCache.h"		//used when not providing an Observer. Can be used to ask in a new touch/gesture occured

#include "TouchScreenEventProcessor.h"	//abstract class used to pre-process touch screen events
#include "DoubleClickFactory.h"			//a TouchScreenEventProcessor capable of generating double click events. will slow down normal touch events
#include "GestureFactory.h"				//a TouchScreenEventProcessor capable of generating the other supported gestures. no slow down involveds

#include "TouchScreenGuiObserver.h"		//Observer for the TouchScreenGuiHelper
#include "TouchScreenGuiHelper.h"		//an Observer which can be used to help answer which part of the screen is touched (and how)

//Please see DebugMsg.h in case you want to enable / disable reported debug messages from this lib

#endif
