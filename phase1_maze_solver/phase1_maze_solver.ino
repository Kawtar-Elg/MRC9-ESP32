/*
 * MRC 9.0 - Moroccan Robotics Challenge 2026
 * Phase 1: Autonomous Maze Solver
 *
 * Hardware:
 *   - ESP32 (main controller)
 *   - L298N motor driver
 *   - 4x DC motors
 *   - 3x HC-SR04 ultrasonic sensors (front, left, right)
 *
 * Algorithm: Left-hand wall following rule
 *   1. If left is open → turn left then move forward
 *   2. Else if front is open → move forward
 *   3. Else if right is open → turn right then move forward
 *   4. Else (dead end) → turn 180°
 */

#include <Arduino.h>

/* ───────────────────────── PIN MAP ───────────────────────── */
/* L298N */
#define IN1  26
#define IN2  27
#define IN3  14
#define IN4  12
#define ENA  25   /* Left  motor PWM – LEDC channel 0 */
#define ENB  33   /* Right motor PWM – LEDC channel 1 */

/* HC-SR04 – Front */
#define TRIG_F  18
#define ECHO_F  19
/* HC-SR04 – Left */
#define TRIG_L  21
#define ECHO_L  22
/* HC-SR04 – Right */
#define TRIG_R  23
#define ECHO_R   5

/* ───────────────────────── CONSTANTS ──────────────────────── */
#define WALL_DIST_CM    15      /* distance below = wall present    */
#define CELL_MOVE_MS   450      /* ms to advance one maze cell      */
#define TURN_90_MS     380      /* ms for a 90° pivot               */
#define TURN_180_MS    760      /* ms for a 180° pivot              */
#define SPEED_MOVE     180      /* motor PWM 0–255                  */
#define SPEED_TURN     160

/* ───────────────────────── PROTOTYPES ─────────────────────── */
static float  ultrasonic_cm(uint8_t trig, uint8_t echo);
static void   motors_forward(uint8_t spd);
static void   motors_backward(uint8_t spd);
static void   motors_stop(void);
static void   motors_pivot_left(uint8_t spd);
static void   motors_pivot_right(uint8_t spd);
static void   do_turn_left_90(void);
static void   do_turn_right_90(void);
static void   do_turn_180(void);
static void   do_move_forward(void);

/* ═══════════════════════════ SETUP ════════════════════════════ */
void setup(void)
{
    Serial.begin(115200);

    /* Motor direction pins */
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

    /* LEDC PWM channels for motor speed */
    ledcSetup(0, 5000, 8); ledcAttachPin(ENA, 0);
    ledcSetup(1, 5000, 8); ledcAttachPin(ENB, 1);

    /* Ultrasonic pins */
    pinMode(TRIG_F, OUTPUT); pinMode(ECHO_F, INPUT);
    pinMode(TRIG_L, OUTPUT); pinMode(ECHO_L, INPUT);
    pinMode(TRIG_R, OUTPUT); pinMode(ECHO_R, INPUT);

    motors_stop();
    Serial.println("[MRC9] Phase 1 ready – starting in 3 s");
    delay(3000);
}

/* ═══════════════════════════ LOOP ═════════════════════════════ */
void loop(void)
{
    float f = ultrasonic_cm(TRIG_F, ECHO_F);
    float l = ultrasonic_cm(TRIG_L, ECHO_L);
    float r = ultrasonic_cm(TRIG_R, ECHO_R);

    Serial.printf("[SENSOR] F=%.1f  L=%.1f  R=%.1f cm\n", f, l, r);

    /* Left-hand wall following */
    if (l > WALL_DIST_CM) {
        Serial.println("[NAV] Turn LEFT");
        do_turn_left_90();
        do_move_forward();
    } else if (f > WALL_DIST_CM) {
        Serial.println("[NAV] Forward");
        do_move_forward();
    } else if (r > WALL_DIST_CM) {
        Serial.println("[NAV] Turn RIGHT");
        do_turn_right_90();
        do_move_forward();
    } else {
        Serial.println("[NAV] Dead-end – Turn 180");
        do_turn_180();
    }

    delay(50);
}

/* ═══════════════════════ ULTRASONIC ═══════════════════════════ */
static float ultrasonic_cm(uint8_t trig, uint8_t echo)
{
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    long us = pulseIn(echo, HIGH, 30000UL); /* 30 ms timeout */
    return (us == 0) ? 999.0f : (us * 0.0343f / 2.0f);
}

/* ═════════════════════ MOTOR PRIMITIVES ═══════════════════════ */
static void motors_forward(uint8_t spd)
{
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    ledcWrite(0, spd); ledcWrite(1, spd);
}

static void motors_backward(uint8_t spd)
{
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    ledcWrite(0, spd); ledcWrite(1, spd);
}

static void motors_stop(void)
{
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    ledcWrite(0, 0); ledcWrite(1, 0);
}

/* Pivot left: right wheels forward, left wheels backward */
static void motors_pivot_left(uint8_t spd)
{
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    ledcWrite(0, spd); ledcWrite(1, spd);
}

/* Pivot right: left wheels forward, right wheels backward */
static void motors_pivot_right(uint8_t spd)
{
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
    ledcWrite(0, spd); ledcWrite(1, spd);
}

/* ═══════════════════════ NAVIGATION ═══════════════════════════ */
static void do_turn_left_90(void)
{
    motors_pivot_left(SPEED_TURN);
    delay(TURN_90_MS);
    motors_stop();
    delay(100);
}

static void do_turn_right_90(void)
{
    motors_pivot_right(SPEED_TURN);
    delay(TURN_90_MS);
    motors_stop();
    delay(100);
}

static void do_turn_180(void)
{
    motors_pivot_right(SPEED_TURN);
    delay(TURN_180_MS);
    motors_stop();
    delay(100);
}

static void do_move_forward(void)
{
    motors_forward(SPEED_MOVE);
    delay(CELL_MOVE_MS);
    motors_stop();
    delay(100);
}
