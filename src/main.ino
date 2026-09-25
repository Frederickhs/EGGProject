#include <Arduino.h>

// Button input pins
const int blueButton = 2;
const int greenButton = 4;
const int redButton = 6;

// LED output pins
const int blueLED = 3;
const int greenLED = 5;
const int redLED = 7;

// Button debounce timing
const unsigned long debounceDelay = 50;

// Red double-press reset timing
const unsigned long redDoublePressWindow = 200;  // Red must be pressed twice within 200 ms
const unsigned long resetFlashToggleTime = 200;  // Speed of reset flashes
const int resetFlashToggleCount = 6;             // 3 flashes = 3 ON + 3 OFF

unsigned long lastRedPressTime = 0;
int resetFlashToggleCounter = 0;
unsigned long lastResetFlashTime = 0;

// Timing values
const unsigned long flashOnTime = 500;
const unsigned long flashOffTime = 500;

const unsigned long randomFlashOnTime = 100;
const unsigned long randomFlashOffTime = 5;

const unsigned long sequenceOnTime = 500;
const unsigned long sequenceOffTime = 10;

// Shared game intro timing
const unsigned long gameIntroToggleTime = 167;   // 12 toggles is about 2 seconds
const int gameIntroToggleCount = 12;             // 6 flashes = 6 ON + 6 OFF

// Shared game loss/restart timing
const unsigned long gameEndFlashToggleTime = 167; // 6 toggles is about 1 second
const int gameEndFlashToggleCount = 6;            // 3 flashes = 3 ON + 3 OFF
const unsigned long gameRestartWaitTime = 3000;   // Wait 3 seconds before restarting game

int gameEndFlashToggleCounter = 0;
unsigned long lastGameEndFlashTime = 0;
unsigned long gameRestartWaitStartTime = 0;

// Simon Says timing
const unsigned long simonShowOnTime = 400;
const unsigned long simonShowOffTime = 150;
const unsigned long simonRoundPauseTime = 150;
const unsigned long simonInputFeedbackTime = 1500; // 1500 microseconds = 1.5 milliseconds

// Simon Says sequence settings
const int maxSimonSequence = 20;
int simonSequence[maxSimonSequence];
int simonSequenceLength = 0;
int simonShowIndex = 0;
int simonInputIndex = 0;

bool simonShowLEDOn = false;
unsigned long lastSimonTime = 0;

bool simonInputFeedbackActive = false;
unsigned long simonInputFeedbackStartTime = 0;

int simonIntroToggleCounter = 0;

// Whack-a-Mole timing and settings
// No fixed round limit. Game continues until the player misses or presses wrong.
const unsigned long whackStartOnTime = 2000;      // First mole stays on for 2 seconds
const unsigned long whackSpeedStep = 150;         // Each hit reduces time by 150 ms
const unsigned long whackMinOnTime = 300;         // Fastest possible mole time
const unsigned long whackHitPauseTime = 150;

int whackIntroToggleCounter = 0;

int whackScore = 0;
int whackCurrentColor = 0;
int whackLastColor = -1;                          // Tracks previous Whack-a-Mole color
unsigned long whackCurrentOnTime = 0;
unsigned long lastWhackTime = 0;

// Project modes
enum Mode {
  OFF,
  ALL_ON,
  FLASHING,
  RANDOM_FLASHING,
  SEQUENCE,

  SIMON_INTRO,
  SIMON_SHOW,
  SIMON_INPUT,
  SIMON_ROUND_PAUSE,
  SIMON_END_FLASH,
  SIMON_RESTART_WAIT,

  WHACK_INTRO,
  WHACK_ACTIVE,
  WHACK_HIT_PAUSE,
  WHACK_END_FLASH,
  WHACK_RESTART_WAIT,

  RESET_FLASH
};

Mode currentMode = OFF;

// Button state tracking
bool lastBlueReading = LOW;
bool lastGreenReading = LOW;
bool lastRedReading = LOW;

bool stableBlueState = LOW;
bool stableGreenState = LOW;
bool stableRedState = LOW;

unsigned long lastBlueDebounceTime = 0;
unsigned long lastGreenDebounceTime = 0;
unsigned long lastRedDebounceTime = 0;

