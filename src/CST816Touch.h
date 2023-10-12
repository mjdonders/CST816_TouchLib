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
		
		enum touch_t {
			TOUCH_DOWN      = 0,			//Touch press
			TOUCH_UP        = 1,			//Touch release
			TOUCH_CONTACT   = 2				//Touch contact
		};

		enum gesture_t {					//when updating, don't forget the GestureIdToString method..
			GESTURE_NONE = 0,
			GESTURE_RIGHT,
			GESTURE_LEFT,
			GESTURE_DOWN,
			GESTURE_UP,
			GESTURE_TOUCH_BUTTON = 5,		// or 'single click'
			GESTURE_DOUBLE_CLICK = 0x0B,	//depending on the operational mode, this might be software determined
			GESTURE_LONG_PRESS = 0x0C
		};
		
		enum device_type_t {
			DEVICE_UNKNOWN,
			DEVICE_CST716	= 0x20,
			DEVICE_CST816S	= 0xB4,
			DEVICE_CST816T	= 0xB5,
			DEVICE_CST816D	= 0xB6
		};
		
		enum touch_opering_mode_t {
			TOUCH_MODE_DEFAULT,			//the mode right after a reset, not actually used
			TOUCH_MODE_FAST,			//quickest mode, double click and long press are software based, double click triggers 2 touch events AND a gesture event
			TOUCH_MODE_HARDWARE			//all fully executed in hardware, long press takes a lot longer, double click however only gives a single gesture event
		};
		
	private:
		TwoWire*					m_pI2C;		
		bool						m_bWeInitializedI2C;
		int							m_iPIN_RESET;
		int 						m_iPIN_INTERRUPT;
		uint8_t						m_uiCTS816S_I2C_ADDRESS;	
		uint8_t						m_ucAddress;
		uint8_t						m_ucBuffer[CST816_TOUCH_BUF_SIZE];
		uint8_t						m_ucPreviousBuffer[CST816_TOUCH_BUF_SIZE];
		TouchSubscriberInterface*	m_pTouchSubscriber;
		
		touch_opering_mode_t		m_eOperatingMode;
		bool						m_bNotifyReleaseOnly;
		bool						m_bNotifyMotion;
		unsigned long				m_ulMovementInterval;	//the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
		
		unsigned long				m_ulLastGestureTime;	//when   - GESTURE
		gesture_t					m_eLastGesture;			//what
		int							m_iLastGestureX;		//where
		int							m_iLastGestureY;		//where
		
		unsigned long				m_ulLastTouchTime;		//when   - this is the last release of a touch, used to check double click
		int							m_iLastTouchX;			//where
		int							m_iLastTouchY;			//where
		
		bool						m_bTouchConsumed;
		bool						m_bGestureConsumed;
	
	private:
		unsigned long	millisDiff(const unsigned long& ulStart, const unsigned long& ulEnd);
		unsigned long	millisDiff(const unsigned long& ulStart);
		bool			parseTouchEvent(int& x, int& y, bool& currentlyPressed);
		bool			writeRegister(uint8_t ucRegister, uint8_t ucByteToWrite);
		bool			readRegister(uint8_t ucRegister, uint8_t iRequestedSize);

		bool			setOperatingMode(touch_opering_mode_t eOperingMode, bool bNotifyMotion, bool bWakeChipFirst);
		bool			printChipOperatingMode();
		
		bool			read();
		bool			hadPhysicalEvent();	//touch or gesture.. Basically: anything to report?
		bool			handleGesture(gesture_t eGesture, int x, int y, bool currentlyPressed);
		bool			handleGesture();
		bool			handleTouch();
	
	public:
		static String	deviceTypeToString(device_type_t eDeviceType);
		static String	gestureIdToString(int iGestureId);
		static String	operatingModeToString(touch_opering_mode_t eOperatingMode);
		void			getLastTouchPosition(int& x, int& y);
		void			getLastGesture(gesture_t& gesture, int& x, int& y);
		bool			hadTouch() const;
		bool			hadGesture() const;

		bool			setAutoSleep(bool bEnable);
		bool			getDeviceType(device_type_t& eDeviceType);
		bool			getFirmwareVersion(unsigned char& ucFW);
		bool			sleep();
		bool			resetChip(bool bRestoreState);	//ensuring it listens to our commands..
		
		void			printBuf(bool bWhenChangedOnly = false);	//just for debug purposes.
		void			deInit();
		void			stop();
		bool			control();	//please call in loop()

		void			setMovementInterval(unsigned long ulMovementInterval);	//sets the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
		bool			setNotifyOnMovement(bool bMovementNotificationsRequired = true);

		bool			setNotificationsOnAllEvents();		//get notifications on touch events and release event
		void			setNotificationsOnReleaseOnly();	//get notifications on release event only - this is the *default*
		
		bool			setOperatingModeFast();				//set quickest operating mode, double click and long press are software based, double click triggers 2 touch events AND a gesture event - this is the *default*
		bool			setOperatingModeHardwareBased();	//set full hardware based operating mode, long press takes a lot longer, double click however only gives a single gesture event 

		/**
		 * initialize. For each PIN which this class should not [use / change]: set to -1
		 * @deprecated, based on the Wire initialization, which does not belong in a library..
		 * WILL initialize Wire. Use 'begin' if this is not desired
		 * return false on issues
		 */
		bool			init(TwoWire& w, TouchSubscriberInterface* pTouchSubscriber = 0, int PIN_INTERRUPT = 16, int I2C_SDA = 18, int I2C_SCL = 17, uint8_t CTS816S_I2C_ADDRESS = 0x15, int PIN_RESET = 21);
		
		/**
		 * begin / initialize this class/library
		 * return false on issues
		 */
		bool			begin(TwoWire& w, TouchSubscriberInterface* pTouchSubscriber = 0, int PIN_INTERRUPT = 16, uint8_t CTS816S_I2C_ADDRESS = 0x15, int PIN_RESET = 21);

	
		CST816Touch();
		CST816Touch(const CST816Touch& dummy);	//not implmented
		virtual ~CST816Touch();
};

}	//namespace end

#endif
