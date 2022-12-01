# Killer Boxes

Survival game made in C++ for Arduino Uno.

## About

This game was made as a hobby project to test our programming and circuitry skills, and to help us learn a little bit about Arduino GPIO in C/C++.

## Setup

1. Wire up a joystick module, two buttons, and an LCD 1602 module to the arduino board as follows:
    - Joystick:
      - VRx -> A0
      - VRy -> A1
      - SW -> GPIO 2

    - Left button: GPIO 8

    - Right button: GPIO 7

    - 1602 LCD module:
      - A4 and A5 (Standard Wiring)

2. Install [this lcd library](https://github.com/johnrickman/LiquidCrystal_I2C) into the Arduino IDE.

3. Compile and run!

## Gameplay

- Press the Joystick button to start the game

- Control player movement with the joystick

- Shoot left and right using the aforementioned left and right buttons

- Press joystick button to use "burst" ability (only 3 uses per game)
