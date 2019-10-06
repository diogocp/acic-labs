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
const byte sequence[] = {RED, GREEN, BLUE, YELLOW, ALL_OFF};
const size_t sequence_len = sizeof(sequence);

// Delay (in milliseconds) between each state transition
const unsigned long INTERVAL = 1000;

// The current position in the sequence
byte sequence_pos = 0;

// When the button is pressed, this flag is set to true, and
// the sequence stops until the button is pressed again
bool stopped = false;

unsigned long start_time = 0;

int prevButtonState = LOW;
int buttonState;

void setup() {
    pinMode(BUTTON, INPUT_PULLUP);
    prevButtonState = digitalRead(BUTTON);

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
    byte led = sequence[sequence_pos];

    if (led >= 0) {
        digitalWrite(led, HIGH);
    }

    start_time = millis();
    while (stopped || (millis() - start_time <= INTERVAL)) {
        buttonState = digitalRead(BUTTON);
        if (prevButtonState == LOW && buttonState == HIGH) {
            // Toggle stopped state
            stopped = !stopped;
        }
        prevButtonState = buttonState;
    }

    if (led >= 0) {
        digitalWrite(led, LOW);
    }

    if (++sequence_pos >= sequence_len) {
        sequence_pos = 0;
    }
}
