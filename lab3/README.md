3rd Lab work: Network of Sensors and Actuators
==============================================


1.a. Changes in the design pattern
----------------------------------

In the master, the design pattern is relatively unchanged. Essentialy,
the calls to the actuator functions were replaced by calls to
`Wire.write()`. That is, instead of controlling the actuators directly,
the master sends a message to the slave.

In the slave there are some more noticeable differences. The main loop
still consists of calls to the actuator functions. However, the pattern
is now more similar to round-robin with interrupts, since when a message
is received from the master, an interrupt service routine is called.
This ISR simply receives the data and stores it in global variables.


1.b. Other implementation issues
--------------------------------

We made several changes to the implementations of the tasks, in order to
align them with the requirements of this assignment.

In particular, we moved some logic that was previously in the actuator
routines to the sensor reading routines. This was done for two main
reasons.

The first reason is that all the logic is concentrated in the master,
thus if some change to the system is desired, only the master needs to
be reflashed. This later proved to be a good decision, when we found
ourselves iterating on the development of the light sensor – LED system,
it was very convenient to have all the logic in one place and not have
to reflash the slave at all.

The second reason is that it allowed for some efficiency gains in the
communication protocol. For example, if we wanted to transmit the raw
value of the temperature sensor, for it to be processed by the slave,
we would need to transmit two bytes. However, the slave only needs to
know if the corresponding LED should be on or off. We can transmit this
information using a single bit. That is exactly what we did, packing all
the information required from all three sensors in only three bytes.


4. Evaluation of the I2C bus performance
----------------------------------------

To estimate the data rate, we measured the time that it took to send a
3-byte message (i.e. the time between the calls to
`Wire.beginTransmission()` and `Wire.endTransmission()`) in a loop.

We found that the whole transmission took 420 μs, for a data rate of
7143 bytes per second or 2381 messages per second. We consider this to
be more than enough for our purposes in this project.

In terms of latency, we measured it by calling `Wire.requestFrom()` and
waiting for the response. When we did this, we made sure that the slave
was not running any other code that could interfere with the
measurements, by clearing the slave's `loop()` and leaving only the
`Wire.onRequest()` interrupt service routine.

Doing this, we found a round-trip time of 220 μs, which again, is more
than enough for this application, since it is fast enough that a human
will not notice it.


5.c. Changes in the protocol required for the detection of a LED failure
------------------------------------------------------------------------

This change turned out to be very simple to implement. The slave checks
the LED for failure periodically, by setting it to full brightness and
reading the relevant digital input. This value is stored in a global
variable.

The master periodically sends a request to the slave, using the function
`Wire.requestFrom()`. This fires an ISR in the slave that replies with
the value of the global variable mentioned.


6. Changes when the light sensor is facing its corresponding LED
----------------------------------------------------------------

When we mounted the light sensor and its corresponding LED face-to-face,
we saw that the LED intensity oscillated a lot.

We can explain this by noting that when the LED is bright, the sensor
receives more light, which makes it want to turn the LED down, which
then means that the sensor will detect less light and want to turn the
LED back up. This feedback loop causes the instability that we observed.

We can stabilize the system by making smaller adjustments to the
intensity of the LED. We explored several methods for doing this,
including taking the average of the previous and current sensor values,
incrementing or decrementing by a fixed amount, and using an
exponentially weighted moving average. We settled on the last option,
since it provided a good balance between simplicity, flexibility,
stability and reaction speed.
