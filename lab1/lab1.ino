/* Applications and Computation for the Internet of Things 2019-2020
 * 1st Lab work: Building an embedded system (V1.3, Sep 2019)
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */

const byte ALL_OFF = -1;
const byte RED     =  4;
const byte GREEN   =  5;
const byte BLUE    =  3;
const byte YELLOW  =  2;
const byte BUTTON  = 12;

// The sequence in which LEDs are lit
const byte SEQUENCE[] = {RED, GREEN, BLUE, YELLOW, ALL_OFF};
const size_t SEQUENCE_LEN = sizeof(SEQUENCE);

// Delay (in milliseconds) between each state transition
const unsigned long INTERVAL = 1000;

// How long we must see a new button state before accepting it
const unsigned long DEBOUNCE_INTERVAL = 50;


void setup() {
    pinMode(BUTTON, INPUT_PULLUP);

    // Initialize the outputs
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
    digitalWrite(YELLOW, LOW);

    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(BLUE, OUTPUT);
    pinMode(YELLOW, OUTPUT);
}

void loop() {
    // The current position in the sequence
    static size_t sequence_pos = 0;

    byte led = SEQUENCE[sequence_pos];

    if (led >= 0) {
        digitalWrite(led, HIGH);
    }

    wait_loop();

    if (led >= 0) {
        digitalWrite(led, LOW);
    }

    if (++sequence_pos >= SEQUENCE_LEN) {
        sequence_pos = 0;
    }
}

void wait_loop() {
    // Initialize debouncing variables. These are marked static
    // so that their values are preserved over LED state changes.
    static byte button_current = HIGH;
    static byte button_previous = HIGH;
    static byte button_debounced = HIGH;
    static unsigned long switch_time = 0;

    // When the button is pressed, this flag is set to true, and
    // the sequence stops until the button is pressed again
    bool stopped = false;

    unsigned long start_time = millis();

    while (stopped || (millis() - start_time <= INTERVAL)) {
        // Read the button state
        button_current = digitalRead(BUTTON);

        // Debounce
        if (button_current != button_previous) {
            switch_time = millis();
        }

        if ((millis() - switch_time) >= DEBOUNCE_INTERVAL) {
            if (button_debounced == LOW && button_current == HIGH) {
                // When the button is released (LOW -> HIGH),
                // we toggle the stopped state.
                stopped = !stopped;
            }

            // Store the new debounced button state
            button_debounced = button_current;
        }

        button_previous = button_current;
    }
}
