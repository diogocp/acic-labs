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


// Identification of the digital outputs (actuators)
const byte GREEN_LED  = 2;
const byte RED_LED    = 3;
const byte YELLOW_LED = 4;

const byte I2C_ADDRESS = 8;

// Interval between LED health checks, in milliseconds
const word HEALTH_CHECK_INTERVAL = 1000;
const byte RED_HEALTH_INPUT   = 12;


int blink_period = 0;
byte brightness = 0;
byte temp_above_threshold = 0;
bool red_led_healthy = true;


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
    pinMode(RED_HEALTH_INPUT, INPUT);

    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(data_received);
    Wire.onRequest(request_received);
}

void loop() {
    rotation_actuator(blink_period);   // 2-5 us
    temperature_actuator(temp_above_threshold); // 5 us

    // Run the health checker immediately before setting the
    // brightness, so that the effect of the health checker
    // (which temporarily sets the brightness to full) is not noticed.
    static unsigned long last_health_check = 0;
    if (millis() - last_health_check >= HEALTH_CHECK_INTERVAL) {
        red_led_healthy = health_checker(RED_LED, RED_HEALTH_INPUT);
    }
    brightness_actuator(brightness);   // 8 us

#ifdef DEBUG
    Serial.print(blink_period);
    Serial.print(" ");
    Serial.print(brightness);
    Serial.print(" ");
    Serial.print(temp_above_threshold);
    Serial.print(" ");
    Serial.println(red_led_healthy);
#endif
}


void data_received(int num_bytes) {
    // Format: 1 bit temperature | 15 bits blink period | 8 bits brightness
    if (num_bytes == 3 || Wire.available() >= 3) {
        byte first_byte = Wire.read();
        blink_period = Wire.read();
        brightness = Wire.read();

        temp_above_threshold = first_byte >> 7;
        blink_period |= ((first_byte & 0x7f) << 8);
    }
}

void request_received() {
    Wire.write(byte(red_led_healthy));
}

bool health_checker(int output, int input) {
    digitalWrite(output, HIGH);
    if (digitalRead(input)) {
        return true;
    }
    else {
        return false;
    }
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

void brightness_actuator(int brightness) {
    // Use PWM to control the LED brightness
    analogWrite(RED_LED, brightness);
}

void temperature_actuator(byte temperature) {
    if (temperature == 0) {
        digitalWrite(YELLOW_LED, LOW);
    }
    else {
        digitalWrite(YELLOW_LED, HIGH);
    }
}
