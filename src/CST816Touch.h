#ifndef _MDO_CST816Touch_H
#define _MDO_CST816Touch_H

#include <Arduino.h>
#include <Wire.h>

namespace MDO {
	
#define CST816_TOUCH_BUF_SIZE 9	//if increased will only provided 0xFF bytes	

class TouchSubscriberInterface;

/**
 * CST816 class, the main class in this library.
 * Tested *only* in combination with a LILYGO T-Display ESP32-S3
 * (default PIN numbers are from that as well)
 */ 
class CST816Touch {
	
	public:	//types
		
		enum gesture_t {					//when updating, don't forget the GestureIdToString method..
			GESTURE_NONE = 0,
			GESTURE_RIGHT,
			GESTURE_LEFT,
			GESTURE_DOWN,
			GESTURE_UP,
			GESTURE_TOUCH_BUTTON = 5,
			GESTURE_DOUBLE_CLICK,			//software determined, the hardware reported evenet seems absent
			//GESTURE_DOUBLE_CLICK = 0x0B,	//commented out, since it never happens on my LILYGO T-Display ESP32-S3's
			GESTURE_LONG_PRESS = 0x0C
		};
		
	private:
		TwoWire*					m_pI2C;		
		int							m_iPIN_RESET;
		int 						m_iPIN_INTERRUPT;
		uint8_t						m_uiCTS816S_I2C_ADDRESS;	
		uint8_t						m_ucAddress;
		uint8_t						m_ucBuffer[CST816_TOUCH_BUF_SIZE];
		uint8_t						m_ucPrevousBuffer[CST816_TOUCH_BUF_SIZE];
		TouchSubscriberInterface*	m_pTouchSubscriber;
		
		unsigned long				m_ulLastGesture;	//when   - GESTURE
		gesture_t					m_eLastGesture;		//what
		int							m_iLastGestureX;	//where
		int							m_iLastGestureY;	//where
		
		unsigned long				m_ulLastTouch;		//when   - this is the last release of a touch, used to check double click
		int							m_iLastTouchX;		//where
		int							m_iLastTouchY;		//where
		
		bool						m_bNotifyReleaseOnly;
		bool						m_bTouchConsumed;
		bool						m_bGestureConsumed;
	
	private:
		unsigned long	millisDiff(const unsigned long& ulStart, const unsigned long& ulEnd);
		unsigned long	millisDiff(const unsigned long& ulStart);
		bool			parseTouchEvent(int& x, int& y, bool& currentlyPressed);
		bool			writeRegister(uint8_t ucRegister, uint8_t ucByteToWrite);
		bool			readRegister(uint8_t ucRegister, uint8_t iRequestedSize);
		
		bool			read();
		bool			hadPhysicalEvent();	//touch or gesture.. Basically: anything to report?
		bool			handleGesture();
		bool			handleTouch();
	
	public:
		static String	gestureIdToString(int iGestureId);
		void			getLastTouchPosition(int& x, int& y);
		void			getLastGesture(gesture_t& gesture, int& x, int& y);
		bool			hadTouch() const;
		bool			hadGesture() const;
		
		bool			getFirmwareVersion(unsigned long& ulFW);
		void			setChipInDynamicMode();	//ensuring it listens to our commands..
		bool			sleep();
		void			printBuf(bool bWhenChangedOnly = false);	//just for debug purposes.
		void			deInit();
		bool			control();	//please call in loop()

		void			setNotificationsOnAllEvents();		//get notifications on touch events and release event
		void			setNotificationsOnReleaseOnly();	//get notifications on release event only - this is the default
		
		/**
		 * init. For each PIN which this class should not [use / change]: set to -1
		 * return false on issues
		 */
		bool			init(TwoWire& w, TouchSubscriberInterface* pTouchSubscriber = 0, int PIN_INTERRUPT = 16, int I2C_SDA = 18, int I2C_SCL = 17, uint8_t CTS816S_I2C_ADDRESS = 0x15, int PIN_RESET = 21);

	
		CST816Touch();
		CST816Touch(const CST816Touch& dummy);	//not implmented
		virtual ~CST816Touch();
};

}	//namespace end

#endif
