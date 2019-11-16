/* Applications and Computation for the Internet of Things 2019-2020
 * Project: Intelligent control of traffic flow
 *
 * Group 11
 * Diogo Pereira (58122)
 * Pedro Carlos  (87556)
 */

#include <Wire.h>


// Identification of the inputs (loop detectors)
const int LD_S = 1;
const int LD_W = 2;

// Identification of the outputs (traffic lights)
const int TL_S = 3;
const int TL_W = 4;

// All units are hundreds of milliseconds
const unsigned long TIME_UNIT_MILLIS = 100;
const unsigned long PERIOD = 200;
const unsigned long MIN_DUTY_CYCLE = 5;
const unsigned long MAX_DUTY_CYCLE = 15;

const byte I2C_ADDRESS = 8;

void setup() {
    Serial.begin(9600);
    Wire.begin();
}

// Definition of the message format
struct Message {
    byte destination_x : 4;
    byte destination_y : 4;
    byte source_x : 4;
    byte source_y : 4;
    byte event;
    byte cars_n;
    byte cars_s;
    byte cars_e;
    byte cars_w;
    unsigned long timestamp;
};

void loop() {
    Message m = {
        .destination_x = 1,
        .destination_y = 3,
        .source_x = 2,
        .source_y = 2,
        .event = 3,
        .cars_n = 0,
        .cars_s = 0,
        .cars_e = 0,
        .cars_w = 0,
        .timestamp = 12345
    };

    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((byte*)&m, sizeof(m));
    Wire.endTransmission();
}