// Flashing mode variables
bool flashLEDState = LOW;
unsigned long lastFlashTime = 0;

// Random flashing mode variables
bool randomFlashLEDState = LOW;
unsigned long lastRandomFlashTime = 0;

// Sequence mode variables
int sequenceStep = 0;
unsigned long lastSequenceTime = 0;

void setup() {
  // Buttons use external 10k pulldown resistors.
  pinMode(blueButton, INPUT);
  pinMode(greenButton, INPUT);
  pinMode(redButton, INPUT);

  // LED outputs.
  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Seed the random number generator.
  // Use an unused analog pin if A0 is available.
  randomSeed(analogRead(A0));

  allLEDsOff();
}

void loop() {
  checkButtons();

  if (currentMode == OFF) {
    allLEDsOff();
  }
  else if (currentMode == ALL_ON) {
    allLEDsOn();
  }
  else if (currentMode == FLASHING) {
    runFlashingMode();
  }
  else if (currentMode == RANDOM_FLASHING) {
    runRandomFlashingMode();
  }
  else if (currentMode == SEQUENCE) {
    runSequenceMode();
  }

  // Simon Says modes
  else if (currentMode == SIMON_INTRO) {
    runSimonIntro();
  }
  else if (currentMode == SIMON_SHOW) {
    runSimonShow();
  }
  else if (currentMode == SIMON_INPUT) {
    runSimonInputFeedback();
  }
  else if (currentMode == SIMON_ROUND_PAUSE) {
    runSimonRoundPause();
  }
  else if (currentMode == SIMON_END_FLASH) {
    runSimonEndFlash();
  }
  else if (currentMode == SIMON_RESTART_WAIT) {
    runSimonRestartWait();
  }

  // Whack-a-Mole modes
  else if (currentMode == WHACK_INTRO) {
    runWhackIntro();
  }
  else if (currentMode == WHACK_ACTIVE) {
    runWhackActive();
  }
  else if (currentMode == WHACK_HIT_PAUSE) {
    runWhackHitPause();
  }
  else if (currentMode == WHACK_END_FLASH) {
    runWhackEndFlash();
  }
  else if (currentMode == WHACK_RESTART_WAIT) {
    runWhackRestartWait();
  }

  // Reset flash mode
  else if (currentMode == RESET_FLASH) {
    runResetFlash();
  }
}

// ---------------- BUTTON LOGIC ----------------

void checkButtons() {
  bool blueReading = digitalRead(blueButton);
  bool greenReading = digitalRead(greenButton);
  bool redReading = digitalRead(redButton);

  // Blue button debounce
  if (blueReading != lastBlueReading) {
    lastBlueDebounceTime = millis();
  }

  if ((millis() - lastBlueDebounceTime) > debounceDelay) {
    if (blueReading != stableBlueState) {
      stableBlueState = blueReading;

      if (stableBlueState == HIGH) {
        handleBluePress();
      }
    }
  }

  lastBlueReading = blueReading;

  // Green button debounce
  if (greenReading != lastGreenReading) {
    lastGreenDebounceTime = millis();
  }

  if ((millis() - lastGreenDebounceTime) > debounceDelay) {
    if (greenReading != stableGreenState) {
      stableGreenState = greenReading;

      if (stableGreenState == HIGH) {
        handleGreenPress();
      }
    }
  }

  lastGreenReading = greenReading;

  // Red button debounce
  if (redReading != lastRedReading) {
    lastRedDebounceTime = millis();
  }

  if ((millis() - lastRedDebounceTime) > debounceDelay) {
    if (redReading != stableRedState) {
      stableRedState = redReading;

      if (stableRedState == HIGH) {
        handleRedPress();
      }
    }
  }

  lastRedReading = redReading;
}

// ---------------- BUTTON ACTIONS ----------------

