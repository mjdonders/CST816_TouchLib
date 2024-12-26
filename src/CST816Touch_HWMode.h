#ifndef _MDO_CST816Touch_HWMode_H
#define _MDO_CST816Touch_HWMode_H

#include "CST816Touch.h"

namespace MDO {

/**
 * Hardware based touch controller implementation.
 * Simple and quick, however limited from a configuration point of view and quite dependant on the capabilities of the hardware.
 */ 
class CST816Touch_HWMode: public CST816Touch {
	
	private:
	
	private:
	
	public:
		bool			setLongPressTime(uint8_t uiLongPressTime);

		virtual void	control();	//please call in loop()
		bool			begin(TwoWire& w, TouchScreenObserver* pTouchScreenObserver, int PIN_INTERRUPT = 16, int PIN_RESET = 21, uint8_t CTS816S_I2C_ADDRESS = 0x15);
		bool			begin(TwoWire& w, 											 int PIN_INTERRUPT = 16, int PIN_RESET = 21, uint8_t CTS816S_I2C_ADDRESS = 0x15);
	
		CST816Touch_HWMode();
		virtual ~CST816Touch_HWMode();
};

}	//namespace end

#endif