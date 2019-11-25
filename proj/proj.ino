/* Applications and Computation for the Internet of Things 2019-2020
 * Project: Intelligent control of traffic flow
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */

#include <Wire.h>


// Identification of the inputs (orientation pins)
const int COORDINATE_X0 = A0;
const int COORDINATE_X1 = A1;
const int COORDINATE_Y0 = A2;
const int COORDINATE_Y1 = A3;

// Identification of the inputs (orientation pins)
const int ORIENTATION_S_PIN = 10;
const int ORIENTATION_W_PIN = 11;

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

// Time for a car to go from one intersection to the next (hundreds of ms)
const unsigned long TRAVEL_TIME = 60;

const byte I2C_ADDRESS = 8;

enum Direction {
    North = 0,
    South = 1,
    East = 2,
    West = 3
};

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
void cars_ns();
void cars_ew();
void mode_standby();
void message_received(int num_bytes);
void switch_lights(Event e, int total_duration);

byte mode = 2;

// Cars counted at the north/south/east/west access during the last period
volatile byte cars[] = {0, 0, 0, 0};

byte coordinate_x = 0;
byte coordinate_y = 0;
byte orientation_s = false;
byte orientation_w = false;

unsigned long period_start_ns = 0;
unsigned long period_start_ew = 0;
volatile unsigned long neighbor_period_start = 0;
volatile bool adjust_to_neighbor = false;
volatile long phase_diff = 0;


void setup() {
    // Input pins for the identification of the intersection
    pinMode(COORDINATE_X0, INPUT_PULLUP);
    pinMode(COORDINATE_X1, INPUT_PULLUP);
    pinMode(COORDINATE_Y0, INPUT_PULLUP);
    pinMode(COORDINATE_Y1, INPUT_PULLUP);

    coordinate_x = (digitalRead(COORDINATE_X1) == HIGH ? 2 : 0) + (digitalRead(COORDINATE_X0) == HIGH ? 1 : 0);
    coordinate_y = (digitalRead(COORDINATE_Y1) == HIGH ? 2 : 0) + (digitalRead(COORDINATE_Y0) == HIGH ? 1 : 0);

    // Input pins for the orientation of the intersection
    pinMode(ORIENTATION_S_PIN, INPUT_PULLUP);
    pinMode(ORIENTATION_W_PIN, INPUT_PULLUP);

    orientation_s = digitalRead(ORIENTATION_S_PIN) == HIGH;
    orientation_w = digitalRead(ORIENTATION_W_PIN) == HIGH;

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

    if (mode == 1) {
        if (cars[North] + cars[South] + cars[East] + cars[West] > 0) {
            duty_cycle = PERIOD * (cars[North] + cars[South]) / (cars[North] + cars[South] + cars[East] + cars[West]);
            if (duty_cycle < MIN_DUTY_CYCLE) duty_cycle = MIN_DUTY_CYCLE;
            if (duty_cycle > MAX_DUTY_CYCLE) duty_cycle = MAX_DUTY_CYCLE;
        }
    }
    cars[North] = cars[South] = cars[East] = cars[West] = 0;

    switch_lights(R2G_N, duty_cycle, false);
    switch_lights(R2G_E, PERIOD - duty_cycle, false);
}

