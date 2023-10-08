# CST816 touchscreen Arduino Library 
This is a library for the touch part of the *LILYGO T-Display ESP32-S3* product.
I'm not in any way linked to LILYGO, but I liked the physical product. Unfortunately, the Arduino library support for the touch part could / should be improved. 
Specifically the lack of gesture support (which the hardware does support): This library fixes that.

Gesture types supported: RIGHT, LEFT, DOWN, UP, TOUCH_BUTTON, DOUBLE_CLICK, LONG_PRESS.

## Note that this library just handles the touch part, not the display itself

All the defaults used are for the LILYGO T-Display ESP32-S3, so that should work in one go.
I didn't test other CST816-based products, but I guess that these should work as well.

There are two main ways of interfacing:
 * The easy way without any callbacks or so: please see example_no_callback
 * My preferred way using a callback mechanism based on an interface class: please see example_interrupt

An ESP32 deep sleep is also possible with a touch-based wakeup, please see example_gesture_only for that.
For the CST816 sleep to work, it needs to be on when the command is issue, the above example shows that as well.
Note: when using this example, ensure the screen is touched just before the a new program is flashed, to ensure that the ESP32-S3 is awake. 

This library was created without official documentation. The documents I could find do not provide any info on registers, exact timing, etc.
If someone could point me to that, this library can most likely be improved..

## Usage
Typically, start with the method *begin*
Standard options are:
 - setOperatingModeFast, this is the quickest option with most options and supporting all gestures. The double click gesture however **will** give two touch events as well. This is the current default mode.
 - setOperatingModeHardwareBased, in which case all gestures are executed in hardware. For both long press and double click, this is the slowest option

And in *loop*, call *control* regularly: see the examples.

Note that by default the chip goes in a standby/sleep mode (which it leaves on touch).
In such a standby/sleep save mode, it will **not respond to a command**.
When the program only interacts with the CST816 chip during initialization, or based on a touch: this scenario will happen.
If it does, the chip can be reset using *resetChip*, that will however ensure that the chip loses state (it's settings). Set bRestoreState to true when this needs to be restored.
It's an option to use *setAutoSleep* to false, making sure this standby/sleep mode won't be entered, but that seems like a waste of power to me.

## Options
- *getDeviceType*, check the type of CST816 chip
- *getFirmwareVersion*, check the firmware version
- *sleep*, might be useful when *setAutoSleep* has been set to false
- *resetChip*, used to ensure that the chip in auto-sleep-mode is ready to receive commands (don't forget bRestoreState in this case)
-
- *setNotificationsOnAllEvents* and *setNotificationsOnReleaseOnly* can be used to either get all events (touch and release) or not
- *setNotifyOnMovement* can be used to get movement gesture updates, the interval of which can be set using *setMovementInterval*
