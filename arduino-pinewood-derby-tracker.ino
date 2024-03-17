#include "constants.h"
#include "utils.h"

/* Set the current mode */
int mode = MODE_STARTUP;
int state = STATE_STARTUP;

/* Store the output states */
bool statusLed = true;
bool ledL1State = true;
bool ledL2State = true;

int readyStateBlinkCount = 0;

unsigned long raceCount = 0;
unsigned long raceStartTime = 0;
unsigned long elapsedL1Time = 0;
unsigned long elapsedL2Time = 0;
int winner = 0;

unsigned long startupStateNextBlink = 0;
unsigned long readyStateNextBlink = 0;
unsigned long racingStateNextBlink = 0;
unsigned long finishedStateNextBlink = 0;

unsigned long statusLedNextBlink = 0;
unsigned long ledL1NextBlink = 0;
unsigned long ledL2NextBlink = 0;

unsigned long startButtonNextReport = 0;
unsigned long resetButtonNextReport = 0;
unsigned long finishSwitchL1NextReport = 0;
unsigned long finishSwitchL2NextReport = 0;
unsigned long uptimeNextReport = 0;

bool readyStateReported = false;
bool startModeReported = false;
bool raceResultsHeaderReported = false;

void setup() {
  // Initialize Outputs
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(LED_L1, OUTPUT);
  pinMode(LED_L2, OUTPUT);
  updateOutputs();

  // Initialize Inputs
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(FINISH_SWITCH_L1, INPUT_PULLUP);
  pinMode(FINISH_SWITCH_L2, INPUT_PULLUP);


  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // Wait 3 seconds before displaying data to give client time to start console
  Serial.println("Startup in 3...");
  delay(1000);
  Serial.println("2...");
  delay(1000);
  Serial.println("1...");
  delay(1000);
  Serial.println();

  // Print Title Information
  printTitle();

  // Wait for Test Mode
  setupWaitForTestMode();

  // Output if we are entering test mode
  if (mode != MODE_TEST) {
    mode = MODE_RUN;
  } else {
    Serial.println("Entering test mode...");
  }
}


void setupWaitForTestMode() {
  // Wait for test mode
  Serial.println("Hold Reset button for 2 seconds to enter test mode...");
  Serial.println("Waiting 10 seconds to enter test mode...");

  bool lastResetButtonPressed = false;
  int resetCount = 0;
  for (int i = 10; i > 0 && mode != MODE_TEST; i--) {
    switch (i % 3) {
      case 0:
        ledL1State = true;
        ledL2State = false;
        break;

      case 1: 
        ledL1State = true;
        ledL2State = true;
        break;

      case 2:
        ledL1State = false;
        ledL2State = true;
        break;
    }

    Serial.println(String(i) + "...");
    delay(1000);

    if (isResetButtonPressed()) {
      resetCount++;
    } else {
      resetCount = 0;
    }

    if (resetCount >= 2) {
      mode = MODE_TEST;

      // Configure uptime to report at the next even report interval
      uptimeNextReport = ((millis() / TEST_MODE_UPTIME_REPORT_INTERVAL) + 1) * TEST_MODE_UPTIME_REPORT_INTERVAL;
    }

    updateOutputs();
  }
}


void loop() {
  if (mode != MODE_TEST) {
    processRunMode();
  } else {
    processTestMode();
  }

  updateOutputs();
}


void processRunMode() {
  switch (state) {
    case STATE_STARTUP:
      processStartupState();
      break;

    case STATE_READY:
      processReadyState();
      break;

    case STATE_RACING:
      processRacingState();
      break;

    case STATE_FINISHED:
    default:
      processFinishedState();
      break;

  }
}


