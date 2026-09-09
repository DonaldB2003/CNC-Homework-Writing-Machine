# 2D CNC Writing Machine (GRBL + Arduino Uno)

A DIY 2D CNC writing/plotting machine built on an Arduino Uno + CNC Shield running GRBL, with pen up/down control via an SG90 servo.

![Finished machine](photos/machine-overview.jpg)

## Overview

- **Controller:** Arduino Uno + CNC Shield
- **Firmware:** GRBL 0.9j (CoreXY-patched) for motion, `robottini/grbl-servo` fork for pen-lift servo control
- **Kinematics:** CoreXY
- **Pen actuator:** SG90 micro servo
- **G-code source:** Inkscape + Gcodetools extension

## Table of Contents

- [Hardware](#hardware)
- [The Diagonal Movement Problem](#the-diagonal-movement-problem)
- [Firmware Setup](#firmware-setup)
- [Servo Pen-Lift Setup](#servo-pen-lift-setup)
- [G-code Scaling Fix](#g-code-scaling-fix)
- [Speed Tuning](#speed-tuning)
- [Final Settings](#final-settings)
- [Gallery](#gallery)

## Hardware

| Component | Notes |
|---|---|
| Arduino Uno | Main controller |
| CNC Shield v3 | Stepper driver breakout |
| 2x Stepper motors | CoreXY belt-driven X/Y |
| SG90 servo | Pen up/down actuator |
| Second Arduino (Nano/Uno) | Dedicated servo driver (see below) |

![Wiring overview](photos/wiring.jpg)

## The Diagonal Movement Problem

Initially, commanding a pure X or Y move caused diagonal pen movement. Root cause: this is a **CoreXY** machine, but stock GRBL was compiled with CoreXY kinematics disabled.

**Fix:** in `firmware/config.h`, uncomment:
```c
#define COREXY
```

> Note: if you edit `config.h` and re-upload but nothing changes, check your Arduino IDE's verbose compile log for `"Using previously compiled file"` — legacy-format libraries like GRBL don't always trigger a recompile from a header-only change. Forcing a trivial edit to each `.c` file (e.g. an extra blank line) resolves it.

## Firmware Setup

1. Download GRBL from [github.com/gnea/grbl](https://github.com/gnea/grbl)
2. Replace `config.h` with the version in [`/firmware`](firmware/config.h) (CoreXY enabled)
3. Copy the `grbl` folder into your Arduino `libraries` directory
4. Open `grbl/examples/grblUpload/grblUpload.ino` in Arduino IDE, select Board: Uno, and upload

## Servo Pen-Lift Setup

GRBL has no native servo support — spindle PWM (`M3 Sxxx`) runs at the wrong frequency for a standard hobby servo (SG90 expects 50Hz pulses; GRBL's Timer2 PWM runs at ~1kHz), so driving the servo directly off the spindle pin doesn't work reliably.

**Solution used:** [`robottini/grbl-servo`](https://github.com/robottini/grbl-servo) — a GRBL fork with native RC-servo pulse generation on the spindle pin.

```
M3 S60   ; pen up
M3 S0    ; pen down
```

![Servo wiring](photos/servo-wiring.jpg)

Alternative approach (not used in the final build, but included for reference): a second Arduino running [`arduino/servo_trigger.ino`](arduino/servo_trigger.ino), triggered by GRBL's coolant pin (`M8`/`M9`), using the standard `Servo` library for a proper 50Hz signal.

## G-code Scaling Fix

Drawings came out ~6.7% larger than designed, despite correct `$100`/`$101` (steps/mm) calibration confirmed by direct ruler measurement.

**Root cause:** Inkscape 0.92 uses 96 DPI internally; the Gcodetools extension's math assumes the older 90 DPI standard. This produces a consistent `96/90 ≈ 1.0667×` oversizing on every export, regardless of machine calibration.

**Fix:** scale the design by `93.75%` (`90/96`) before generating G-code, or post-process the exported `.gcode` file by multiplying every X/Y coordinate by `0.9375`. See [`gcode/`](gcode/) for a before/after example.

## Speed Tuning

Two separate settings control drawing speed:

| Setting | What it does |
|---|---|
| `F` value in G-code (set via the pen-plot Inkscape extension) | Requested speed |
| `$110`/`$111` (GRBL max rate) | Hard ceiling — GRBL never exceeds this regardless of requested `F` |
| `$120`/`$121` (GRBL acceleration) | How fast the machine can ramp to speed — dominant factor for short strokes like text |

For text/detailed drawings made of many short strokes, **acceleration matters more than max rate** — the machine rarely reaches top speed on short segments, so raising acceleration has a bigger real-world impact than raising max rate alone.

## Final Settings

```
$0 = 10       (step pulse, usec)
$3 = 3        (dir port invert mask)
$100 = 250.000 (x, step/mm)   ; calibrated via ruler test — adjust for your build
$101 = 250.000 (y, step/mm)   ; calibrated via ruler test — adjust for your build
$110 = 1500.000 (x max rate, mm/min)
$111 = 1500.000 (y max rate, mm/min)
$120 = 300.000  (x accel, mm/sec^2)
$121 = 300.000  (y accel, mm/sec^2)
```

*(Fill in your actual final values here.)*

## Gallery

| | |
|---|---|
| ![Sample text output](photos/sample-text.jpg) | ![Sample drawing output](photos/sample-drawing.jpg) |

## Credits

- [GRBL](https://github.com/gnea/grbl)
- [robottini/grbl-servo](https://github.com/robottini/grbl-servo)
- [Gcodetools for Inkscape](https://inkscape.org)
