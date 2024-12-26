#ifndef _MDO_CST816Touch_H
#define _MDO_CST816Touch_H

#include <Arduino.h>
#include <Wire.h>

#include "TouchScreenController.h"

namespace MDO {

#define CST816_TOUCH_BUF_SIZE 9	//if increased will only provided 0xFF bytes	

class TouchScreenObserver;

/**
 * CST816 class, the main 'worker' class in this library.
 * Tested *only* in combination with a LILYGO T-Display ESP32-S3 and the LILYGO T-Display S3 AMOLED
 * (default PIN numbers are from the first)
 *
 * I found the following in my boards:
 * T-Display S3 AMOLED:	device: CST816T, firmware: 3
 * T-Display S3:		device: CST816T, firmware: 2
 */ 
class CST816Touch: public TouchScreenController {
	
	public:	//types
		
		enum touch_opering_mode_t {
			TOUCH_MODE_DEFAULT,			//the mode right after a reset, not actually used
			TOUCH_MODE_FAST,			//quickest mode, double click and long press are software based, double click triggers 2 touch events AND a gesture event
			TOUCH_MODE_HARDWARE			//all fully executed in hardware, long press takes a lot longer, double click however only gives a single gesture event
		};
		
	private:
		TwoWire*					m_pI2C;		
		int							m_iPIN_RESET;
		int 						m_iPIN_INTERRUPT;
		uint8_t						m_uiCTS816S_I2C_ADDRESS;	
		uint8_t						m_ucAddress;	//I2C address
		uint8_t						m_ucBuffer[CST816_TOUCH_BUF_SIZE];
		uint8_t						m_ucPreviousBuffer[CST816_TOUCH_BUF_SIZE];
		
		touch_opering_mode_t		m_eOperatingMode;
		bool						m_bNotifyReleaseOnly;
		bool						m_bNotifyMotion;
		unsigned long				m_ulMovementInterval;	//the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
		
		unsigned long				m_ulLastGestureTime;	//when   - GESTURE

		unsigned long				m_ulLastTouchTime;		//when   - this is the last release of a touch, used to check double click

		int							m_iTouchButtonX;
		int							m_iTouchButtonY;

		bool						m_bSwapXY;
	
	private:
		bool			parseTouchControllerEvent(int& x, int& y, bool& currentlyPressed);
		bool			writeRegister(uint8_t ucRegister, uint8_t ucByteToWrite);
		bool			readRegister(uint8_t ucRegister, uint8_t iRequestedSize);
		
		bool			read();
		bool			hadPhysicalEvent();	//touch or gesture.. Basically: anything to report?
		void			handleGesture(TouchScreenController::gesture_t eGesture, int x, int y, bool currentlyPressed);
		void			handleGesture();
		void			handleTouch();
		bool			setOperatingMode(touch_opering_mode_t eOperingMode, bool bNotifyMotion, bool bWakeChipFirst);
		
	protected:
		bool			printChipOperatingMode();
		bool			setOperatingModeFast();				//set quickest operating mode, double click and long press are software based, double click triggers 2 touch events AND a gesture event - this is the *default*
		bool			setOperatingModeHardwareBased();	//set full hardware based operating mode, long press takes a lot longer, double click however only gives a single gesture event 

		void			setMovementInterval(unsigned long ulMovementInterval);	//sets the minimal time (in msec) which should have passed before a new gesture-movement is reported. defaults to 50
		bool			setNotifyOnMovement(bool bMovementNotificationsRequired = true);	//for events during movement. only for Fast mode
		bool			setNotificationsOnAllEvents();		//get notifications on touch events and release event
		void			setNotificationsOnReleaseOnly();	//get notifications on release event only - this is the *default*		
	
	public:
		static uint8_t	getDefaultI2Caddress();
		static String	operatingModeToString(touch_opering_mode_t eOperatingMode);

		bool			setAutoSleep(bool bEnable);
		bool			getDeviceType(TouchScreenController::device_type_t& eDeviceType);
		bool			getFirmwareVersion(unsigned char& ucFW);
		bool			sleep();
		bool			resetChip(bool bRestoreState);	//ensuring it listens to our commands.. Does nothing when PIN_RESET is not provided.
		
		void			printBuf(bool bWhenChangedOnly = false);	//just for debug purposes.
		void			stop();
		virtual void	control();	//please call in loop()
		
		void			setTouchButtonCoordinates(int iTouchButtonX, int iTouchButtonY);
		void			setSwapXY(bool bSwapXY = true);		//default is T-Display which needs swapping the XY coordinates

	protected:
		bool			setHardwareLongPressTime(uint8_t uiLongPressTime);	//set long press time in hardware
	
		/**
		 * begin / initialize this class/library
		 * return false on issues
		 */
		bool			begin(TwoWire& w, TouchScreenObserver* pTouchScreenObserver, int PIN_INTERRUPT = 16, int PIN_RESET = 21, uint8_t CTS816S_I2C_ADDRESS = 0x15);

		CST816Touch();	//protected constructor, to highlight: don't use this class directly
	private:
		CST816Touch(const CST816Touch& dummy);	//not implemented, keep this private to get a nice compiler error when still used
	public:
		virtual ~CST816Touch();
};

}	//namespace end

#endif
