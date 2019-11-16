/* Applications and Computation for the Internet of Things 2019-2020
 * Project: Intelligent control of traffic flow
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */

#include <Wire.h>


// Identification of the inputs (loop detectors)
const int LD_1 = 1;
const int LD_2 = 2;

// Identification of the outputs (traffic lights)
const int TL1_R = 3;
const int TL1_Y = 4;
const int TL1_G = 5;
const int TL2_R = 6;
const int TL2_Y = 7;
const int TL2_G = 8;

// All units are hundreds of milliseconds
const unsigned long TIME_UNIT_MILLIS = 100;
const unsigned long PERIOD = 200;
const unsigned long MIN_DUTY_CYCLE = 5;
const unsigned long MAX_DUTY_CYCLE = 15;

const byte I2C_ADDRESS = 8;

// Definition of the message format
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


void setup() {
    pinMode(TL1_R, OUTPUT);
    pinMode(TL1_Y, OUTPUT);
    pinMode(TL1_G, OUTPUT);
    pinMode(TL2_R, OUTPUT);
    pinMode(TL2_Y, OUTPUT);
    pinMode(TL2_G, OUTPUT);

    // Initial state of the system (at an intersection)
    // All traffic lights blinking yellow with a 2 seconds
    // period (ON + OFF cycle time) during 6 seconds.
    for(int i = 0; i < 3; i++) {
        standby_state();
    }

    Serial.begin(9600);
    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(message_received);
}

void loop() {
    // TODO: this is just a demo of sending/receiving messages
    static unsigned long last_message_sent = millis();

    Message m = {
        .dst_x = 1,
        .dst_y = 3,
        .src_x = 2,
        .src_y = 2,
        .event = 3,
        .cars_n = 0,
        .cars_s = 0,
        .cars_e = 0,
        .cars_w = 0,
        .timestamp = 12345
    };

    if (millis() > last_message_sent + 10*PERIOD) {
        Serial.println("Sending message");
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write((byte*)&m, sizeof(m));
        Wire.endTransmission();
        Serial.println("Message sent");
        last_message_sent = millis();
    }
}

void message_received(int num_bytes) {
    byte buffer[sizeof(Message)];

    // Receive message
    Serial.println("Receiving message");
    for (int i = 0; i < sizeof(Message) && Wire.available(); i++) {
        buffer[i] = Wire.read();
    }
    Serial.println("Message received");

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

void standby_state() {
    digitalWrite(TL1_R, LOW);
    digitalWrite(TL1_Y, LOW);
    digitalWrite(TL1_G, LOW);
    digitalWrite(TL2_R, LOW);
    digitalWrite(TL2_Y, LOW);
    digitalWrite(TL2_G, LOW);

    // Blink yellow lights with a 2-second period
    delay(1000);
    digitalWrite(TL1_Y, HIGH);
    digitalWrite(TL2_Y, HIGH);
    delay(1000);
    digitalWrite(TL1_Y, LOW);
    digitalWrite(TL2_Y, LOW);
}