void handleBluePress() {
  if (currentMode == RESET_FLASH) {
    return;
  }

  // Simon Says input
  if (currentMode == SIMON_INPUT) {
    handleSimonInput(0); // Blue button = blue LED
    return;
  }

  // Whack-a-Mole input
  if (currentMode == WHACK_ACTIVE) {
    handleWhackInput(0); // Blue button = blue LED
    return;
  }

  // Ignore normal button actions while games are running animations.
  if (isSimonMode() || isWhackMode()) {
    return;
  }

  // If random flashing is already happening,
  // pressing blue again starts Simon Says.
  if (currentMode == RANDOM_FLASHING) {
    startSimonGame();
    return;
  }

  // Otherwise, blue starts random flashing mode.
  currentMode = RANDOM_FLASHING;

  randomFlashLEDState = HIGH;
  lastRandomFlashTime = millis();
  turnRandomOneOrTwoLEDsOn();
}

void handleGreenPress() {
  if (currentMode == RESET_FLASH) {
    return;
  }

  // Simon Says input
  if (currentMode == SIMON_INPUT) {
    handleSimonInput(1); // Green button = green LED
    return;
  }

  // Whack-a-Mole input
  if (currentMode == WHACK_ACTIVE) {
    handleWhackInput(1); // Green button = green LED
    return;
  }

  // Ignore normal button actions while games are running animations.
  if (isSimonMode() || isWhackMode()) {
    return;
  }

  // UPDATED:
  // If all LEDs are already on, pressing green again
  // starts Whack-a-Mole.
  if (currentMode == ALL_ON) {
    startWhackGame();
    return;
  }

  // UPDATED:
  // From OFF or another normal lighting mode,
  // green turns on all three LEDs and leaves them on.
  currentMode = ALL_ON;
  allLEDsOn();
}

void handleRedPress() {
  unsigned long currentTime = millis();

  if (currentMode == RESET_FLASH) {
    return;
  }

  // Red double-press reset check.
  // Works from any mode, including game modes and flashing modes.
  if (lastRedPressTime != 0 &&
      currentTime - lastRedPressTime <= redDoublePressWindow) {
    lastRedPressTime = 0;
    startResetFlash();
    return;
  }

  lastRedPressTime = currentTime;

  // Simon Says input
  if (currentMode == SIMON_INPUT) {
    handleSimonInput(2); // Red button = red LED
    return;
  }

  // Whack-a-Mole input
  if (currentMode == WHACK_ACTIVE) {
    handleWhackInput(2); // Red button = red LED
    return;
  }

  // Ignore normal red button action while games are running animations.
  if (isSimonMode() || isWhackMode()) {
    return;
  }

  // Red starts sequence mode.
  currentMode = SEQUENCE;

  sequenceStep = 0;
  lastSequenceTime = millis();

  allLEDsOff();
  digitalWrite(blueLED, HIGH);
}

// ---------------- ORIGINAL LED MODES ----------------

void runFlashingMode() {
  unsigned long currentTime = millis();

  if (flashLEDState == HIGH) {
    if (currentTime - lastFlashTime >= flashOnTime) {
      flashLEDState = LOW;
      lastFlashTime = currentTime;
      allLEDsOff();
    }
  }
  else {
    if (currentTime - lastFlashTime >= flashOffTime) {
      flashLEDState = HIGH;
      lastFlashTime = currentTime;
      allLEDsOn();
    }
  }
}

void runRandomFlashingMode() {
  unsigned long currentTime = millis();

  if (randomFlashLEDState == HIGH) {
    if (currentTime - lastRandomFlashTime >= randomFlashOnTime) {
      randomFlashLEDState = LOW;
      lastRandomFlashTime = currentTime;
      allLEDsOff();
    }
  }
  else {
    if (currentTime - lastRandomFlashTime >= randomFlashOffTime) {
      randomFlashLEDState = HIGH;
      lastRandomFlashTime = currentTime;
      turnRandomOneOrTwoLEDsOn();
    }
  }
}

