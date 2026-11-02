#include <Servo.h>

#define PIN_SERVO 10


#define _DUTY_MIN 0   
#define _DUTY_NEU 1500 
#define _DUTY_MAX 3000 

#define _INTERVAL_SERVO  20     
#define _INTERVAL_SERIAL 1000   
#define _PAUSE_MS        5000   


#define SPEED_1   3.0
#define ANGLE_1   180.0

#define SPEED_2   0.3
#define ANGLE_2   90.0

Servo myservo;

float _SERVO_SPEED;           
float angle_target;           
float duty_start;             
float duty_curr;             
unsigned long time_start;     
unsigned long last_servo;     
unsigned long last_serial;    
unsigned long time_pause;     

int phase;                    


float angleToDuty(float angle) {
  return _DUTY_MIN + (_DUTY_MAX - _DUTY_MIN) * angle / 180.0;
}


void startExperiment(float speed, float target) {
  _SERVO_SPEED = speed;
  angle_target = target;
  duty_start   = _DUTY_MIN;
  duty_curr    = _DUTY_MIN;
  myservo.writeMicroseconds(_DUTY_MIN);  
  delay(2000);                          
  time_start  = millis();
  last_servo  = time_start;
  last_serial = time_start;
}


void printStatus() {
  float elapsed = (millis() - time_start) / 1000.0;
  float angle   = (duty_curr - _DUTY_MIN) * 180.0 / (_DUTY_MAX - _DUTY_MIN);
  Serial.print("SPEED=");   Serial.print(_SERVO_SPEED, 1);
  Serial.print(" | time="); Serial.print(elapsed, 1);
  Serial.print("s | duty="); Serial.print(duty_curr, 1);
  Serial.print("us | angle="); Serial.print(angle, 1);
  Serial.println("deg");
}

void setup() {
  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_MIN);
  Serial.begin(57600);
  Serial.println("=== Servo slow drive test (13C02) ===");
  delay(2000);

  Serial.println("[Experiment 1] _SERVO_SPEED = 3 : 0 -> 180 deg in 60 s");
  startExperiment(SPEED_1, ANGLE_1);
  phase = 1;
}

void loop() {
  unsigned long now = millis();

  if (phase == 1 || phase == 3) {

    if (now - last_servo >= _INTERVAL_SERVO) {
      last_servo += _INTERVAL_SERVO;

      float elapsed = (now - time_start) / 1000.0;
      float angle   = _SERVO_SPEED * elapsed;
      if (angle > angle_target) angle = angle_target;

      duty_curr = angleToDuty(angle);
      myservo.writeMicroseconds((int)(duty_curr + 0.5));
    }

    if (now - last_serial >= _INTERVAL_SERIAL) {
      last_serial += _INTERVAL_SERIAL;
      printStatus();
    }

    if (duty_curr >= angleToDuty(angle_target)) {
      printStatus();
      if (phase == 1) {
        Serial.println("[Experiment 1] done");
        time_pause = millis();
        phase = 2;
      } else {
        Serial.println("[Experiment 2] done");
        phase = 4;
      }
    }
  }

  else if (phase == 2) {
    if (now - time_pause >= _PAUSE_MS) {
      Serial.println("[Experiment 2] _SERVO_SPEED = 0.3 : 0 -> 90 deg in 300 s");
      startExperiment(SPEED_2, ANGLE_2);
      phase = 3;
    }
  }

}
