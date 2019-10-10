/* Applications and Computation for the Internet of Things 2019-2020
 * 2nd Lab work: Sensing the Physical World
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */

/* When DEBUG is defined, the values of the sensors are printed
 * to the serial port in the following order:
 *     temperature  rotation  light
 * Use the Arduino IDE Serial Plotter to visualize these values.
 */
#define DEBUG
#ifdef DEBUG
    #define print(x) Serial.print(x);
    #define println(x) Serial.println(x);
#else
    #define print(x)
    #define println(x)
#endif


const byte TEMPERATURE_SENSOR = A0;
const byte LIGHT_SENSOR       = A1;
const byte POTENTIOMETER      = A3;

const byte GREEN_LED  = 2;
const byte RED_LED    = 3;
const byte YELLOW_LED = 4;

const word MIN_BLINK_RATE = 200;
const word MAX_BLINK_RATE = 2000;

const int TEMPERATURE_THRESHOLD = 200;


void setup() {
    // Initialize the outputs
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);

#ifdef DEBUG
    Serial.begin(9600);
#endif
}

void loop() {
    handle_temperature();
    print(" ");
    handle_rotation();
    print(" ");
    handle_light();

    println();
}

void handle_temperature() {
    // Read the temperature sensor value
    int value = analogRead(TEMPERATURE_SENSOR);
    print(value);

    // Convert the sensor value to a temperature (in degrees Celsius)
    float t = (((value / 1024.0) * 5.0) - 0.5) * 100.0;

    if (t > TEMPERATURE_THRESHOLD) {
        digitalWrite(YELLOW_LED, HIGH);
    } else {
        digitalWrite(YELLOW_LED, LOW);
    }
}

void handle_rotation() {
    // Current LED state
    static byte led_state = LOW;
    // Last time (in milliseconds) that the LED state was toggled
    static unsigned long toggle_time = 0;

    // Read the rotation value
    int value = analogRead(POTENTIOMETER);
    print(value);

    // Convert the rotation value to a blinking rate
    word blink_rate = map(value, 0, 1023, MIN_BLINK_RATE, MAX_BLINK_RATE);

    // Check if enough time has passed to toggle the LED state
    unsigned long current_time = millis();
    if (current_time > toggle_time + blink_rate) {
        // More time has passed than the blink rate; toggle LED state
        led_state = (led_state == LOW) ? HIGH : LOW;
        digitalWrite(GREEN_LED, led_state);

        // Store the time that we toggled the state of the LED
        toggle_time = current_time;
    }
}

void handle_light() {
    // Read the light sensor value
    int value = analogRead(LIGHT_SENSOR);
    print(value);

    // Map the light sensor value to a brightness level
    byte brightness = map(value, 0, 1023, 0, 255);

    analogWrite(RED_LED, brightness);
}
