/* Applications and Computation for the Internet of Things 2019-2020
 * Project: Intelligent control of traffic flow
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */

#include <Wire.h>


// Identification of the outputs (traffic lights)
const int TL_NS_R = 4;
const int TL_NS_Y = 5;
const int TL_NS_G = 6;
const int TL_EW_R = 7;
const int TL_EW_Y = 8;
const int TL_EW_G = 9;

// Identification of the inputs (loop detectors)
const int LD_NS = 2;
const int LD_EW = 3;

// Identification of the inputs (red led health check)
const int TL_NS_R_OK = 12;
const int TL_EW_R_OK = 13;

// All units are milliseconds
const unsigned long PERIOD = 20000;
const unsigned long YELLOW_DURATION = 1000;
const unsigned long MIN_DUTY_CYCLE = 5000;
const unsigned long MAX_DUTY_CYCLE = 15000;
// Time for a car to go from one intersection to the next
const unsigned long TRAVEL_TIME = 6000;

const byte I2C_ADDRESS = 8;

// Definition of the message format
enum Event {
    R2G_N = 0,
    R2G_S = 1,
    R2G_E = 2,
    R2G_W = 3
};
struct Message {
    byte dst_x : 4;
    byte dst_y : 4;
    byte src_x : 4;
    byte src_y : 4;
    byte event;
    byte cars_n;
    byte cars_s;
    byte cars_e;
    byte cars_w;
    unsigned long timestamp;
};
void print_message(Message* m);


byte mode = 1;

// Cars counted at the north/south/east/west access during the last period
volatile byte cars_n = 0;
volatile byte cars_s = 0;
volatile byte cars_e = 0;
volatile byte cars_w = 0;

volatile byte orientation_s = false;
volatile byte orientation_w = false;


void setup() {
    pinMode(LD_NS, INPUT_PULLUP);
    pinMode(LD_EW, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(LD_NS), cars_ns, FALLING);
    attachInterrupt(digitalPinToInterrupt(LD_EW), cars_ew, FALLING);

    pinMode(TL_NS_R_OK, INPUT);
    pinMode(TL_EW_R_OK, INPUT);

    pinMode(TL_NS_R, OUTPUT);
    pinMode(TL_NS_Y, OUTPUT);
    pinMode(TL_NS_G, OUTPUT);
    pinMode(TL_EW_R, OUTPUT);
    pinMode(TL_EW_Y, OUTPUT);
    pinMode(TL_EW_G, OUTPUT);

    // Initial state of the system (at an intersection)
    // All traffic lights blinking yellow with a 2 seconds
    // period (ON + OFF cycle time) during 6 seconds.
    for (int i = 0; i < 3; i++) {
        mode_standby();
    }

    Serial.begin(9600);
    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(message_received);
}

void mode01() {
    unsigned long duty_cycle = PERIOD / 2;

    if (mode > 0) {
        if (cars_n + cars_s + cars_e + cars_w > 0) {
            duty_cycle = PERIOD * (cars_n + cars_s) / (cars_n + cars_s + cars_e + cars_w);
            if (duty_cycle < MIN_DUTY_CYCLE) duty_cycle = MIN_DUTY_CYCLE;
            if (duty_cycle > MAX_DUTY_CYCLE) duty_cycle = MAX_DUTY_CYCLE;
        }
    }
    Serial.print("Duty cycle: "); Serial.println(duty_cycle);
    cars_n = cars_s = cars_e = cars_w = 0;

    switch_lights(R2G_N, duty_cycle);
    switch_lights(R2G_E, PERIOD - duty_cycle);
}

