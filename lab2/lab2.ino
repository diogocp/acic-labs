/* Applications and Computation for the Internet of Things 2019-2020
 * 2nd Lab work: Sensing the Physical World
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */


/* When DEBUG is defined, these values are printed to the serial port:
 *     blink_period  brightness  temperature
 */
#define DEBUG


// Identification of the analog inputs (sensors)
const byte TEMPERATURE_SENSOR = A0;
const byte LIGHT_SENSOR       = A1;
const byte POTENTIOMETER      = A3;

// Identification of the digital outputs (actuators)
const byte GREEN_LED  = 2;
const byte RED_LED    = 3;
const byte YELLOW_LED = 4;

// The range of the blinking period, depending on the potentiometer value
const word MIN_BLINK_PERIOD = 200;
const word MAX_BLINK_PERIOD = 2000;

// The temperature above which the yellow LED turns on
const float TEMPERATURE_THRESHOLD = 25.0;
// How long a temperature must be continuously observed above
// or below the threshold for the LED to switch states
const word TEMPERATURE_DEBOUNCE_TIME = 20;


void setup() {
#ifdef DEBUG
    Serial.begin(9600);
#endif

    // Initialize the outputs
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
}

void loop() {
    // Each of the sensor reading functions takes ~112 us,
    // which is pretty much all spent in analogRead().
    int blink_period = rotation_sensor();
    byte brightness = brightness_sensor();
    float temperature = temperature_sensor();

    // The actuator functions take negligible time,
    // compared to the sensor functions.
    rotation_actuator(blink_period);   // 2-5 us
    brightness_actuator(brightness);   // 8 us
    temperature_actuator(temperature); // 7-9 us

#ifdef DEBUG
    Serial.print(blink_period);
    Serial.print(" ");
    Serial.print(brightness);
    Serial.print(" ");
    Serial.println(temperature);
#endif
}


int rotation_sensor() {
    // Read the rotation value
    int value = analogRead(POTENTIOMETER);

    // Convert the rotation value to a blinking rate
    int blink_period = map(value, 0, 1023, MIN_BLINK_PERIOD, MAX_BLINK_PERIOD);

    return blink_period;
}

void rotation_actuator(int blink_period) {
    // Current LED state
    static byte led_state = LOW;
    // Last time (in milliseconds) that the LED state was toggled
    static unsigned long toggle_time = 0;

    // Check if enough time has passed to toggle the LED state
    unsigned long current_time = millis();
    if (current_time > toggle_time + blink_period) {
        // More time has passed than the blink rate; toggle LED state
        led_state = (led_state == LOW) ? HIGH : LOW;
        digitalWrite(GREEN_LED, led_state);

        // Store the time that we toggled the state of the LED
        toggle_time = current_time;
    }
}


byte brightness_sensor() {
    // Read the light sensor value
    int value = analogRead(LIGHT_SENSOR);

    // Map the light sensor value to a brightness level
    byte brightness = map(value, 0, 1023, 0, 255);

    return brightness;
}

void brightness_actuator(int brightness) {
    // Use PWM to control the LED brightness
    analogWrite(RED_LED, brightness);
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

void temperature_actuator(float temperature) {
    /* This function implements a simple debouncing mechanism to
     * prevent the LED from flickering when the temperature is
     * close to the threshold. The LED will only switch states
     * when a temperature has been observed continuously above or
     * below the threshold for a certain period of time, determined
     * by the TEMPERATURE_DEBOUNCE_TIME constant.
     */
    static unsigned long threshold_time = 0;
    static int switching_to = LOW;

    unsigned long current_time = millis();

    if (temperature > TEMPERATURE_THRESHOLD) {
        if (switching_to == LOW) {
            threshold_time = current_time;
        }
        switching_to = HIGH;
    } else {
        if (switching_to == HIGH) {
            threshold_time = current_time;
        }
        switching_to = LOW;
    }

    if (current_time - threshold_time > TEMPERATURE_DEBOUNCE_TIME) {
        digitalWrite(YELLOW_LED, switching_to);
        threshold_time = current_time;
    }
}
