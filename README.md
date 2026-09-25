# Arduino LED Games Console

An interactive Arduino project featuring multiple LED lighting modes and two built-in games using three buttons and three LEDs.

## Features

### Standard Modes

#### Green Button - All LEDs On
Turns on all three LEDs and leaves them on.

#### Blue Button - Random Flashing
Starts a random light show that continuously illuminates one or two LEDs in changing patterns.

#### Red Button - Sequence Mode
Starts a repeating LED sequence:

Blue → Green → Red → Repeat

---

## Simon Says Game

### Start Simon Says

1. Press the Blue button to enter Random Flashing mode.
2. Press the Blue button again to start Simon Says.

### How It Works

- The system displays a sequence of colors.
- Repeat the sequence using the matching buttons.
- Each successful round adds another color.
- Maximum sequence length is 20 steps.
- Red is never selected twice in a row.
- A wrong input ends the game.

### Controls

- Blue Button = Blue LED
- Green Button = Green LED
- Red Button = Red LED

### Game Flow

1. Intro flash animation
2. Sequence display
3. Player input
4. Sequence grows after a successful round
5. Failure triggers game-over animation
6. Automatic restart after 3 seconds

---

## Whack-a-Mole Game

### Start Whack-a-Mole

1. Press the Green button to turn on all LEDs.
2. Press the Green button again to start Whack-a-Mole.

### How It Works

- One LED lights up.
- Press the matching button before time expires.
- Each correct hit increases score.
- The game becomes faster after every successful hit.
- An incorrect button press or timeout ends the game.

### Difficulty Progression

- Starting reaction time: 2000 ms
- Time reduction per hit: 150 ms
- Minimum reaction time: 300 ms

### Game Flow

1. Intro flash animation
2. Random LED appears
3. Player presses matching button
4. Speed increases after every successful hit
5. Failure triggers game-over animation
6. Automatic restart after 3 seconds

---

## Universal Reset

A full system reset can be triggered from any mode.

### Reset Action

Double-press the Red button within 200 milliseconds.

### Reset Effects

- Flashes all LEDs three times
- Clears Simon Says progress
- Clears Whack-a-Mole progress
- Resets timers and game state
- Returns the system to OFF mode

---

## Hardware Requirements

### Components

- Arduino Uno (or compatible board)
- 3 LEDs
  - Blue
  - Green
  - Red
- 3 Push Buttons
- 3 × 220Ω resistors (LED current limiting)
- 3 × 10kΩ resistors (button pull-downs)
- Breadboard
- Jumper wires

---

## Pin Configuration

### Buttons

| Button | Arduino Pin |
|----------|----------|
| Blue | 2 |
| Green | 4 |
| Red | 6 |

### LEDs

| LED | Arduino Pin |
|----------|----------|
| Blue | 3 |
| Green | 5 |
| Red | 7 |

---

## Software Design

The project is built using a finite state machine (FSM) architecture and non-blocking timing with `millis()`.

### Main Modes

```cpp
OFF
ALL_ON
RANDOM_FLASHING
SEQUENCE

SIMON_INTRO
SIMON_SHOW
SIMON_INPUT
SIMON_ROUND_PAUSE
SIMON_END_FLASH
SIMON_RESTART_WAIT

WHACK_INTRO
WHACK_ACTIVE
WHACK_HIT_PAUSE
WHACK_END_FLASH
WHACK_RESTART_WAIT

RESET_FLASH
```

This structure allows the Arduino to remain responsive without using `delay()`.

---

## Learning Concepts

This project demonstrates:

- Arduino digital inputs and outputs
- Button debouncing
- Finite state machines
- Non-blocking timing with `millis()`
- Random number generation
- Embedded game development
- Event-driven programming

---

## Future Improvements

Possible enhancements include:

- LCD or OLED score display
- Sound effects using a buzzer
- EEPROM high-score storage
- Additional game modes
- Difficulty settings
- Multiplayer support

---

## Author

Arduino LED Games Console

A demonstration project combining LED effects, state-machine programming, and interactive games using only three buttons and three LEDs.