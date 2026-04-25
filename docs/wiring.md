# MRC 9.0 – ESP32 Wiring Guide

## L298N Motor Driver → ESP32

| L298N Pin | ESP32 GPIO | Description              |
|-----------|-----------|--------------------------|
| IN1       | GPIO 26   | Left motor direction A   |
| IN2       | GPIO 27   | Left motor direction B   |
| IN3       | GPIO 14   | Right motor direction A  |
| IN4       | GPIO 12   | Right motor direction B  |
| ENA       | GPIO 25   | Left motor PWM speed     |
| ENB       | GPIO 33   | Right motor PWM speed    |
| GND       | GND       | Common ground            |
| VCC       | Battery+  | Motor power (7.4 V)      |
| 5V out    | 5V ESP32  | Logic power (optional)   |

> Connect all 3 batteries in series for ~11.1 V, or 2S for ~7.4 V.

## HC-SR04 Ultrasonic Sensors

| Sensor  | TRIG      | ECHO      |
|---------|-----------|-----------|
| Front   | GPIO 18   | GPIO 19   |
| Left    | GPIO 21   | GPIO 22   |
| Right   | GPIO 23   | GPIO 5    |

Power: VCC → 5V, GND → GND

## Relay Module (Water Pump)

| Relay Pin | ESP32     |
|-----------|-----------|
| IN        | GPIO 4    |
| VCC       | 5V        |
| GND       | GND       |

Pump connects across relay NO/COM terminals.

## Servo Motor (Gripper)

| Servo Pin | ESP32     |
|-----------|-----------|
| Signal    | GPIO 13   |
| VCC       | 5V        |
| GND       | GND       |

## Power Supply

```
3× 3.7V Li-ion batteries
  ├── 2S in series (7.4V) → L298N VCC (motors)
  └── 3S in series (11.1V) via 5V regulator → ESP32 VIN
```

> **Important:** Connect ESP32 GND and L298N GND together.