void processStartupState() {
  unsigned long currentMillis = millis();

  if (!startModeReported) {
    // Alterate the LED stats to start the blinking
    ledL1State = true;
    ledL2State = false;
    startupStateNextBlink = currentMillis + READY_STATE_TRANSITION_BLINK_INTERVAL;

    Serial.println();
    Serial.println("Racers, start your engines!");
    Serial.println("Press Reset button to prepare for first race...");
    Serial.println("");
    startModeReported = true;
  }

  // Alternate L1 and L2 lights on initial startup
  if (currentMillis >= startupStateNextBlink) {
    ledL1State = !ledL1State;
    ledL2State = !ledL2State;
    startupStateNextBlink = currentMillis + READY_STATE_TRANSITION_BLINK_INTERVAL;
  }

  if (isResetButtonPressed()) {
    state = STATE_READY;
    ledL1State = true;
    ledL2State = true;
    readyStateBlinkCount = 0;
    resetButtonNextReport = currentMillis + RUN_MODE_RESET_NEW_LINE_INTERVAL;
    readyStateNextBlink = currentMillis + READY_STATE_TRANSITION_BLINK_INTERVAL;
  }
}


void processReadyState() {
  unsigned long currentMillis = millis();

  if (!raceResultsHeaderReported) {
    Serial.println("");
    Serial.println("Race #, L1 Time, L2 Time, Winning Lane");
    raceResultsHeaderReported = true;
  }

  if (!readyStateReported && currentMillis > readyStateNextBlink) {
    ledL1State = !ledL1State;
    ledL2State = !ledL2State;

    // Count the blink after we transition back to high after the blink delay
    if (ledL1State == true) {
      readyStateBlinkCount++;
    }

    if (readyStateBlinkCount <= READY_STATE_TRANSITION_BLINK_COUNT) {
      readyStateNextBlink = currentMillis + READY_STATE_TRANSITION_BLINK_INTERVAL;
    } else {
      readyStateReported = true;
    }
  }

  // Print a new line periodically if "RESET" is pressed multiple times
  // This will add space to the console output as a hint that "something" happened here
  if (isResetButtonPressed()) {
    if (currentMillis >= resetButtonNextReport) {
      Serial.println();
      resetButtonNextReport = currentMillis + RUN_MODE_RESET_NEW_LINE_INTERVAL;;
    }
  }

  if (isStartButtonPressed()) {
    state = STATE_RACING;
    raceCount++;
    raceStartTime = currentMillis;
    elapsedL1Time = 0;
    elapsedL2Time = 0;
    winner = 0;
    racingStateNextBlink = currentMillis + RACING_BLINK_INTERVAL;
  }
}


void processRacingState() {
  unsigned long currentMillis = millis();

  if (winner == 0 && currentMillis > racingStateNextBlink) {
    ledL1State = !ledL1State;
    ledL2State = !ledL2State;

    racingStateNextBlink = currentMillis + RACING_BLINK_INTERVAL;
  }

  // Check if lane 1 just finished
  if (elapsedL1Time == 0 && isFinishSwitchL1Pressed()) {
    elapsedL1Time = currentMillis - raceStartTime;
  }

  // Check if lane 2 just finished
  if (elapsedL2Time == 0 && isFinishSwitchL2Pressed()) {
    elapsedL2Time = currentMillis - raceStartTime;
  }

  // Check for  winner on every pass
  if (winner == 0) {
    if (elapsedL1Time != 0 && elapsedL2Time == 0) {
      winner = 1;
    } else if (elapsedL2Time != 0 && elapsedL1Time == 0) {
      winner = 2;
    } else if (elapsedL1Time != 0 && elapsedL2Time != 0) {
      if (elapsedL1Time < elapsedL2Time) {
        winner = 1;
      } else if (elapsedL1Time > elapsedL2Time) {
        winner = 2;
      } else {
        // Special case - we have a tie (unlikely)
        winner = 3;
      }
    }
  }

  // If a winner is defined, set the led status (set it every time, it won't hurt)
  if (winner == 1) {
    ledL1State = true;
    ledL2State = false;
  } else if (winner == 2) {
    ledL1State = false;
    ledL2State = true;
  } else if (winner == 3) {
    ledL1State = true;
    ledL2State = true;
  }

  if ((elapsedL1Time != 0 && elapsedL2Time != 0 && winner != 0) || isResetButtonPressed()) {
    reportWinner();

    // If no winner was defined, alternate the blinking order to show "no completion"
    if (winner == 0) {
      ledL1State = true;
      ledL2State = false;
    }

    state = STATE_FINISHED;
    finishedStateNextBlink = currentMillis + FINISHED_STATE_BLINK_INTERVAL;

    // Give at least 1 second before responding to the reset button
    resetButtonNextReport = currentMillis + RUN_MODE_RESET_NEW_LINE_INTERVAL;
  }
}


