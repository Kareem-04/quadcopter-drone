# 🚁 DIY Quadcopter Drone

A fully custom-built quadcopter featuring an **ESP32-based flight controller** running ESP-FC firmware, a **custom ESP32 remote controller**, and wireless communication over **ESP-NOW** — all configurable through the Betaflight Configurator.

> 📚 Course project — Mechatronics Engineering  
> 🏫 Egypt Japan University of Science and Technology (E-JUST)

---

## 📋 Overview

This project is a complete DIY drone built from scratch — from hardware assembly and custom PCB schematics to firmware flashing and PID tuning. The flight controller runs [ESP-FC](https://github.com/rtlopez/esp-fc), an open-source ESP32-based firmware fully compatible with the Betaflight Configurator (v10.10), giving access to familiar PID tuning, motor configuration, and blackbox logging tools.

The remote controller is a custom ESP32 transmitter that reads four joystick axes and two auxiliary switches, applies deadzone filtering, and sends control packets wirelessly to the drone at **50 Hz** using the **ESP-NOW** protocol — chosen for its low latency and direct peer-to-peer communication without needing a WiFi router.

---

## ✨ Features

- 🎮 **Custom 6-channel remote controller** — throttle, roll, pitch, yaw + 2 aux switches
- 📡 **ESP-NOW wireless link** — low-latency, peer-to-peer, no router required
- 🧠 **ESP-FC flight controller** — open-source, ESP32-based, Betaflight-compatible
- 🔧 **Betaflight Configurator support** — full PID tuning, motor setup, and sensor calibration via GUI
- 📐 **MPU6050 IMU** — 6-axis gyro + accelerometer for attitude estimation
- ⚡ **DShot / PWM ESC support** — compatible with a wide range of ESC protocols
- 🛡️ **Deadzone filtering** — prevents stick drift on roll, pitch, and yaw axes
- 📊 **Blackbox logging** — flight data recording for post-flight analysis
- ✈️ **Multiple flight modes** — ACRO, ANGLE, and AIRMODE
- 🔴 **Failsafe mode** — auto-disarm on signal loss

---

## 🏗️ System Architecture

```
  [Remote Controller]                   [Quadcopter]
  ┌──────────────────────┐             ┌──────────────────────────────┐
  │  ESP32 Transmitter   │             │     ESP32 Flight Controller  │
  │                      │  ESP-NOW    │   (ESP-FC Firmware)          │
  │  Joystick ADC (×4)  │────────────►│                              │
  │  Aux Switches (×2)  │  50 Hz      │   MPU6050 (I2C)              │
  │  Deadzone Filter    │  6 channels │   ├─ Gyro (500°/s)           │
  │  map() → 1000-2000  │             │   └─ Accel                   │
  └──────────────────────┘             │                              │
                                        │   PID Controller (up to 4kHz)│
                                        │   └─ Roll / Pitch / Yaw      │
                                        │                              │
                                        │   4× ESC outputs             │
                                        │   └─ PWM / DShot → Motors    │
                                        └──────────────────────────────┘
                                                    │
                                          ┌─────────┴─────────┐
                                          │  Betaflight        │
                                          │  Configurator      │
                                          │  (USB / WiFi)      │
                                          └───────────────────┘
```

---

## 🧰 Hardware Components

| Component | Details | Purpose |
|-----------|---------|---------|
| Flight Controller MCU | ESP32-DEVKIT-V1 | Main flight computer |
| Remote Controller MCU | ESP32-DEVKIT-V1 | Transmitter |
| IMU | MPU6050 (I2C) | 6-axis gyro + accelerometer |
| ESCs | 4× brushless ESC | Motor speed control |
| Motors | 4× brushless DC motors | Propulsion |
| Frame | Custom (SolidWorks CAD) | Quadcopter X-frame |
| Joysticks | 2× analog joystick modules | Throttle/Yaw + Roll/Pitch |
| Aux Switches | 2× DIP toggle switches | Flight mode, arm/disarm |
| Voltage Regulator | 3.3V LDO | Power for ESP32 logic |
| Battery | LiPo | Main power source |

---

## 📐 Pin Configuration

### Remote Controller (ESP32 Transmitter)

| ESP32 Pin | Connected To | Channel |
|-----------|-------------|---------|
| GPIO34 (ADC) | Joystick 1 — Vertical | Throttle |
| GPIO33 (ADC) | Joystick 1 — Horizontal | Roll |
| GPIO32 (ADC) | Joystick 2 — Vertical | Pitch |
| GPIO35 (ADC) | Joystick 2 — Horizontal | Yaw |
| GPIO4 (D4) | Toggle Switch 1 | AUX1 (arm/disarm) |
| GPIO5 (D5) | Toggle Switch 2 | AUX2 (flight mode) |

### Flight Controller (ESP32)

| ESP32 Pin | Connected To | Function |
|-----------|-------------|---------|
| GPIO21 (SDA) | MPU6050 SDA | I2C data |
| GPIO22 (SCL) | MPU6050 SCL | I2C clock |
| GPIO25 | ESC Motor 1 | PWM/DShot output |
| GPIO26 | ESC Motor 2 | PWM/DShot output |
| GPIO27 | ESC Motor 3 | PWM/DShot output |
| GPIO14 | ESC Motor 4 | PWM/DShot output |

---

## 📡 ESP-NOW Communication Protocol

The remote controller and flight controller communicate using **ESP-NOW** — a connectionless Wi-Fi protocol by Espressif that allows direct ESP-to-ESP data transfer without a router.

### Data Packet (6 channels, sent at 50 Hz)

```cpp
typedef struct struct_message {
    int roll;      // 1000–2000 µs
    int pitch;     // 1000–2000 µs
    int throttle;  // 1000–2000 µs
    int yaw;       // 1000–2000 µs
    int aux1;      // 0 or 1 (toggle switch)
    int aux2;      // 0 or 1 (toggle switch)
} struct_message;
```

### Channel Mapping

```
ADC raw (0–4095)
    → map() → PWM range (1000–2000 µs)
    → Deadzone filter (snap to 1500 if within threshold)
    → esp_now_send() every 20 ms
```

### Deadzone Settings

| Channel | Deadzone (µs from center) | Reason |
|---------|--------------------------|--------|
| Roll | ±40 | Prevents drift on level flight |
| Pitch | ±40 | Prevents drift on level flight |
| Yaw | ±100 | Wider zone — yaw axis more sensitive to noise |
| Throttle | none | Full range always needed |

---

## ✈️ Flight Controller — ESP-FC Firmware

The drone runs [**ESP-FC**](https://github.com/rtlopez/esp-fc), an open-source flight controller firmware for ESP32 that is fully compatible with Betaflight Configurator v10.10. This allows using the familiar Betaflight GUI for all configuration and tuning without needing to compile Betaflight for ESP32.

### Key Capabilities

| Feature | Details |
|---------|---------|
| Gyro loop rate | Up to 4 kHz (with SPI gyro) |
| Flight modes | ACRO, ANGLE, AIRMODE |
| ESC protocols | PWM, Oneshot125/42, Multishot, DShot150/300/600 |
| Receiver input | PPM, SBUS, IBUS, CRSF, ESP-NOW (built-in) |
| Sensors | MPU6050, MPU9250, ICM20602, BMI160 |
| Blackbox | OpenLog / Flash recording |
| Filters | LPF, Dynamic Notch, dTerm, RPM filter |
| Configurator | Betaflight Configurator v10.10 |

### Flashing the Firmware

1. Download the latest firmware from the [ESP-FC Releases page](https://github.com/rtlopez/esp-fc/releases)
2. Open [ESP Tool Web Flasher](https://espressif.github.io/esptool-js/)
3. Connect your ESP32 via USB and click **Connect**
4. Add the firmware `.bin` file and set Flash Address to `0x00`
5. Click **Program**, then power-cycle the board

### Configuration via Betaflight Configurator

1. Install [Betaflight Configurator v10.10](https://github.com/betaflight/betaflight-configurator/releases)
2. Connect the flight controller via USB
3. Configure pin assignments via the **CLI tab** (see [ESP-FC CLI docs](https://github.com/rtlopez/esp-fc/blob/master/docs/cli.md))
4. Set up motor order, ESC protocol, PID gains, and receiver type
5. Test motors without propellers before first flight

---

## 💻 Software & Tools

| Tool | Purpose |
|------|---------|
| [ESP-FC Firmware](https://github.com/rtlopez/esp-fc) | Flight controller firmware |
| [Betaflight Configurator v10.10](https://github.com/betaflight/betaflight-configurator/releases) | GUI for PID tuning, motor config, calibration |
| [Betaflight Blackbox Viewer](https://blackbox.betaflight.com/) | Post-flight log analysis |
| [ESP Tool Web Flasher](https://espressif.github.io/esptool-js/) | Firmware flashing via browser |
| PlatformIO (VS Code) | Remote controller firmware development |
| SolidWorks | Frame CAD design |

---

## 📁 Repository Structure

```
quadcopter-drone/
├── README.md
├── remote_controller/
│   └── transmitter/
│       └── transmitter.ino         ← ESP32 transmitter firmware (ESP-NOW)
├── schematics/
│   ├── flight_controller.pdf       ← FC wiring: ESP32 + MPU6050 + 4× ESC
│   └── remote_controller.pdf       ← TX wiring: ESP32 + joysticks + switches
├── CAD/
│   └── (SolidWorks assembly files) ← Frame 3D models
└── docs/
    └── (videos and references)
```

---

## ⚠️ Notes

- The receiver MAC address in `transmitter.ino` (`broadcastAddress[]`) must be replaced with your actual flight controller ESP32's MAC address before flashing.
- Deadzone values (`ROLL_DEADZONE`, `PITCH_DEADZONE`, `YAW_DEADZONE`) may need adjustment depending on your joystick module's noise characteristics.
- Not all Betaflight Configurator features are functional in ESP-FC — if a setting reverts after saving, it is not supported by the firmware.
- Always test motor direction and order through the Configurator **without propellers** before the first flight.
- ESP-FC is not the same as Betaflight — it mimics the Betaflight 4.2 interface but has its own codebase and limitations.

---

## 🔮 Future Improvements

- Add **barometer** (BMP280) for altitude hold mode
- Implement **GPS navigation** module
- Add **FPV camera** and video transmitter
- Improve frame design with **vibration dampening** mounts for the IMU
- Migrate to a dedicated **F4/F7 flight controller** for higher loop rates

---

## 👥 Team

**Kareem Shaban Eid** — Mechatronics Engineering Student, E-JUST  
[LinkedIn](https://linkedin.com/in/kareem-04-soliman) · [GitHub](https://github.com/Kareem-04)  

**Mahmoud Alaa** — Mechatronics Engineering Student, E-JUST  
**Mariam Nasr** — Mechatronics Engineering Student, E-JUST  
**Al zahraa Khattab** — Mechatronics Engineering Student, E-JUST  
**Aisha Mostafa** — Mechatronics Engineering Student, E-JUST  
**Malak Ashraf** — Mechatronics Engineering Student, E-JUST  

*Supervised by Prof. Mohamed Alkalla*
