# arduino-pinewood-derby-tracker

Arduino based firmware for tracking Pinewood Derby races.

## Summary

This firmware is designed to provide a basic hardware device to automate tracking of Pinewood Derby races. It is
intended to provide a simple Open Source alternative to commercial products for the DIYer.

Summary:
* Supports 2-lanes (can be expanded to support more lanes)
* Tracks the time of a race and identifies the winning (first lane to cross the finish line)
* Serial interface that emulates the Micro Wizard K1 timer for use with derby software like DerbyDay! and Derby Net
  * NOTE: Serial interface currently implements features supported by Derby Net
  * Many responses are hard-coded

## Operating Modes

This software includes 1 operating mode:

1. Run Mode - Used to run races

By default, the tracker will transition into "Run Mode" automatically after startup. 

### Run Mode

Run mode operates as a basic state machine which uses the digital inputs to transition between states.

States:
1. Startup - Holding mode after startup - Only occurs once after startup is complete
  * Finish line LED's Alternate L1 / L2 2x a second in this mode 
  * Press Reset Button to transition to "Ready" state
2. Ready - Ready to start a race - Cars should typically be staged at this point
  * Race results header is output to the Serial Console using the Micro Wizard N1 format - `A=X.YYYY! B=M.NNNNN"...`
  * Both Finish Line LED's will flash 4x at the start of this mode, and then go solid
  * Press Start Button / Trigger Start Gate to transition to "Racing" state
3. Racing - Race has started - Cars should be on their way down the track
  * Both Finish Line LED's will flash quickly (10x per second) during the race
  * Transitions to "Finished" state when one of the finish line switches are activated
4. Finished - Race is over - Results are generated
  * Winning Lane LED is activated
  * Race Results are output to the Serial once both cars pass the finish line or the max race time has been exceeded - `A=X.YYYY! B=M.NNNNN"...`
  * Press Reset Button to close out any pending lanes and transition to the "Ready" state for the next race
    * If start was triggered but no finish line switches are tripped, race will be recorded with lane times of `00.000`
    * If one finish switch was not tripped, race time of completed lane will be recorded and non-finished lane will report `00.000`, Winning Lane will be the completed lane


## Hardware

This sketch was designed and tested using an Arduino Nano knock-off.

It was designed and tested using a USB cable to provide power and to capture the Serial output.

### Inputs

The following inputs are required (Based on Arduino Nano Hardware):
* Pin D4 - Input for Lane 1 Switch
* Pin D5 - Input for Lane 2 Switch
* Pin D6 - Reset Button / Switch
* Pin D7 - Start Button / Start Gate

Input Pins Are Configured as Follows:
* Digital Inputs with Internal Pull-up Resistor Activated
* Input should provide ground signal when button is pressed, switch is activated, or beam is crossed 
* Input should be open otherwise
* Input logic provides no debounce behavior to reduce risk of inconsistent finish results due to differences in switches
  * Run Mode operates as a state machine with delays between each state change to eliminate the need for debouncing
    
NOTE: Ensure you are not providing excessive positive voltage on the input pins. Pins are only designed for 5v
(+/- 0.5v) maximum. Exceeding these values may damage the pins or microcontroller.

### Outputs

The following outputs are required (Based on Arduino Nano Hardware):
* Pin D2 - Output for Lane 1 LED
* Pin D3 - Output for Lane 2 LED
* Serial over USB for race tracking and debug output
  * Configured Rate:  115200 bps
  * 8N1 - 8 data bits, no parity, 1 stop bit, no flow control

Output LED Pins Limits:
* Voltage: 5v
* Current limit
  * Continuous: 20ma
  * Peak: 40ma

NOTE: Make sure to size your current limiting resistor appropriately to avoid drawing too much current from the pins.
Exceeding these values may damage the pins or microcontroller.