void runSequenceMode() {
  unsigned long currentTime = millis();

  switch (sequenceStep) {
    case 0:
      if (currentTime - lastSequenceTime >= sequenceOnTime) {
        allLEDsOff();
        sequenceStep = 1;
        lastSequenceTime = currentTime;
      }
      break;

    case 1:
      if (currentTime - lastSequenceTime >= sequenceOffTime) {
        digitalWrite(greenLED, HIGH);
        sequenceStep = 2;
        lastSequenceTime = currentTime;
      }
      break;

    case 2:
      if (currentTime - lastSequenceTime >= sequenceOnTime) {
        allLEDsOff();
        sequenceStep = 3;
        lastSequenceTime = currentTime;
      }
      break;

    case 3:
      if (currentTime - lastSequenceTime >= sequenceOffTime) {
        digitalWrite(redLED, HIGH);
        sequenceStep = 4;
        lastSequenceTime = currentTime;
      }
      break;

    case 4:
      if (currentTime - lastSequenceTime >= sequenceOnTime) {
        allLEDsOff();
        sequenceStep = 5;
        lastSequenceTime = currentTime;
      }
      break;

    case 5:
      if (currentTime - lastSequenceTime >= sequenceOffTime) {
        digitalWrite(blueLED, HIGH);
        sequenceStep = 0;
        lastSequenceTime = currentTime;
      }
      break;
  }
}

// ---------------- SIMON SAYS MODE ----------------

bool isSimonMode() {
  return currentMode == SIMON_INTRO ||
         currentMode == SIMON_SHOW ||
         currentMode == SIMON_INPUT ||
         currentMode == SIMON_ROUND_PAUSE ||
         currentMode == SIMON_END_FLASH ||
         currentMode == SIMON_RESTART_WAIT;
}

void startSimonGame() {
  currentMode = SIMON_INTRO;

  simonSequenceLength = 0;
  simonShowIndex = 0;
  simonInputIndex = 0;
  simonIntroToggleCounter = 0;
  simonInputFeedbackActive = false;

  lastSimonTime = millis();

  allLEDsOn(); // Start intro flash ON
}

void runSimonIntro() {
  unsigned long currentTime = millis();

  if (currentTime - lastSimonTime >= gameIntroToggleTime) {
    lastSimonTime = currentTime;
    simonIntroToggleCounter++;

    if (simonIntroToggleCounter >= gameIntroToggleCount) {
      allLEDsOff();
      addSimonStep();
      startSimonShow();
    }
    else {
      toggleAllLEDs();
    }
  }
}

void addSimonStep() {
  if (simonSequenceLength < maxSimonSequence) {
    int newColor = random(0, 3); // 0 blue, 1 green, 2 red

    // Do not allow red twice in a row.
    if (simonSequenceLength > 0 &&
        simonSequence[simonSequenceLength - 1] == 2) {
      while (newColor == 2) {
        newColor = random(0, 3);
      }
    }

    simonSequence[simonSequenceLength] = newColor;
    simonSequenceLength++;
  }
}

void startSimonShow() {
  currentMode = SIMON_SHOW;

  simonShowIndex = 0;
  simonShowLEDOn = false;
  simonInputFeedbackActive = false;
  lastSimonTime = millis();

  allLEDsOff();
}

void runSimonShow() {
  unsigned long currentTime = millis();

  if (simonShowIndex >= simonSequenceLength) {
    currentMode = SIMON_INPUT;
    simonInputIndex = 0;
    allLEDsOff();
    return;
  }

  if (simonShowLEDOn == false) {
    if (currentTime - lastSimonTime >= simonShowOffTime) {
      allLEDsOff();
      turnGameLEDOn(simonSequence[simonShowIndex]);
      simonShowLEDOn = true;
      lastSimonTime = currentTime;
    }
  }
  else {
    if (currentTime - lastSimonTime >= simonShowOnTime) {
      allLEDsOff();
      simonShowLEDOn = false;
      simonShowIndex++;
      lastSimonTime = currentTime;
    }
  }
}

void handleSimonInput(int buttonColor) {
  allLEDsOff();
  turnGameLEDOn(buttonColor);

  simonInputFeedbackActive = true;
  simonInputFeedbackStartTime = micros();

  if (buttonColor == simonSequence[simonInputIndex]) {
    simonInputIndex++;

    if (simonInputIndex >= simonSequenceLength) {
      currentMode = SIMON_ROUND_PAUSE;
      lastSimonTime = millis();
    }
  }
  else {
    startSimonEndFlash();
  }
}

void runSimonInputFeedback() {
  if (simonInputFeedbackActive) {
    unsigned long currentTime = micros();

    if (currentTime - simonInputFeedbackStartTime >=
        simonInputFeedbackTime) {
      allLEDsOff();
      simonInputFeedbackActive = false;
    }
  }
}

