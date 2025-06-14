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
unsigned long totalRaceTime = 0;
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

unsigned long maxRaceTime = 10000;

bool readyStateReported = false;

int serialInputs[2] = {-1, -1};

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
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  readSerialInput();
  processInputCommand();

  if (mode != MODE_TEST) {
    processRunMode();
  } else {
    processTestMode();
  }

  updateOutputs();
}

void readSerialInput() {
  if (Serial.available() > 0) {
    serialInputs[0] = serialInputs[1];
    serialInputs[1] = Serial.read();
  }
}

void processInputCommand() {
  bool commandProcessed = false;
  String cmd = String("");
  if (serialInputs[0] >= 0 && serialInputs[1] >= 0) {
    cmd.concat((char)serialInputs[0]);
    cmd.concat((char)serialInputs[1]);
  }

  // Return Version of Firmware and Serial Number
  if (cmd.equals("RV")) {
    printVersionAndSerialNumber();
    commandProcessed = true;

  // Return Serial Number
  } else if (cmd.equals("RS")) {
    printSerialNumber();
    commandProcessed = true;

  // Return Features - All Features Disabled
  } else if (cmd.equals("RF")) {
    // Serial.println("0000 0011");
    Serial.println("0000 0011");
    commandProcessed = true;

  // Read Mode - Shows the current modes set for the timer
  } else if (cmd.equals("RM")) {
    Serial.println("0 000000 0 0 0");
    commandProcessed = true;

  // Returns start switch condition 
  } else if (cmd.equals("RG")) {
    if (isStartButtonPressed()) {
      Serial.println("1");
    } else {
      Serial.println("0");
    }
    commandProcessed = true;

  // Reset Lane - Force results
  } else if (cmd.equals("RL")) {
    // TOOD: Implement
    commandProcessed = true;

  } else if (cmd.equals("MA") || cmd.equals("MB") || cmd.equals("MC") || cmd.equals("MD") || cmd.equals("ME") || cmd.equals("MG")) {
    // TODO: Implement
    commandProcessed = true;

} else if (cmd.equals("MB")) {
    commandProcessed = true;
    
  } else if (cmd.equals("N0")) {
    commandProcessed = true;

  }

  if (commandProcessed) {
    serialInputs[0] = -1;
    serialInputs[1] = -1;

    //Serial.println("Received: " + cmd);
  }
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

  state = STATE_READY;
  ledL1State = true;
  ledL2State = true;
  readyStateBlinkCount = 0;
  resetButtonNextReport = currentMillis + RUN_MODE_RESET_NEW_LINE_INTERVAL;
  readyStateNextBlink = currentMillis + READY_STATE_TRANSITION_BLINK_INTERVAL;
}


void processReadyState() {
  unsigned long currentMillis = millis();

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
  unsigned long elapsedRaceTime = currentMillis - raceStartTime;

  if (winner == 0 && currentMillis > racingStateNextBlink) {
    ledL1State = !ledL1State;
    ledL2State = !ledL2State;

    racingStateNextBlink = currentMillis + RACING_BLINK_INTERVAL;
  }

  // Check if lane 1 just finished
  if (elapsedL1Time == 0 && isFinishSwitchL1Pressed()) {
    elapsedL1Time = elapsedRaceTime;
  }

  // Check if lane 2 just finished
  if (elapsedL2Time == 0 && isFinishSwitchL2Pressed()) {
    elapsedL2Time = elapsedRaceTime;
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

  if ((elapsedL1Time != 0 && elapsedL2Time != 0 && winner != 0) || elapsedRaceTime >= maxRaceTime || isResetButtonPressed()) {
    totalRaceTime = elapsedRaceTime;
    reportResults();

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

void printVersionAndSerialNumber() {
  Serial.println("MICRO WIZARD - Emulator - 2025");
  Serial.print("K1 Version 1.10E Serial Number <");
  Serial.print(MOCK_SERIAL_NUMBER);
  Serial.println(">");
}

void printSerialNumber() {
  Serial.println(MOCK_SERIAL_NUMBER);
}

void reportFlags() {
  // This command will return 8 binary bits like 0011 0111.
  // A 1 means the option is enabled:
  // "1111 1111" all feature bits set
  // "0000 0000" all feature bits clear 

  //   8th bit Sequence of Finish K3 only
  Serial.print(0);
  //   7th bit Countdown Clock
  Serial.print(0);
  //   6th bit Laser Reset from computer
  Serial.print(0);
  //   5th bit Force End the race and send results
  Serial.print(0);
  //   4th bit Eliminator mode
  Serial.print(0);
  //   3rd bit Reverse Lanes
  Serial.print(0);
  //   2nd bit Mask Lanes
  Serial.print(0);
  //   1st bit Serial race data option
  Serial.println(1);
}

void reportMode() {
  // Shows the current modes set for the timer:
  // ex 1: "0 000000 0 0 0"
  // ex 1: "6 000011 0 0 1"

  // Number of lanes used in reverse order mode - 6
  Serial.print(0);
  Serial.print(' ');
  // Lanes E and F are masked - 000011
  Serial.print("000000");
  Serial.print(' ');
  // Lanes are not reversed  - 0
  Serial.print(0);
  // Not in eliminator mode - 0
  Serial.print(0);
  // Old data format - 0
  Serial.println(1);
}

void reportResults() {
  Serial.print("Winner: ");
  Serial.println(winner);

  // N1 - New format 
  // Converts the race time data to the new timer format:
  // A=3.001!  B=3.002”  C=3.003#   D=3.004$  E=3.005%  F=3.006&   <CR>  <LF> 
  Serial.print("A=");
  Serial.print(elapsedTimeSecs(elapsedL1Time));
  if (winner == 1 || winner == 0 || winner == 3) {
    Serial.print('!');
  } else {
    Serial.print('"');
  }
  Serial.print(" ");

  Serial.print("B=");
  Serial.print(elapsedTimeSecs(elapsedL2Time));
  if (winner == 2 || winner == 0 || winner == 3) {
    Serial.print('!');
  } else {
    Serial.print('"');
  }
  Serial.print(" ");

  Serial.print("C=");
  Serial.print(elapsedTimeSecs(0));
  Serial.print('#');
  Serial.print(" ");

  Serial.print("D=");
  Serial.print(elapsedTimeSecs(0));
  Serial.print('$');
  Serial.print(" ");

  Serial.print("E=");
  Serial.print(elapsedTimeSecs(0));
  Serial.print('%');
  Serial.print(" ");

  Serial.print("F=");
  Serial.print(elapsedTimeSecs(0));
  Serial.print('&');
  Serial.println();
}