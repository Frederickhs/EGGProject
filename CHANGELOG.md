# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and the project follows Semantic Versioning (SemVer).

---

## [1.0.0] - 2026-09-25

### Added

#### Core Hardware Functionality
- Three-button input system with debounce protection.
- Three-LED output system (Blue, Green, Red).
- Random seed initialization using analog input.
- Non-blocking timing architecture using `millis()` and `micros()`.

#### Standard LED Modes
- OFF mode.
- ALL_ON mode.
- RANDOM_FLASHING mode.
- SEQUENCE mode.
- Random illumination of one or two LEDs during random flashing mode.
- Continuous Blue → Green → Red sequence mode.

#### Simon Says Game
- Simon Says intro animation.
- Random sequence generation.
- Progressive difficulty through expanding sequences.
- Maximum sequence length of 20 steps.
- Player input validation.
- Input feedback lighting.
- Round transition handling.
- Game-over flash animation.
- Automatic game restart after failure.
- Prevention of consecutive Red selections.

#### Whack-a-Mole Game
- Whack-a-Mole intro animation.
- Random target generation.
- Progressive speed increase after successful hits.
- Adjustable reaction window based on score.
- Correct-hit detection.
- Wrong-input detection.
- Timeout failure detection.
- Game-over flash animation.
- Automatic game restart after failure.
- Prevention of consecutive Red targets.

#### Reset System
- Universal double-press Red button reset.
- Global reset availability from all operating modes.
- Reset confirmation flash sequence.
- Complete game-state reset functionality.
- Automatic return to OFF mode after reset.

#### Software Architecture
- Finite State Machine (FSM) design using Mode enumeration.
- Dedicated helper functions for LED control.
- Separate game-state management for Simon Says and Whack-a-Mole.
- Shared animation framework for introductions, failures, and resets.
- Modular button handling architecture.
- Modular game-mode architecture.

### Technical Highlights

- Fully non-blocking implementation.
- No use of `delay()` for game logic.
- Responsive button handling during gameplay.
- Debounced inputs for reliable operation.
- Extensible mode-based architecture suitable for future game additions.

### Initial Release

- First public release of the Arduino LED Games Console project.
- Includes lighting effects, Simon Says, Whack-a-Mole, and universal reset functionality.