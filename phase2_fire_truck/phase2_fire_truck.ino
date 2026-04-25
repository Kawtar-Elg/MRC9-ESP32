/*
 * MRC 9.0 - Moroccan Robotics Challenge 2026
 * Phase 2: Remote-Controlled Fire Truck
 *
 * Hardware:
 *   - ESP32 (main controller + WiFi AP)
 *   - L298N motor driver + 4x DC motors
 *   - 3x HC-SR04 ultrasonic sensors (front, left, right)
 *   - 1x Relay → water pump (120 L/h 3-6 V)
 *   - 1x Servo motor 180° (gripper)
 *
 * Control: Connect phone/laptop to WiFi "MRC9-FireTruck" (pass: mrc2026)
 *          then open http://192.168.4.1 in your browser.
 *
 * HTTP commands  (GET /cmd?c=X):
 *   F  – forward          B – backward
 *   L  – turn left        R – turn right
 *   S  – stop
 *   P1 – pump ON          P0 – pump OFF
 *   G1 – gripper OPEN     G0 – gripper CLOSE
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

/* ───────────────────────── PIN MAP ───────────────────────── */
/* L298N */
#define IN1  26
#define IN2  27
#define IN3  14
#define IN4  12
#define ENA  25   /* Left  motor PWM – LEDC channel 0 */
#define ENB  33   /* Right motor PWM – LEDC channel 1 */

/* Relay → water pump */
#define RELAY_PIN  4

/* Servo motor (gripper) */
#define SERVO_PIN  13

/* HC-SR04 – Front obstacle detection */
#define TRIG_F  18
#define ECHO_F  19
/* HC-SR04 – Left */
#define TRIG_L  21
#define ECHO_L  22
/* HC-SR04 – Right */
#define TRIG_R  23
#define ECHO_R   5

/* ───────────────────────── CONSTANTS ──────────────────────── */
#define AP_SSID         "MRC9-FireTruck"
#define AP_PASS         "mrc2026"

#define SPEED_NORMAL    200
#define SPEED_TURN      160

#define SERVO_OPEN_DEG   90   /* gripper open  position */
#define SERVO_CLOSE_DEG   0   /* gripper close position */

/* ───────────────────────── GLOBALS ────────────────────────── */
WebServer server(80);
Servo    gripper;