void processFinishedState() {
  unsigned long currentMillis = millis();

  if (currentMillis > finishedStateNextBlink) {
    // If a winner is defined, set the led status (set it every time, it won't hurt)
    if (winner == 1) {
      ledL1State = !ledL1State;
    } else if (winner == 2) {
      ledL2State = !ledL2State;
    } else {
      ledL1State = !ledL1State;
      ledL2State = !ledL2State;
    }

    finishedStateNextBlink = currentMillis + FINISHED_STATE_BLINK_INTERVAL;
  }

  if (currentMillis > resetButtonNextReport && isResetButtonPressed()) {
    state = STATE_READY;
    ledL1State = true;
    ledL2State = true;
    readyStateReported = false;
    readyStateBlinkCount = 0;
    resetButtonNextReport = currentMillis + RUN_MODE_RESET_NEW_LINE_INTERVAL;
    readyStateNextBlink = currentMillis + READY_STATE_TRANSITION_BLINK_INTERVAL;
  }
}


void processTestMode() {
  unsigned long currentMillis = millis();

  if (isStartButtonPressed()) {
    if (currentMillis >= startButtonNextReport) {
      Serial.println("Start Button pressed");
      startButtonNextReport = currentMillis + TEST_MODE_REPORT_INTERVAL;
    }
  }

  if (isResetButtonPressed()) {
    if (currentMillis >= resetButtonNextReport) {
      Serial.println("Reset Button pressed");
      resetButtonNextReport = currentMillis + TEST_MODE_REPORT_INTERVAL;
    }
  }

  if (isFinishSwitchL1Pressed()) {
    if (currentMillis >= finishSwitchL1NextReport) {
      Serial.println("Finish Switch L1 pressed");
      finishSwitchL1NextReport = currentMillis + TEST_MODE_REPORT_INTERVAL;
    }
    ledL1State = true;
  } else {
    ledL1State = false;
  }

  if (isFinishSwitchL2Pressed()) {
    if (currentMillis >= finishSwitchL2NextReport) {
      Serial.println("Finish Switch L2 pressed");
      finishSwitchL2NextReport = currentMillis + TEST_MODE_REPORT_INTERVAL;
    }
    ledL2State = true;
  } else {
    ledL2State = false;
  }

  // Blink Status LED in Test Mode
  if (currentMillis >= statusLedNextBlink) {
    statusLed = !statusLed;
    statusLedNextBlink = currentMillis + TEST_MODE_STATUS_BLINK_INTERVAL;
  }

  if (currentMillis >= uptimeNextReport) {
    Serial.println(uptime(currentMillis));
    uptimeNextReport = currentMillis + TEST_MODE_UPTIME_REPORT_INTERVAL;

  }
}


void reportWinner() {
  Serial.print(String(raceCount));
  Serial.print(", ");
  Serial.print(elapsedTimeSecs(elapsedL1Time));
  Serial.print(", ");
  Serial.print(elapsedTimeSecs(elapsedL2Time));
  Serial.print(", ");
  if (winner != 0) {
    Serial.print(String(winner));
  } else {
    Serial.print("N/A");
  }
  Serial.println();
}


void updateOutput(int pin, bool ledHigh) {
  if (ledHigh) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}


void updateOutputs() {
  updateOutput(STATUS_LED_PIN, statusLed);
  updateOutput(LED_L1, ledL1State);
  updateOutput(LED_L2, ledL2State);
}


bool isFinishSwitchL1Pressed() {
  return digitalRead(FINISH_SWITCH_L1) == LOW;
}


bool isFinishSwitchL2Pressed() {
  return digitalRead(FINISH_SWITCH_L2) == LOW;
}


bool isResetButtonPressed() {
  return digitalRead(RESET_BUTTON) == LOW;
}


int isStartButtonPressed() {
  return digitalRead(START_BUTTON) == LOW;
}
