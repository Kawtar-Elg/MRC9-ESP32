/*
 * Servo Test – moves to 0°, 90°, 180° every 2 seconds
 * Board : Arduino Uno  (uses built-in Servo library)
 * Pin   : D9
 */

#include <Servo.h>

#define SERVO_PIN  9

Servo srv;

void setup() {
    Serial.begin(9600);
    srv.attach(SERVO_PIN);
    Serial.println("Servo test start");
}

void loop() {
    Serial.println("-> 0 deg");
    srv.write(0);
    delay(2000);

    Serial.println("-> 90 deg");
    srv.write(90);
    delay(2000);

    Serial.println("-> 180 deg");
    srv.write(180);
    delay(2000);
}