/* ═══════════════════════ HTML PAGE ════════════════════════════
   Served to the operator's phone/browser.
   Touch-and-hold buttons: motor runs while held, stops on release.
   Single-tap buttons: pump & gripper toggles.
   ─────────────────────────────────────────────────────────── */
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>MRC 9.0 – Fire Truck</title>
  <style>
    * { box-sizing:border-box; margin:0; padding:0; }
    body {
      background:#0d1117; color:#e6edf3;
      font-family:Arial,sans-serif; text-align:center;
      user-select:none; -webkit-user-select:none;
    }
    h1  { color:#ff6b35; padding:18px; font-size:1.5rem; }
    h2  { color:#8b949e; font-size:0.9rem; padding-bottom:12px; }
    .panel { padding:10px; }
    .btn {
      display:inline-block; width:80px; height:80px;
      background:#161b22; border:2px solid #30363d;
      color:#e6edf3; font-size:2rem; border-radius:12px;
      margin:6px; cursor:pointer; line-height:80px;
      transition:background .1s, border-color .1s;
    }
    .btn:active, .btn.active { background:#ff6b35; border-color:#ff6b35; }
    .pump  { background:#b91c1c; border-color:#ef4444; width:140px; height:56px; font-size:1rem; line-height:56px; }
    .pump-off { background:#15803d; border-color:#22c55e; }
    .grip  { background:#1d4ed8; border-color:#3b82f6; width:140px; height:56px; font-size:1rem; line-height:56px; }
    #status { margin-top:14px; color:#8b949e; font-size:0.85rem; }
    .sensors { margin-top:16px; font-size:0.85rem; color:#58a6ff; }
  </style>
</head>
<body>
  <h1>&#x1F692; Fire Truck Control</h1>
  <h2>MRC 9.0 — ENSMR Rabat 2026</h2>

  <!-- Movement -->
  <div class="panel">
    <div>
      <span class="btn" id="F" ontouchstart="hold('F')" ontouchend="release()"
            onmousedown="hold('F')" onmouseup="release()">&#x25B2;</span>
    </div>
    <div>
      <span class="btn" id="L" ontouchstart="hold('L')" ontouchend="release()"
            onmousedown="hold('L')" onmouseup="release()">&#x25C4;</span>
      <span class="btn" id="S" ontouchstart="send('S')" onmousedown="send('S')">&#x25A0;</span>
      <span class="btn" id="R" ontouchstart="hold('R')" ontouchend="release()"
            onmousedown="hold('R')" onmouseup="release()">&#x25BA;</span>
    </div>
    <div>
      <span class="btn" id="B" ontouchstart="hold('B')" ontouchend="release()"
            onmousedown="hold('B')" onmouseup="release()">&#x25BC;</span>
    </div>
  </div>

  <!-- Pump & Gripper -->
  <div class="panel">
    <span class="btn pump"     onclick="send('P1')">&#x1F4A7; PUMP ON</span>
    <span class="btn pump pump-off" onclick="send('P0')">&#x274C; PUMP OFF</span>
  </div>
  <div class="panel">
    <span class="btn grip" onclick="send('G1')">&#x1F916; OPEN</span>
    <span class="btn grip" onclick="send('G0')" style="background:#7c3aed;border-color:#a855f7">&#x270A; CLOSE</span>
  </div>

  <div id="status">Connecting...</div>
  <div class="sensors" id="sensors"></div>

  <script>
    let activeCmd = null;
    let interval  = null;

    function send(cmd) {
      fetch('/cmd?c=' + cmd)
        .then(r => r.text())
        .then(t => { document.getElementById('status').innerText = t; });
    }

    function hold(cmd) {
      if (activeCmd === cmd) return;
      activeCmd = cmd;
      send(cmd);
      interval = setInterval(() => send(cmd), 150);
    }

    function release() {
      clearInterval(interval);
      interval = null;
      activeCmd = null;
      send('S');
    }

    /* Poll sensors every second */
    setInterval(() => {
      fetch('/sensors')
        .then(r => r.text())
        .then(t => { document.getElementById('sensors').innerText = t; });
    }, 1000);
  </script>
</body>
</html>
)HTML";

/* ═══════════════════════ PROTOTYPES ═══════════════════════════ */
static float  ultrasonic_cm(uint8_t trig, uint8_t echo);
static void   motors_forward(uint8_t spd);
static void   motors_backward(uint8_t spd);
static void   motors_stop(void);
static void   motors_left(uint8_t spd);
static void   motors_right(uint8_t spd);
static void   handle_root(void);
static void   handle_cmd(void);
static void   handle_sensors(void);

/* ═══════════════════════════ SETUP ════════════════════════════ */
void setup(void)
{
    Serial.begin(115200);

    /* Motor pins */
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    ledcSetup(0, 5000, 8); ledcAttachPin(ENA, 0);
    ledcSetup(1, 5000, 8); ledcAttachPin(ENB, 1);

    /* Relay */
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); /* pump OFF at boot */

    /* Servo */
    gripper.attach(SERVO_PIN);
    gripper.write(SERVO_CLOSE_DEG);

    /* Ultrasonic */
    pinMode(TRIG_F, OUTPUT); pinMode(ECHO_F, INPUT);
    pinMode(TRIG_L, OUTPUT); pinMode(ECHO_L, INPUT);
    pinMode(TRIG_R, OUTPUT); pinMode(ECHO_R, INPUT);

    motors_stop();

    /* Start WiFi AP */
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.printf("[WiFi] AP started  SSID=%s  IP=%s\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());

    /* HTTP routes */
    server.on("/",        HTTP_GET, handle_root);
    server.on("/cmd",     HTTP_GET, handle_cmd);
    server.on("/sensors", HTTP_GET, handle_sensors);
    server.begin();

    Serial.println("[MRC9] Phase 2 ready – open http://192.168.4.1");
}

/* ═══════════════════════════ LOOP ═════════════════════════════ */
void loop(void)
{
    server.handleClient();
}

/* ═══════════════════════ HTTP HANDLERS ════════════════════════ */
static void handle_root(void)
{
    server.send_P(200, "text/html", INDEX_HTML);
}

static void handle_cmd(void)
{
    if (!server.hasArg("c")) { server.send(400, "text/plain", "Bad request"); return; }

    String cmd = server.arg("c");
    String reply = "OK: " + cmd;

    if      (cmd == "F")  { motors_forward(SPEED_NORMAL);  reply = "Moving FORWARD"; }
    else if (cmd == "B")  { motors_backward(SPEED_NORMAL); reply = "Moving BACKWARD"; }
    else if (cmd == "L")  { motors_left(SPEED_TURN);       reply = "Turning LEFT"; }
    else if (cmd == "R")  { motors_right(SPEED_TURN);      reply = "Turning RIGHT"; }
    else if (cmd == "S")  { motors_stop();                  reply = "STOPPED"; }
    else if (cmd == "P1") { digitalWrite(RELAY_PIN, HIGH);  reply = "Pump ON"; }
    else if (cmd == "P0") { digitalWrite(RELAY_PIN, LOW);   reply = "Pump OFF"; }
    else if (cmd == "G1") { gripper.write(SERVO_OPEN_DEG);  reply = "Gripper OPEN"; }
    else if (cmd == "G0") { gripper.write(SERVO_CLOSE_DEG); reply = "Gripper CLOSE"; }
    else                  { reply = "Unknown command"; }

    Serial.println("[CMD] " + reply);
    server.send(200, "text/plain", reply);
}

static void handle_sensors(void)
{
    float f = ultrasonic_cm(TRIG_F, ECHO_F);
    float l = ultrasonic_cm(TRIG_L, ECHO_L);
    float r = ultrasonic_cm(TRIG_R, ECHO_R);
    char buf[64];
    snprintf(buf, sizeof(buf), "Front: %.1f cm | Left: %.1f cm | Right: %.1f cm", f, l, r);
    server.send(200, "text/plain", buf);
}

/* ═══════════════════════ ULTRASONIC ═══════════════════════════ */
static float ultrasonic_cm(uint8_t trig, uint8_t echo)
{
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long us = pulseIn(echo, HIGH, 30000UL);
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

static void motors_left(uint8_t spd)
{
    /* Pivot left: left wheels backward, right wheels forward */
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    ledcWrite(0, spd); ledcWrite(1, spd);
}

static void motors_right(uint8_t spd)
{
    /* Pivot right: left wheels forward, right wheels backward */
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
    ledcWrite(0, spd); ledcWrite(1, spd);
}
