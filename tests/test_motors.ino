/*
 * L298N Motor Driver Test – Arduino Uno
 * Sequence: forward → backward → turn left → turn right → stop
 *
 * Wiring:
 *   IN1 → pin 5   IN2 → pin 6   (left motors)
 *   IN3 → pin 7   IN4 → pin 8   (right motors)
 *   ENA → pin 9   ENB → pin 10  (PWM speed)
 */

#define IN1  5
#define IN2  6
#define IN3  7
#define IN4  8
#define ENA  9
#define ENB  10

#define SPEED  180   /* 0–255 */

void motors_forward()  { digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  analogWrite(ENA,SPEED); analogWrite(ENB,SPEED); }
void motors_backward() { digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); analogWrite(ENA,SPEED); analogWrite(ENB,SPEED); }
void motors_left()     { digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  analogWrite(ENA,SPEED); analogWrite(ENB,SPEED); }
void motors_right()    { digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); analogWrite(ENA,SPEED); analogWrite(ENB,SPEED); }
void motors_stop()     { digitalWrite(IN1,LOW);  digitalWrite(IN2,LOW);  digitalWrite(IN3,LOW);  digitalWrite(IN4,LOW);  analogWrite(ENA,0);     analogWrite(ENB,0); }

void setup() {
    Serial.begin(9600);
    pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
    pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);
    pinMode(ENA,OUTPUT); pinMode(ENB,OUTPUT);
    motors_stop();
    Serial.println("Motor test start");
    delay(2000);
}

void loop() {
    Serial.println("-> FORWARD");  motors_forward();  delay(1500);
    Serial.println("-> STOP");     motors_stop();     delay(500);
    Serial.println("-> BACKWARD"); motors_backward(); delay(1500);
    Serial.println("-> STOP");     motors_stop();     delay(500);
    Serial.println("-> LEFT");     motors_left();     delay(800);
    Serial.println("-> STOP");     motors_stop();     delay(500);
    Serial.println("-> RIGHT");    motors_right();    delay(800);
    Serial.println("-> STOP");     motors_stop();     delay(2000);
}