void switch_lights(Event e, int total_duration) {
    int tl_g;
    int tl_r;
    int tl_r_ok;

    if (e == R2G_N || e == R2G_S) {
        tl_g = TL_NS_G;
        tl_r = TL_EW_R;
        tl_r_ok = TL_EW_R_OK;
    } else if (e == R2G_E || R2G_W) {
        tl_g = TL_EW_G;
        tl_r = TL_NS_R;
        tl_r_ok = TL_NS_R_OK;
    } else {
        // Unsupported event; go to standby mode until reset
        while (true) mode_standby();
    }

    digitalWrite(tl_g, HIGH);
    digitalWrite(tl_r, HIGH);
    for (int i = 0; i < 100; i++) {
        // LED health check
        while (digitalRead(tl_r_ok) == LOW)
        {
            mode_standby();
            digitalWrite(tl_g, HIGH);
            digitalWrite(tl_r, HIGH);
        }
        delay((total_duration - YELLOW_DURATION) / 100);
    }
    digitalWrite(tl_r, LOW);
    digitalWrite(tl_g, LOW);

    // Yellow for 1 second
    digitalWrite(TL_NS_Y, HIGH);
    digitalWrite(TL_EW_Y, HIGH);
    delay(YELLOW_DURATION);
    digitalWrite(TL_NS_Y, LOW);
    digitalWrite(TL_EW_Y, LOW);
}

void loop() {
    // TODO: this is just a demo of sending/receiving messages
    static unsigned long last_message_sent = millis();

    Message m = {
        .dst_x = 1,
        .dst_y = 3,
        .src_x = 2,
        .src_y = 2,
        .event = R2G_S,
        .cars_n = cars_n,
        .cars_s = cars_s,
        .cars_e = cars_e,
        .cars_w = cars_w,
        .timestamp = millis()/100
    };

    if (millis() > last_message_sent + PERIOD) {
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write((byte*)&m, sizeof(m));
        Wire.endTransmission();
        last_message_sent = millis();
    }

    switch (mode) {
        case 0:
        case 1:
            mode01();
            break;
        case 255:
            mode_standby();
            break;
    }
}

void message_received(int num_bytes) {
    byte buffer[sizeof(Message)];

    // Receive message
    for (int i = 0; i < sizeof(Message) && Wire.available(); i++) {
        buffer[i] = Wire.read();
    }

    // Parse message
    Message* m = ((Message*)&buffer);
    print_message(m);

    // TODO: handle the message
    // note that `m` is backed by `buffer` which is local
    // need to make it global if we want to process `m` outside this handler
}

// Prints a message in JSON format
void print_message(Message* m) {
    Serial.print("{ ");
    Serial.print("dst_x: "); Serial.print(m->dst_x); Serial.print(", ");
    Serial.print("dst_y: "); Serial.print(m->dst_y); Serial.print(", ");
    Serial.print("src_x: "); Serial.print(m->src_x); Serial.print(", ");
    Serial.print("src_y: "); Serial.print(m->src_y); Serial.print(", ");
    Serial.print("event: "); Serial.print(m->event); Serial.print(", ");
    Serial.print("cars_n: "); Serial.print(m->cars_n); Serial.print(", ");
    Serial.print("cars_s: "); Serial.print(m->cars_s); Serial.print(", ");
    Serial.print("cars_e: "); Serial.print(m->cars_e); Serial.print(", ");
    Serial.print("cars_w: "); Serial.print(m->cars_w); Serial.print(", ");
    Serial.print("timestamp: "); Serial.print(m->timestamp);
    Serial.println(" }");
}

void mode_standby() {
    digitalWrite(TL_NS_R, LOW);
    digitalWrite(TL_NS_Y, LOW);
    digitalWrite(TL_NS_G, LOW);
    digitalWrite(TL_EW_R, LOW);
    digitalWrite(TL_EW_Y, LOW);
    digitalWrite(TL_EW_G, LOW);

    // Blink yellow lights with a 2-second period
    delay(1000);
    digitalWrite(TL_NS_Y, HIGH);
    digitalWrite(TL_EW_Y, HIGH);
    delay(1000);
    digitalWrite(TL_NS_Y, LOW);
    digitalWrite(TL_EW_Y, LOW);
}

void cars_ns() {
    if (orientation_s) {
        cars_s++;
    } else {
        cars_n++;
    }
}

void cars_ew() {
    if (orientation_w) {
        cars_w++;
    } else {
        cars_e++;
    }
}
