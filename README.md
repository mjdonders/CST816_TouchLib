# CST816 touchscreen Arduino Library 
This is a library for the touch part of the *LILYGO T-Display ESP32-S3* and *LILYGO T-Display S3 AMOLED* products.
I'm not in any way linked to LILYGO, but I liked the physical products. Unfortunately, the Arduino library support for the touch part could / should be improved. 
Specifically the lack of gesture support: This library fixes that.

Gesture types supported: RIGHT, LEFT, DOWN, UP, TOUCH_BUTTON, DOUBLE_CLICK, LONG_PRESS.

**Note that this library just handles the touch part, not the display itself**

All the defaults used are for the LILYGO T-Display ESP32-S3, so that should work in one go.
The examples also show how to use the T-Display S3 AMOLED: there are some hardware differences like pins and resolution.
I didn't test other CST816-based products, but I guess that these should work as well.

There are two main ways of interfacing:
 - The easy way without any callbacks or so: please see example_no_callback
 - My preferred way using a callback mechanism based on an interface class: please see example_gesture_only

An ESP32 deep sleep is also possible with a touch-based wakeup, please see example_gesture_only for that.
For the CST816 sleep to work, it needs to be on when the command is issued (see 'Sleep behaviour' below), the above example shows that as well.
Note: when using this example, ensure the screen is touched just before the a new program is flashed, to ensure that the ESP32-S3 is awake. 

## Usage
There are two main classes: 
 - CST816Touch_HWMode for the hardware based operation and
 - CST816Touch_SWMode for the (mainly) software based operation.

Typically, start with initialize Wire and a call to  *begin*.
The CST816Touch_SWMode allows to use the following TouchScreenEventProcessor's (all optional):
 - DoubleClickFactory, this is a software based double click implementation. Since this does not cache/buffer anything, it's quick (from a response point of view). That however does mean that only after the second click a double click gesture can be generated. Meaning that a double click even always receives a touch event first.
 - ElegantDoubleClickFactory, this alternative software based double click implementation is more elegant. By caching events, it can ensure that a double click will not also generate a touch event first. For this to work, touch events are cached and therefore delayed by the maximum allowed double click time.
 - GestureFactory, this is a software based approach to gesture generation from touch events. It supports left, right, up, down, long press. Should not be used when the hardware already supports these gestures.

Relevant events are always relayed to a TouchScreenObserver.
You can implement and provide your own (just derive from this class), or not.
When no TouchScreenObserver is provided in *begin*, a TouchScreenEventCache is used. Since that is a Singleton, you can check everywhere in your code if something happended with the touch screen.
See *TouchScreenEventCache::getInstance*.

Please don't forget to call *control* regularly (in *loop*). See the examples.

## Sleep behaviour
Note that by default the chip goes in a standby/sleep mode (which it leaves on physical touch events).
In such a standby/sleep save mode, it will **not respond to a command**.
When the program only interacts with the CST816 chip during initialization, or based on a touch: this scenario will happen.
If it does, the chip can be reset using *resetChip*, that will however ensure that the chip loses state (it's settings). Set bRestoreState to true when this needs to be restored.
It's an option to use *setAutoSleep* to false, making sure this standby/sleep mode won't be entered.

The above *resetChip* will only work if a reset pin is assigned (which is not the case for the T-Display S3 AMOLED).
Right after the call to *begin*, *setAutoSleep* to false can be used. However since that's not always needed, I didn't implement this as default.

## Usage - GUI handling
There are two classes available to aid 'GUI handling'. Please see:
- *TouchScreenGuiHelper*, which can be used to define regions of interest per 'page' (for multi-page user interfaces). The provided name will be reported when touched later. It will also report click, double click (when available based on the setup) and long press (also: when available based on the setup).
- *TouchScreenGuiObserver*, this is the receiver of the above GUI events.
Please see example_gui_helper.

## Other options
- *getDeviceType*, check the type of CST816 chip
- *getFirmwareVersion*, check the firmware version
- *sleep*, might be useful when *setAutoSleep* has been set to false
- *resetChip*, used to ensure that the chip in auto-sleep-mode is ready to receive commands (don't forget bRestoreState in this case). Only does something when the reset pin is provided in *begin*.



Compared to the previous version of the library, the following have been removed (externally):
- *setNotificationsOnAllEvents* and *setNotificationsOnReleaseOnly* can be used to either get all events (touch and release) or not
- *setNotifyOnMovement* can be used to get movement gesture updates, the interval of which can be set using *setMovementInterval*
The reason is that I don't see the need for these small touch screens.
When this is required, please let me know since I do see possibilities for this. For now, I just want to release the current status.