void mode2() {
    unsigned long duty_cycle = PERIOD / 2;

    if (cars[North] + cars[South] + cars[East] + cars[West] > 0) {
        duty_cycle = PERIOD * (cars[North] + cars[South]) / (cars[North] + cars[South] + cars[East] + cars[West]);
        if (duty_cycle < MIN_DUTY_CYCLE) duty_cycle = MIN_DUTY_CYCLE;
        if (duty_cycle > MAX_DUTY_CYCLE) duty_cycle = MAX_DUTY_CYCLE;
    }

    byte last_cars[] = {cars[North], cars[South], cars[East], cars[West]};
    cars[North] = cars[South] = cars[East] = cars[West] = 0;

    Message m_ns = {
        .dst_x = coordinate_x,
        .dst_y = orientation_s ? coordinate_y + 1 : coordinate_y - 1,
        .src_x = coordinate_x,
        .src_y = coordinate_y,
        .event = orientation_s ? R2G_S : R2G_N,
        .cars_n = last_cars[North],
        .cars_s = last_cars[South],
        .cars_e = last_cars[East],
        .cars_w = last_cars[West],
        .timestamp = millis()/100
    };
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((byte*)&m_ns, sizeof(m_ns));
    Wire.endTransmission();

    period_start_ns = millis()/100;

    switch_lights(R2G_N, duty_cycle, true);

    Message m_ew = {
        .dst_x = orientation_w ? coordinate_x + 1 : coordinate_x - 1,
        .dst_y = coordinate_y,
        .src_x = coordinate_x,
        .src_y = coordinate_y,
        .event = orientation_w ? R2G_W : R2G_E,
        .cars_n = last_cars[North],
        .cars_s = last_cars[South],
        .cars_e = last_cars[East],
        .cars_w = last_cars[West],
        .timestamp = millis()/100
    };
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((byte*)&m_ew, sizeof(m_ew));
    Wire.endTransmission();

    period_start_ew = millis()/100;

    switch_lights(R2G_E, PERIOD - duty_cycle, false);
}


void loop() {
    switch (mode) {
        case 0:
        case 1:
            mode01();
            break;
        case 2:
            mode2();
            break;
        default:
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

    Direction max_flow_direction = get_max_flow_direction(cars);

    if(cars[max_flow_direction] == 0) {
        adjust_to_neighbor = false;
    } else if(m->dst_x == coordinate_x && m->dst_y == coordinate_y
              && buffer[1] == get_neighbor(max_flow_direction)
              && max_flow_direction == get_max_flow_direction(&buffer[3])) {
        neighbor_period_start = millis()/100;
        adjust_to_neighbor = true;
        if(m->event == R2G_N | m->event == R2G_S) {
            phase_diff = PERIOD/100 + ((long)period_start_ns - (long)neighbor_period_start);
        } else {
            phase_diff = PERIOD/100 + ((long)period_start_ew - (long)neighbor_period_start);
        }
        Serial.print("Phase: "); Serial.println(phase_diff);
    }
}

Direction get_max_flow_direction(byte* cars) {
    byte maximum = 0;
    byte argmax = 0;
    for(int i = 0; i < 4; i++) {
        //Serial.print("cars[");Serial.print(i);Serial.print("] = ");Serial.println(cars[i]);
        if(cars[i] > maximum) {
            maximum = cars[i];
            argmax = i;
        }
    }
    return argmax;
}

byte get_neighbor(Direction dir) {
    int dest_x;
    int dest_y;

    switch(dir) {
        case North:
            dest_x = coordinate_x;
            dest_y = coordinate_y + 1;
            break;
        case South:
            dest_x = coordinate_x;
            dest_y = coordinate_y - 1;
            break;
        case East:
            dest_x = coordinate_x + 1;
            dest_y = coordinate_y;
            break;
        case West:
            dest_x = coordinate_x - 1;
            dest_y = coordinate_y;
            break;
    }

    if(dest_x < 0 || dest_x >= (1<<4) ||
       dest_y < 0 || dest_y >= (1<<4)) {
        return coordinate_x | (coordinate_y << 4);
    }
    return dest_x | (dest_y << 4);
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
        cars[South]++;
    } else {
        cars[North]++;
    }
}

void cars_ew() {
    if (orientation_w) {
        cars[West]++;
    } else {
        cars[East]++;
    }
}

void switch_lights(Event e, int total_duration, bool phase_adjust) {
    int tl_g;
    int tl_r;
    int tl_r_ok;
    static byte desired_adjustment = 0;

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

    if(phase_adjust && adjust_to_neighbor) {
        desired_adjustment = 60 - phase_diff;
        Serial.print("Desired adjustment: "); Serial.println(desired_adjustment);
    }
    if(adjust_to_neighbor && desired_adjustment >= 20) {
        delay(1000);
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
