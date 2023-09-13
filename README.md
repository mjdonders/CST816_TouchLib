# CST816 touchscreen Arduino Library 
This is a library for the touch part of the *LILYGO T-Display ESP32-S3* product.
I'm not in any way linked to LILYGO, but I liked the physical product. Unfurtunately, the Arduino library support for the touch part could / should be improved. 
Specifically the lack of gesture support (which the hardware does support): This library fixes that.

## Note that this library just handles the touch part, not the display itself

All the defaults used are for the LILYGO T-Display ESP32-S3, so that should work in one go.
I didn't test other CST816-based products, but I guess that these should work as well.

There are two main ways of interfacing:
 * The easy way without any callbacks or so: please see example_no_callback
 * My preferred way using a callback mechanism based on an interface class: please see example_gesture_only

I managed to get a sleep working (alowing the esp32 to wake up from deep sleep after), please see example_gesture_only for that.

Note that all was created without official documentation. The documents I could find do not provide any info on registers, exact timing, etc.
If someone could point me to that, this library can most likely be improved (a lot)..
