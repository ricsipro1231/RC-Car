#include "Motor.h"
#include "Pins.h"
#include <Servo.h>
#include "Bluetooth.h"
#include "Button.h"
#include "HC_SR04.h"                                                
#include "L3G4200D.h"

uint32_t currentTime = 0;
uint32_t previousTime = 0;

uint32_t selectModeCurrentTime = 0;
uint8_t selectModePreviousTime = 0;
uint16_t interval = 6000;
uint8_t exitState = 0;

uint8_t counter = 0;
uint8_t s_e_l_e_c_t = 0;
double y_axis = 0;

double x_axis = 0;
double z_axis = 0;

Bluetooth bluetooth;

Servo HC_SR04_SERVO;
Motor motors(MOTOR1_PIN_1, MOTOR1_PIN_2, MOTOR1_EN, MOTOR2_PIN_1, MOTOR2_PIN_2, MOTOR2_EN);

HC_SR04 Sensor(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);

Button Mode_Select_Button(MODE_SELECT_BUTTON, 0);

L3G4200D Gyro;

void Bluetooth_Mode();  
void Obstacle_avoiding();
void Voice_control();
void Line_follower();

void exitStateCheck() {
  if(digitalRead(MODE_SELECT_BUTTON)) {
    if(exitState == 0) {
      exitState = 1;
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(INFRA_SENSOR_RIGHT, INPUT);
  pinMode(INFRA_SENSOR_LEFT, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  if(!motors.Init()) {
    Serial.println("Motor ERROR");
  }

  if(!bluetooth.Init()) {
    Serial.println("Bluetooth ERROR");
  }

  if(!Mode_Select_Button.Init()) {
    Serial.println("Button ERROR");
  }

  if(!Sensor.Init()) {
    Serial.println("HC_SR04 ERROR");
  }

  HC_SR04_SERVO.attach(SERVO1_PIN);

  attachInterrupt(digitalPinToInterrupt(MODE_SELECT_BUTTON), exitStateCheck, RISING);
  exitState = 1;
}

void loop() {
  exitState = 1;

  if(Mode_Select_Button.Read(500)) {
    counter ++;
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  if(counter != 0) {
    selectModeCurrentTime = millis();

    if(selectModeCurrentTime - selectModePreviousTime >= interval && !digitalRead(MODE_SELECT_BUTTON)) {
      s_e_l_e_c_t = 1;
    }
    selectModePreviousTime = selectModeCurrentTime;
  }
  else {
    selectModeCurrentTime = 0;
    selectModePreviousTime = 0;
  }

  if(s_e_l_e_c_t == 1) {
    switch (counter) {
        case 1:
          exitState = 0;
          Bluetooth_Mode();
        break;

        case 2:
          exitState = 0;
          Obstacle_avoiding();
        break;

        case 3:
          exitState = 0;
          Line_follower();
        break;

        case 4:
          exitState = 0;
          Voice_control();
        break;          
      }
  }
}

void Bluetooth_Mode() {
  while (exitState == 0) {
    char bluetoothCommand;
    digitalWrite(LED_BUILTIN, HIGH);

    bluetoothCommand = bluetooth.ReadMessage();

    if(bluetoothCommand == 'F') {
      motors.Forward(190);
    }

    else if(bluetoothCommand == 'B') {
      motors.Backward(190);
    }

    else if(bluetoothCommand == 'R') {
      motors.TurnRight(190);
    }

    else if(bluetoothCommand == 'L'){
      motors.TurnLeft(190);
    }

    else {
      motors.Stop();
    }
  }

  counter = 0;
  s_e_l_e_c_t = 0;
  digitalWrite(LED_BUILTIN, LOW);
  return;
}

void Obstacle_avoiding() {
  uint32_t distance = 0;
  uint32_t distanceLeft = 0;
  uint32_t distanceRight = 0;

  while (exitState == 0) {
    distance = Sensor.Read();
    digitalWrite(LED_BUILTIN, HIGH);

    if(distance < 15) {
      currentTime = millis();

      motors.Backward(190);

      if(currentTime - previousTime >= 1000) {
        motors.Stop();

        previousTime = currentTime;
      }

      if(currentTime - previousTime > 2500) {
        HC_SR04_SERVO.write(90);
        distanceLeft = Sensor.Read();

        previousTime = currentTime;
      }

      if(currentTime - previousTime > 3500) {
        HC_SR04_SERVO.write(170);
        distanceRight = Sensor.Read();

        previousTime = currentTime;
      }

      if(currentTime - previousTime >= 4000) {
        HC_SR04_SERVO.write(80);
      }

      if(distanceRight > distanceLeft) {
          currentTime = millis();

          motors.TurnRight(190);

          if(currentTime - previousTime >= 1000) {
            motors.Stop();

            previousTime = currentTime;
          }
      }
      else {
          currentTime = millis();

          motors.TurnLeft(190);

          if(currentTime - previousTime >= 1000) {
            motors.Stop();

            previousTime = currentTime;
          }
      }
    }
    else {
      motors.Forward(190);
    }
  }

  counter = 0;
  s_e_l_e_c_t = 0;
  digitalWrite(LED_BUILTIN, LOW);
  return;
}

void Voice_control() {
  String Data = "";

  while (exitState == 0) {
     Data = bluetooth.ReadString();
     digitalWrite(LED_BUILTIN, HIGH);

    if(Data == "*move forward#") {
      currentTime = millis();
      
      motors.Forward(190);
      
      if(currentTime - previousTime > 1000) {
        motors.Stop();

        previousTime = currentTime;
      }

      Data = "";
    }

    else if(Data == "*move backward#") {
      motors.Backward(190);

      if(currentTime - previousTime > 1000) {
        motors.Stop();

        previousTime = currentTime;
      }

      Data = "";
    }

    else if(Data == "*turn left#") {
        Gyro.Read(x_axis, y_axis, z_axis);

        if(x_axis != 90) {
          Gyro.Read(x_axis, y_axis, z_axis);

          motors.TurnLeft(190);
        }

      if(currentTime - previousTime > 1000) {
        motors.Stop();

        previousTime = currentTime;
      }

      Data = "";
    }

    else if(Data == "*turn right#") {
        if(x_axis != 90) {
          Gyro.Read(x_axis, y_axis, z_axis);

          motors.TurnRight(190);
        }

      if(currentTime - previousTime > 1000) {
        motors.Stop();

        previousTime = currentTime;
      }

      Data = "";
    }

    else if(Data == "*stop#") {
      motors.Stop();

      Data = "";
    }
  }

  counter = 0;
  s_e_l_e_c_t = 0;
  digitalWrite(LED_BUILTIN, LOW);
  return;
} 

void Line_follower() {
  while (exitState == 0)  {
      digitalWrite(LED_BUILTIN, HIGH);

    if(analogRead(INFRA_SENSOR_RIGHT) <= 35 && analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.Forward(100);

    else if(analogRead(INFRA_SENSOR_RIGHT) <= 35 && !analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.TurnRight(255);

    else if(!analogRead(INFRA_SENSOR_RIGHT) <= 35 && analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.TurnRight(255);

    else if(!analogRead(INFRA_SENSOR_RIGHT) <= 35 && !analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.Stop();
  }

  counter = 0;
  s_e_l_e_c_t = 0;
  digitalWrite(LED_BUILTIN, LOW);
  return;
}