void runSimonRoundPause() {
  unsigned long currentTime = millis();

  runSimonInputFeedback();

  if (currentTime - lastSimonTime >= simonRoundPauseTime) {
    if (simonSequenceLength < maxSimonSequence) {
      addSimonStep();
      startSimonShow();
    }
    else {
      // If player reaches max length,
      // restart Simon instead of leaving game mode.
      startSimonEndFlash();
    }
  }
}

void startSimonEndFlash() {
  currentMode = SIMON_END_FLASH;

  gameEndFlashToggleCounter = 0;
  lastGameEndFlashTime = millis();
  simonInputFeedbackActive = false;

  allLEDsOn(); // Start game-ended flash ON
}

void runSimonEndFlash() {
  unsigned long currentTime = millis();

  if (currentTime - lastGameEndFlashTime >= gameEndFlashToggleTime) {
    lastGameEndFlashTime = currentTime;
    gameEndFlashToggleCounter++;

    if (gameEndFlashToggleCounter >= gameEndFlashToggleCount) {
      allLEDsOff();
      currentMode = SIMON_RESTART_WAIT;
      gameRestartWaitStartTime = millis();
    }
    else {
      toggleAllLEDs();
    }
  }
}

void runSimonRestartWait() {
  unsigned long currentTime = millis();

  allLEDsOff();

  if (currentTime - gameRestartWaitStartTime >= gameRestartWaitTime) {
    startSimonGame();
  }
}

// ---------------- WHACK-A-MOLE MODE ----------------

bool isWhackMode() {
  return currentMode == WHACK_INTRO ||
         currentMode == WHACK_ACTIVE ||
         currentMode == WHACK_HIT_PAUSE ||
         currentMode == WHACK_END_FLASH ||
         currentMode == WHACK_RESTART_WAIT;
}

void startWhackGame() {
  currentMode = WHACK_INTRO;

  whackScore = 0;
  whackLastColor = -1;
  whackIntroToggleCounter = 0;
  lastWhackTime = millis();

  allLEDsOn(); // Start intro flash ON
}

void runWhackIntro() {
  unsigned long currentTime = millis();

  if (currentTime - lastWhackTime >= gameIntroToggleTime) {
    lastWhackTime = currentTime;
    whackIntroToggleCounter++;

    if (whackIntroToggleCounter >= gameIntroToggleCount) {
      allLEDsOff();
      startWhackRound();
    }
    else {
      toggleAllLEDs();
    }
  }
}

void startWhackRound() {
  currentMode = WHACK_ACTIVE;

  int newColor = random(0, 3); // 0 blue, 1 green, 2 red

  // Do not allow red twice in a row.
  if (whackLastColor == 2) {
    while (newColor == 2) {
      newColor = random(0, 3);
    }
  }

  whackCurrentColor = newColor;
  whackLastColor = whackCurrentColor;

  whackCurrentOnTime = calculateWhackOnTime();

  allLEDsOff();
  turnGameLEDOn(whackCurrentColor);

  lastWhackTime = millis();
}

unsigned long calculateWhackOnTime() {
  unsigned long newTime = whackStartOnTime;

  if ((whackScore * whackSpeedStep) < whackStartOnTime) {
    newTime = whackStartOnTime - (whackScore * whackSpeedStep);
  }

  if (newTime < whackMinOnTime) {
    newTime = whackMinOnTime;
  }

  return newTime;
}

void runWhackActive() {
  unsigned long currentTime = millis();

  // If the player does not press the correct button in time,
  // game over/restart.
  if (currentTime - lastWhackTime >= whackCurrentOnTime) {
    startWhackEndFlash();
  }
}

void handleWhackInput(int buttonColor) {
  if (buttonColor == whackCurrentColor) {
    whackScore++;

    allLEDsOff();

    currentMode = WHACK_HIT_PAUSE;
    lastWhackTime = millis();
  }
  else {
    startWhackEndFlash();
  }
}

void runWhackHitPause() {
  unsigned long currentTime = millis();

  if (currentTime - lastWhackTime >= whackHitPauseTime) {
    startWhackRound();
  }
}

