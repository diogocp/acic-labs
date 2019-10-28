/* Applications and Computation for the Internet of Things 2019-2020
 * 3rd Lab work: Network of Sensors and Actuators
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */

#include <Wire.h>


/* When DEBUG is defined, these values are printed to the serial port:
 *     blink_period  brightness  temp_above_threshold  red_led_healthy
 */
#define DEBUG


// Identification of the analog inputs (sensors)
const byte TEMPERATURE_SENSOR = A0;
const byte LIGHT_SENSOR       = A1;
const byte POTENTIOMETER      = A3;

// The range of the blinking period, depending on the potentiometer value
const word MIN_BLINK_PERIOD = 200;
const word MAX_BLINK_PERIOD = 2000;

// The temperature above which the yellow LED turns on
const float TEMPERATURE_THRESHOLD = 23.0;
const float TEMPERATURE_NOISE_THRESHOLD = 0.5;

const byte I2C_ADDRESS = 8;

// Interval between LED health checks, in milliseconds
const word HEALTH_CHECK_INTERVAL = 1000;


void setup() {
#ifdef DEBUG
    Serial.begin(9600);
#endif

    Wire.begin();
}

void loop() {
    // Each of the sensor reading functions takes ~112 us,
    // which is pretty much all spent in analogRead().
    int blink_period = rotation_sensor();
    byte brightness = brightness_sensor();
    float temperature = temperature_sensor();
    bool temp_above_threshold = temperature_filter(temperature);

    Wire.beginTransmission(I2C_ADDRESS);
    // Format: 1 bit temperature | 15 bits blink period | 8 bits brightness
    Wire.write((temp_above_threshold << 7) | highByte(blink_period));
    Wire.write(lowByte(blink_period));
    Wire.write(brightness);
    Wire.endTransmission();

    // Detect failure of the red LED (open-circuit)
    static byte red_led_healthy = 1;
    static unsigned long last_health_check = 0;
    if (millis() - last_health_check >= HEALTH_CHECK_INTERVAL) {
        Wire.requestFrom(I2C_ADDRESS, byte(1));
        red_led_healthy = Wire.read();
        last_health_check = millis();
    }

#ifdef DEBUG
    Serial.print(blink_period);
    Serial.print(" ");
    Serial.print(brightness);
    Serial.print(" ");
    Serial.print(temperature);
    Serial.print(" ");
    Serial.print(temp_above_threshold);
    Serial.print(" ");
    Serial.println(red_led_healthy);
#endif
}


int rotation_sensor() {
    // Read the rotation value
    int value = analogRead(POTENTIOMETER);

    // Convert the rotation value to a blinking rate
    int blink_period = map(value, 0, 1023, MIN_BLINK_PERIOD / 2, MAX_BLINK_PERIOD / 2);

    return blink_period;
}

byte brightness_sensor() {
    // Read the light sensor value
    int value = analogRead(LIGHT_SENSOR);

    // Map the light sensor value to a brightness level
    byte brightness = map(value, 0, 1023, 0, 255);

    return brightness;
}

float temperature_sensor() {
    // Read the temperature sensor value
    int value = analogRead(TEMPERATURE_SENSOR);

    // Convert the sensor value to a temperature (in degrees Celsius)
    //float temperature = (((value / 1024.0) * 5.0) - 0.5) * 100.0;

    // Simpler, equivalent formula with only one floating-point division
    float temperature = (value * 125L - 12800) / 256.0;

    return temperature;
}

bool temperature_filter(float temperature) {
    // This function implements a simple mechanism to filter sensor noise.

    static bool above_threshold = false;

    if (temperature > TEMPERATURE_THRESHOLD + TEMPERATURE_NOISE_THRESHOLD) {
        above_threshold = true;
    }
    else if (temperature < TEMPERATURE_THRESHOLD - TEMPERATURE_NOISE_THRESHOLD) {
        above_threshold = false;
    }

    return above_threshold;
}
