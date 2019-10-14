2nd Lab work: Sensing the Physical World
========================================


Temperature
-----------

The mapping process was based on the conversion function given:

    T = (((value / 1024.0) × 5.0) − 0.5) × 100

although we simplified it slightly to reduce the number of floating
point operations required (for better performance and precision):

    T = (value × 125 − 12800) / 256.0

In terms of calibration, as we did not have access to any precision
instruments, we simply observed that the values obtained were roughly
in line with ambient temperature, and increased when touching or
breathing over the sensor, as expected.

We implemented a simple "debouncing" mechanism to prevent the LED from
flickering when the temperature is close to the threshold. The LED will
only switch states when a temperature has been observed continuously
above or below the threshold for at least 20 milliseconds.

To demonstrate the functionality of the system, we use touch the sensor
with a fingertip or breathe over it. This causes a temperature rise,
which, after a few seconds, leads to the yellow LED turning on. After
removing the fingertip from the LED, the temperature starts falling,
which eventually leads to the LED turning back off.


Rotation
--------

The mapping process implemented was a straightforward linear conversion
from the sensor values (ranging from 0 to 1023) to the blinking period
(ranging from 200 to 2000 milliseconds).

For calibration, we verified that the lowest observed sensor value was 0
and the highest was 1023. We also checked that the progression in values
as the potentiometer was turned was roughly linear, although again, as
we did not have any precision instruments avaliable, no further
calibration was possible.

To control the actuator, we store the last time that the LED state
changed (from on to off or from off to on), and at each iteration of the
loop we check if enough time (i.e. the blinking period, given by the
value of the potentiometer) has passed. If so, the LED state is toggled.

To demonstrate the functionality of the system, we simply turn the
potentiometer up and down and observe as the green LED blinks faster and
slower.


Light
-----

The mapping process implemented was a straightforward linear conversion
from the sensor values (ranging from 0 to 1023) to the Arduino PWM
output parameter, which has a range from 0 to 255.

As we did not have any instruments to measure luminous intensity, we did
not do any calibration apart from verifying that the maximum and
minimum sensor values (0-1023) could be reached by covering the sensor
and shining a flashlight on it, respectively.

The actuator (yellow LED) is directly controlled by the analogWrite()
function, which uses pulse width modulation (PWM) to control the
brightness of the LED.

To demonstrate the functionality of the system, we use a marker pen cap
to cover the light sensor, which dims the LED, and shine a flashlight
on the sensor to make the LED go to full brightness.


What is the system software pattern of the application?
-------------------------------------------------------

The software pattern implemented is round-robin.

First, all sensors are read in turn. This takes the most time, but it is
unavoidable as there is no interrupt support for analog inputs on the
Arduino (and even if there was, it would likely not be very useful, as
the values of analog inputs tend to change almost constantly).

Then, each actuator is updated in turn, according to the updated values
of the sensors.


What are the timing constraints of the system?
----------------------------------------------

Each of the sensor reading functions takes ~112 μs, which is pretty much
all spent in analogRead().

The actuator functions take negligible time, compared to the sensor
functions:
  - temperature: 7-9 μs
  - rotation:    2-5 μs
  - light:       8 μs