void startWhackEndFlash() {
  currentMode = WHACK_END_FLASH;

  gameEndFlashToggleCounter = 0;
  lastGameEndFlashTime = millis();

  allLEDsOn(); // Start game-ended flash ON
}

void runWhackEndFlash() {
  unsigned long currentTime = millis();

  if (currentTime - lastGameEndFlashTime >= gameEndFlashToggleTime) {
    lastGameEndFlashTime = currentTime;
    gameEndFlashToggleCounter++;

    if (gameEndFlashToggleCounter >= gameEndFlashToggleCount) {
      allLEDsOff();
      currentMode = WHACK_RESTART_WAIT;
      gameRestartWaitStartTime = millis();
    }
    else {
      toggleAllLEDs();
    }
  }
}

void runWhackRestartWait() {
  unsigned long currentTime = millis();

  allLEDsOff();

  if (currentTime - gameRestartWaitStartTime >= gameRestartWaitTime) {
    startWhackGame();
  }
}

// ---------------- RESET MODE ----------------

void startResetFlash() {
  currentMode = RESET_FLASH;

  resetFlashToggleCounter = 0;
  lastResetFlashTime = millis();

  allLEDsOn();
}

void runResetFlash() {
  unsigned long currentTime = millis();

  if (currentTime - lastResetFlashTime >= resetFlashToggleTime) {
    lastResetFlashTime = currentTime;
    resetFlashToggleCounter++;

    if (resetFlashToggleCounter >= resetFlashToggleCount) {
      allLEDsOff();
      resetAllGameVariables();
      currentMode = OFF;
    }
    else {
      toggleAllLEDs();
    }
  }
}

void resetAllGameVariables() {
  // Clear Simon Says state.
  simonSequenceLength = 0;
  simonShowIndex = 0;
  simonInputIndex = 0;
  simonShowLEDOn = false;
  simonIntroToggleCounter = 0;
  simonInputFeedbackActive = false;
  simonInputFeedbackStartTime = 0;

  // Clear Whack-a-Mole state.
  whackScore = 0;
  whackCurrentColor = 0;
  whackLastColor = -1;
  whackCurrentOnTime = 0;
  whackIntroToggleCounter = 0;

  // Clear shared game end/restart state.
  gameEndFlashToggleCounter = 0;
  gameRestartWaitStartTime = 0;

  // Clear standard mode timing/state.
  flashLEDState = LOW;
  randomFlashLEDState = LOW;
  sequenceStep = 0;

  // Clear red double-press memory.
  lastRedPressTime = 0;
}

// ---------------- HELPER FUNCTIONS ----------------

void allLEDsOn() {
  digitalWrite(blueLED, HIGH);
  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, HIGH);
}

void allLEDsOff() {
  digitalWrite(blueLED, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
}

void toggleAllLEDs() {
  digitalWrite(blueLED, !digitalRead(blueLED));
  digitalWrite(greenLED, !digitalRead(greenLED));
  digitalWrite(redLED, !digitalRead(redLED));
}

void turnGameLEDOn(int ledColor) {
  allLEDsOff();

  if (ledColor == 0) {
    digitalWrite(blueLED, HIGH);
  }
  else if (ledColor == 1) {
    digitalWrite(greenLED, HIGH);
  }
  else if (ledColor == 2) {
    digitalWrite(redLED, HIGH);
  }
}

void turnRandomOneOrTwoLEDsOn() {
  allLEDsOff();

  int numberOfLEDs = random(1, 3);

  if (numberOfLEDs == 1) {
    int ledChoice = random(0, 3);

    if (ledChoice == 0) {
      digitalWrite(blueLED, HIGH);
    }
    else if (ledChoice == 1) {
      digitalWrite(greenLED, HIGH);
    }
    else {
      digitalWrite(redLED, HIGH);
    }
  }
  else {
    int pairChoice = random(0, 3);

    if (pairChoice == 0) {
      digitalWrite(blueLED, HIGH);
      digitalWrite(greenLED, HIGH);
    }
    else if (pairChoice == 1) {
      digitalWrite(blueLED, HIGH);
      digitalWrite(redLED, HIGH);
    }
    else {
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, HIGH);
    }
  }
}
