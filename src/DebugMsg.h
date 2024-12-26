#ifndef _MDO_DebugMsg_H
#define _MDO_DebugMsg_H


namespace MDO {

//if you like, enable the following line for debug serial messages
//I tried this from the INO file, but could not get that to work..
#define CST816_TOUCH_LIB_DEBUG

#ifdef CST816_TOUCH_LIB_DEBUG
#define CST816_TOUCH_DEBUG_PRINT(str) Serial.print(str)
#else
#define CST816_TOUCH_DEBUG_PRINT(str)
#endif

#ifdef CST816_TOUCH_LIB_DEBUG
#define CST816_TOUCH_DEBUG_PRINTLN(str) Serial.println(str)
#else 
#define CST816_TOUCH_DEBUG_PRINTLN(str)
#endif

}	//namespace end

#endif