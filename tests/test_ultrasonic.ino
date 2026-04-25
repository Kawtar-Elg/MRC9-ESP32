/*
 * HC-SR04 Ultrasonic Sensor Test – Arduino Uno
 * Prints distance every 500 ms to Serial Monitor (9600 baud)
 *
 * Wiring:
 *   TRIG → pin 2
 *   ECHO → pin 3
 *   VCC  → 5V    GND → GND
 */

#define TRIG  2
#define ECHO  3

float read_cm() {
    digitalWrite(TRIG, LOW);  delayMicroseconds(2);
    digitalWrite(TRIG, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    long us = pulseIn(ECHO, HIGH, 30000UL);
    return (us == 0) ? -1 : us * 0.0343f / 2.0f;
}

void setup() {
    Serial.begin(9600);
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    Serial.println("Ultrasonic test start");
}

void loop() {
    float d = read_cm();
    if (d < 0)
        Serial.println("No echo (out of range)");
    else
        Serial.print("Distance: "), Serial.print(d, 1), Serial.println(" cm");
    delay(500);
}
