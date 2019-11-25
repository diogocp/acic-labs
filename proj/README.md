Project: Intelligent control of traffic flow
============================================

a. Design of the circuits of the controller, traffic lights, and sensors
------------------------------------------------------------------------


b. Overall architecture of the software in the controller
---------------------------------------------------------

The overall architecture is round-robin with interrupts. The push-button
switches (representing induction loops) are handled in interrupts, which
merely increase a counter. The reception of I2C messages is also handled
in an interrupt service routine.

c. Safety and fault-tolerance measures adopted
----------------------------------------------

The system periodically (multiple times a second) checks for failures of
the red lights. If a failure is detected, it immediately switches to a
fail-safe mode, with the yellow lights blinking.

d. Intelligent functions implemented
------------------------------------

We implemented the logic for mode 2, which creates a green wave letting
cars flow with less disruption. This works by slightly changing the
period to gradually bring the traffic lights in neighboring
intersections out of phase just enough so that when cars reach the next
intersection, the light will turn green.
