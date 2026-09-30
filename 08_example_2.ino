// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)

#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT (INTERVAL * 500.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // coefficient to convert duration to distance

unsigned long last_sampling_time;   // unit: msec

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO

  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float distance;
  int brightness;

  // wait until next sampling time
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  // measure distance
  distance = USS_measure(PIN_TRIG, PIN_ECHO);

  // 거리와 LED 밝기 계산
  if ((distance == 0.0) || (distance > _DIST_MAX)) {

    // 300mm 초과 또는 측정 실패 → LED OFF
    distance = _DIST_MAX + 10.0;
    brightness = 255;

  } 
  else if (distance < _DIST_MIN) {

    distance = _DIST_MIN - 10.0;
    brightness = 255;

  } 
  else if (distance <= 200.0) {

    brightness = (int)(255.0 * (200.0 - distance) / 100.0);

  } 
  else {

    brightness = (int)(255.0 * (distance - 200.0) / 100.0);
  }

  analogWrite(PIN_LED, brightness);

  // output the distance to the serial port
  Serial.print("Min:");
  Serial.print(_DIST_MIN);
  Serial.print(",distance:");
  Serial.print(distance);
  Serial.print(",Max:");
  Serial.print(_DIST_MAX);
  Serial.println("");

  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
