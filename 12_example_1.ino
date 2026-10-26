#include <Servo.h>

// Arduino pin assignment
#define PIN_SERVO 10
#define PIN_LED    9
#define PIN_TRIG  12
#define PIN_ECHO  13

// Servo pulse widths (unit: microseconds)
#define _DUTY_MIN 1500
#define _DUTY_MAX 3000
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX) / 2)

// Distance range (unit: cm)
#define _DIST_MIN 18.0f
#define _DIST_MAX 36.0f

// EMA coefficient
#define _EMA_ALPHA 0.3f

// Servo speed limit (unit: degree/second)
#define _SERVO_SPEED 100.0f

// Update intervals (unit: milliseconds)
#define INTERVAL      20UL
#define INTERVAL_DIST 60UL

// Echo timeout (unit: microseconds)
#define ECHO_TIMEOUT 10000UL

Servo myservo;

// Timing variables
unsigned long last_sampling_time;
unsigned long last_distance_time;

// Servo control variables
int duty_change_per_interval;
int duty_target;
int duty_curr;

// Distance variables
float dist_raw = 0.0f;
float dist_ema = (_DIST_MIN + _DIST_MAX) / 2.0f;

bool ema_initialized = false;

void setup() {
  // Initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_TRIG, LOW);

  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX);

  // Start from the neutral position
  duty_target = duty_curr = _DUTY_NEU;
  myservo.writeMicroseconds(duty_curr);

  Serial.begin(57600);

  // Convert angular speed into pulse-width change per interval
  duty_change_per_interval = (int)(
    (_DUTY_MAX - _DUTY_MIN)
    * (_SERVO_SPEED / 180.0f)
    * (INTERVAL / 1000.0f)
    + 0.5f
  );

  // Prevent the servo from getting stuck due to rounding
  if (duty_change_per_interval < 1) {
    duty_change_per_interval = 1;
  }

  last_sampling_time = millis();

  last_distance_time = millis() - INTERVAL_DIST;
}

void loop() {
  unsigned long now = millis();

  // 1. Measure distance and update the target position
  if (now - last_distance_time >= INTERVAL_DIST) {
    last_distance_time = now;

    // Send the ultrasonic trigger pulse
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);

    digitalWrite(PIN_TRIG, LOW);

    unsigned long duration =
      pulseIn(PIN_ECHO, HIGH, ECHO_TIMEOUT);

    // Convert round-trip time into distance
    dist_raw = duration * 0.0343f / 2.0f;

    bool measurement_valid = (duration > 0);

    bool in_range =
      measurement_valid
      && dist_raw >= _DIST_MIN
      && dist_raw <= _DIST_MAX;

    digitalWrite(PIN_LED, in_range ? HIGH : LOW);

    if (measurement_valid) {
      // Range filter: clamp valid measurements to 18~36 cm
      float dist_filtered =
        constrain(dist_raw, _DIST_MIN, _DIST_MAX);

      // Initialize EMA using the first valid measurement
      if (!ema_initialized) {
        dist_ema = dist_filtered;
        ema_initialized = true;
      } else {
        // EMA filter
        dist_ema =
          _EMA_ALPHA * dist_filtered
          + (1.0f - _EMA_ALPHA) * dist_ema;
      }

      // Map 18~36 cm to the calibrated servo pulse range
      float ratio =
        (dist_ema - _DIST_MIN) / (_DIST_MAX - _DIST_MIN);

      duty_target = (int)(
        _DUTY_MIN
        + ratio * (_DUTY_MAX - _DUTY_MIN)
        + 0.5f
      );

      duty_target =
        constrain(duty_target, _DUTY_MIN, _DUTY_MAX);
    }

  }

  // 2. Update the servo with a speed limit
  now = millis();

  if (now - last_sampling_time >= INTERVAL) {
    last_sampling_time = now;

    if (duty_target > duty_curr) {
      duty_curr += duty_change_per_interval;

      if (duty_curr > duty_target) {
        duty_curr = duty_target;
      }
    } else if (duty_target < duty_curr) {
      duty_curr -= duty_change_per_interval;

      if (duty_curr < duty_target) {
        duty_curr = duty_target;
      }
    }

    myservo.writeMicroseconds(duty_curr);

    // 3. Serial Plotter output required by the assignment
    Serial.print("Min:");
    Serial.print(_DIST_MIN);

    Serial.print(",dist:");
    Serial.print(dist_raw);

    Serial.print(",ema:");
    Serial.print(dist_ema);

    Serial.print(",Servo:");
    Serial.print(myservo.read());

    Serial.print(",Max:");
    Serial.print(_DIST_MAX);

    Serial.println("");
  }
}
