/* Optical Tacometer with measurements output to serial monitor

  Measure the time needed for some number of  sensor transitions, and calculate
  RPM that implies, assuming a single transition (in one direction) per revolution.

  variant of https://github.com/WCRSyyc/optical_tachometer

  The version has been adjusted to control measurement and reportomg timing
  using ONLY the standard timers.  No delay() calls are used.
*/
const int STATUS_LED_PIN = LED_BUILTIN;  // Toggle showing the sketch seeing inputs
const int RPM_SENSOR_PIN = 2;  // Interrupt sense input
// 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200
const long SERIAL_BAUD = 115200;
const unsigned long UPDATE_THROTTLE = 500;  // minimum 0.5 sec between speed calcs
const unsigned long IDLE_TIMEOUT = 5000;  // milliseconds to wait for new measurement
const unsigned int UPDATE_COUNT = 5;  // minimum revolutions to use for calculations
const unsigned int TICKS_PER_REV = 1;  // number of transitions per revolution
volatile unsigned int revolutionCount;    // volatile (updated by interrupt handler)
unsigned int previousCounted;  // most recent recorded sensor event count
unsigned long int maximumRPM;  // Maximum RPM measurement
unsigned long measurementStart;  // time latest measurement was started
unsigned long measurmentReported;  // time most recent measurement was reported
unsigned long idleStart = 0;  // previous time a measurement was recorded
int measureStatus = LOW;       // Status LED state (value); toggled to show working
int RPMprevLen = 0;     // Previous RPM value displayed length
bool isIdle = true;  // Flag to prevent multiple reports of the same maximum

void setup()
{
  pinMode(STATUS_LED_PIN, OUTPUT);

  Serial.begin(SERIAL_BAUD);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("TACHOMETER"));
  Serial.println(F("  WCRS"));

  // Call Interrupt handler function on each LOW to HIGH transition
  attachInterrupt(digitalPinToInterrupt(RPM_SENSOR_PIN), revolutionCounter, RISING);

  revolutionCount = 0;  // First initialization to start measurements
  measurementStart = 0;
  previousCounted = 0;
}

void loop()
{
  unsigned long currtime = millis(); // the current time
  unsigned long idletime = currtime - idleStart;  // time since last measurement
  unsigned long throttleTime = currtime - measurmentReported;

  if((revolutionCount >= UPDATE_COUNT) & (throttleTime >= UPDATE_THROTTLE)) {
    // it´s time to report a new (raw) measurement
    reportLatestRPM();
    idleStart = currtime;  // Just had a new reading; reset start idle interval to now
    isIdle = false;
  }

  if(!isIdle & (idletime > IDLE_TIMEOUT)) {  // There has been no new reading for awhile
    showMaxRPM();  // Show the maximum RPM
    idleStart = currtime;
    isIdle = true;
    // TODO: prevent repeated maximum reports until have a new raw measurement
  }

  showActive();
}


/**
 * revolutionCounter Interrupt handler
 *
 * Every time the sensor goes from LOW to HIGH, this function will be called
 */
void revolutionCounter()
{
  revolutionCount++;  // Increment the number of revolutions detected
} // ./ revolutionCounter()


/**
 * calculate and report the current RPM
 */
void reportLatestRPM()
{
  /* RPM: revolutions per minute:
   * rpm
   *    = revolutions / minutes elapsed
   *    = rev / (seconds / 60) == (60 * rev) / seconds
   *    = rev / (milliseconds / 60000) == (60000 * rev) / milliseconds
   *    = rev / (microseconds / 60000000) == (60000000 * rev) / microseconds
   * ms = (millis() - measurementStart)
   * rev = revolutions / TICKS_PER_REV
   *
   * rpm = (60000 * rev) / ms
   *
   * rpm = (60000 * (revolutions / TICKS_PER_REV)) / (millis() - measurementStart)
   * rpm = (60000 * revolutions) / TICKS_PER_REV)) / (millis() - measurementStart)
   * rpm = (60000 * revolutions) / (TICKS_PER_REV * (millis() - measurementStart))
   *
   * Something is wrong in the calculation here
  */
  // Calculate the RPM using the time needed for the number of revolutions seen
  unsigned long int measuredRPM = 60 * 1000 * revolutionCount / (TICKS_PER_REV * (millis() - measurementStart));
  measurementStart = millis();  // Start a new measurement
  revolutionCount = 0;

  if(measuredRPM > maximumRPM) {
    maximumRPM = measuredRPM;   //  Track the maximum RPM reading seen
  }

  showRawRPM(measuredRPM);
}


/** showMaxRPM
 *
 * Display the maximum RPM value seen so far
 */
void showMaxRPM()
{
  Serial.println(F("\nMaximum RPM"));
  Serial.print(maximumRPM, DEC);
  Serial.println(F(" RPM\n\n Idle state\Ready to measure"));
}


/** showRawRPM
 *
 * Display the latest RPM measurement
 *
 * @param rpmValue - revolutions per minute value
 */
void showRawRPM(int rpmValue)
{
  Serial.print(F("Measuring "));
  Serial.print(rpmValue, DEC);
  Serial.println(F(" rpm"));
  measurmentReported = millis();  // marker to prevent continious updates
}


/** showActive
 *
 * Toggle an LED to show that the script is (still) seeing trigger events on the
 * sensor.
 */
void showActive()
{
  unsigned int countedRevolutions = revolutionCount;
  if(countedRevolutions != previousCounted) {  // count has changed since previous loop
    if (measureStatus == LOW) {  // toggle the status LED to show something is happening
      measureStatus = HIGH;
    } else {
      measureStatus = LOW;
    }
    digitalWrite(STATUS_LED_PIN, measureStatus);  // Toggle the status LED
    previousCounted = countedRevolutions;
  }
}
