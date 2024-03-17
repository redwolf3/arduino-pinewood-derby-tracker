#ifndef constants
#define constants


/* Status Pin to show Arduino State */
const int16_t STATUS_LED_PIN = LED_BUILTIN;

/* LED Output Pin for Lane 1 - D2 */
const int16_t LED_L1 = PD2;
/* LED Output Pin for Lane 2 - D3 */
const int16_t LED_L2 = PD3;

/* Input Pin for Lane 1 Finish Switch - D4 */
const int16_t FINISH_SWITCH_L1 = PD4;
/* Input Pin for Lane 2 Finish Switch - D5 */
const int16_t FINISH_SWITCH_L2 = PD5;

/* Input Pin for Reset Button - D6 */
const int16_t RESET_BUTTON = PD6;
/* Input Pin for Start Button - D7 */
const int16_t START_BUTTON = PD7;

/* Define base operating modes */
const int16_t MODE_STARTUP = 0;
const int16_t MODE_RUN = 1;
const int16_t MODE_TEST = 2;

/* Define Run States */
const int16_t STATE_STARTUP = 0;
const int16_t STATE_READY = 1;
const int16_t STATE_RACING = 2;
const int16_t STATE_FINISHED = 3;

/* Define Button Press Report Interval (in milliseconds for Test Mode */
const uint32_t TEST_MODE_REPORT_INTERVAL = 1000;
const uint32_t TEST_MODE_STATUS_BLINK_INTERVAL = 500;
const uint32_t TEST_MODE_UPTIME_REPORT_INTERVAL = 10000;

const uint32_t RUN_MODE_RESET_NEW_LINE_INTERVAL = 1000;

const uint32_t READY_STATE_TRANSITION_BLINK_INTERVAL = 300;
const int16_t READY_STATE_TRANSITION_BLINK_COUNT = 4;

const uint32_t STAGING_BLINK_INTERVAL = 1000;
const uint32_t RACING_BLINK_INTERVAL = 100;
const uint32_t FINISHED_STATE_BLINK_INTERVAL = 1000;


#endif