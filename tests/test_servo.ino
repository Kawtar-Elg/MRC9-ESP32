/*
 * Servo Test – moves to 0°, 90°, 180° every 2 seconds
 * Board : ESP32
 * Library: ESP32Servo (install via Library Manager)
 */

#include <Arduino.h>
#include <ESP32Servo.h>

#define SERVO_PIN  13   /* change if needed */

Servo srv;

void setup() {
    Serial.begin(115200);
    srv.attach(SERVO_PIN);
    Serial.println("Servo test start");
}

void loop() {
    Serial.println("→ 0°");
    srv.write(0);
    delay(2000);

    Serial.println("→ 90°");
    srv.write(90);
    delay(2000);

    Serial.println("→ 180°");
    srv.write(180);
    delay(2000);
}
