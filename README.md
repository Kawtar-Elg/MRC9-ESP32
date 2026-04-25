# MRC 9.0 – Moroccan Robotics Challenge 2026

**Event:** 25 & 26 April 2026 – École Nationale Supérieure des Mines de Rabat  
**Organized by:** MINES MAKERS club / AEENIM

---

## Hardware Kit

| Component               | Qty | Notes                      |
|-------------------------|-----|----------------------------|
| ESP32 board             | 1   | Main microcontroller       |
| L298N motor driver      | 1   | Controls 4 DC motors       |
| DC motors               | 4   | Wheel drive                |
| HC-SR04 ultrasonic      | 3   | Front / Left / Right       |
| Servo motor (180°)      | 1   | Gripper actuation          |
| Relay module            | 1   | Controls water pump        |
| Water pump 3-6V 120L/h  | 1   | Fire extinguishing (P2)    |
| 3.7V Li-ion battery     | 3   | Power supply               |
| Car chassis (4-wheel)   | 1   | Mechanical frame           |

---

## Repository Structure

```
MRC9-ESP32/
├── phase1_maze_solver/
│   └── phase1_maze_solver.ino   ← Upload for Phase 1
├── phase2_fire_truck/
│   └── phase2_fire_truck.ino    ← Upload for Phase 2
├── docs/
│   └── wiring.md                ← Pin wiring reference
└── README.md
```

---

## Phase 1 – Autonomous Maze Solver

The robot navigates a closed maze from **START → FINISH** with no human input.

**Algorithm:** Left-hand wall following rule  
1. If **left** path is open → turn left, advance  
2. Else if **front** path is open → advance  
3. Else if **right** path is open → turn right, advance  
4. Else (dead end) → turn 180°

**Tuning parameters** in `phase1_maze_solver.ino`:

```c
#define WALL_DIST_CM   15    // wall detection threshold (cm)
#define CELL_MOVE_MS  450    // time to cross one maze cell (ms)
#define TURN_90_MS    380    // time for a 90° turn (ms)
#define SPEED_MOVE    180    // motor speed 0-255
#define SPEED_TURN    160
```

> Adjust `TURN_90_MS` and `CELL_MOVE_MS` on your actual robot until
> turns are exactly 90° and the cell advance is correct.

---

## Phase 2 – Remote-Controlled Fire Truck

The robot is controlled by an operator via a **mobile web app** over WiFi.

**Setup:**
1. Flash `phase2_fire_truck.ino` to the ESP32
2. On your phone/laptop, connect to WiFi: **`MRC9-FireTruck`** (password: `mrc2026`)
3. Open browser → `http://192.168.4.1`

**Controls:**

| Button          | Action                   |
|-----------------|--------------------------|
| ▲ (hold)        | Forward                  |
| ▼ (hold)        | Backward                 |
| ◄ (hold)        | Turn left                |
| ► (hold)        | Turn right               |
| ■               | Stop                     |
| 💧 PUMP ON      | Activate water pump      |
| ✕ PUMP OFF      | Deactivate water pump    |
| OPEN / CLOSE    | Gripper servo control    |

Live sensor readings (F/L/R distances) update every second on the control page.

---

## Arduino IDE Setup

1. Install **Arduino IDE 2.x**
2. Add ESP32 board package:  
   `File → Preferences → Additional Boards URLs`:  
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. `Tools → Board → ESP32 Arduino → ESP32 Dev Module`
4. Install library: **ESP32Servo** via Library Manager (needed for Phase 2)
5. Set **Upload Speed:** `115200`, **Port:** your COM/USB port
6. Click **Upload**

---

## Wiring

See [docs/wiring.md](docs/wiring.md) for the full pin reference table.

---

*Good luck to all teams! — MINES MAKERS, ENSMR Rabat*
