#include "Motor.h"
#include "Pins.h"
#include <Servo.h>
#include "Bluetooth.h"
#include "Button.h"
#include "HC_SR04.h"
#include "L3G4200D.h"

uint32_t currentTime = 0;
uint32_t previousTime = 0;

uint8_t counter = 0;
uint8_t exitState = 0;

double x_axis = 0;
double y_axis = 0;
double z_axis = 0;

Bluetooth bluetooth;

Servo HC_SR04_SERVO;
Motor motors(MOTOR1_PIN_1, MOTOR1_PIN_2, MOTOR1_EN, MOTOR2_PIN_1, MOTOR2_PIN_2, MOTOR2_EN);

HC_SR04 Sensor(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);

Button Mode_Select_Button(MODE_SELECT_BUTTON, 0);

L3G4200D Gyro;
#define SIDE_POSITION (90)

void Bluetooth_Mode();
void Obstacle_avoiding();
void Voice_control();
void Line_follower();

void checkExit() {
  if(exitState == 0) {
    if(digitalRead(MODE_SELECT_BUTTON)) {
      exitState = 1;
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(INFRA_SENSOR_RIGHT, INPUT);
  pinMode(INFRA_SENSOR_LEFT, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(MOTOR1_PIN_1, OUTPUT);
  pinMode(MOTOR1_PIN_2, OUTPUT);
  pinMode(MOTOR2_PIN_1, OUTPUT);
  pinMode(MOTOR2_PIN_2, OUTPUT);
  pinMode(MOTOR1_EN, OUTPUT);
  pinMode(MOTOR2_EN, OUTPUT);

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
  attachInterrupt(digitalPinToInterrupt(MODE_SELECT_BUTTON), checkExit, RISING);
}

void loop() {  
  if(Mode_Select_Button.Read(500)) {
      counter ++;

    if(counter > 5) {
      counter = 1;
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);    
  }
  
  if(Mode_Select_Button.Read(3000) && counter != 0) {
    counter --;
    exitState = 0;    
    
    switch(counter) {
      case 1:
        delay(1000);
        Bluetooth_Mode();
      break;

      case 2:
        delay(1000);
        Obstacle_avoiding();
      break;

      case 3:
        delay(1000);
        Voice_control();
      break;

      case 4:
        delay(1000);
        Line_follower();
      break;
    }
  }

  Serial.println(counter);
}

void Bluetooth_Mode() {
  while(exitState == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    char bluetoothCommand;

    if(Serial2.available()) {
      bluetoothCommand = Serial2.read();
      Serial.println(bluetoothCommand); 
    }
    

    if(bluetoothCommand == 'F') {
        digitalWrite(MOTOR1_PIN_1, LOW);
      digitalWrite(MOTOR1_PIN_2, HIGH);
      digitalWrite(MOTOR2_PIN_1, HIGH);
      digitalWrite(MOTOR2_PIN_2, LOW);  
      
      for(int i = 0; i < 255; i++) {
        analogWrite(MOTOR1_EN, i);
        analogWrite(MOTOR2_EN, i);
      }

      Serial.println("Fshdshd");
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

    bluetoothCommand = 'S';
  }

  counter = 0;
  return;
}

void Obstacle_avoiding() {
  Serial.println("Obstacle");
  
  uint32_t distance = 0;
  uint32_t distanceLeft = 0;
  uint32_t distanceRight = 0;

  digitalWrite(LED_BUILTIN, HIGH);
  
  while(exitState == 0) {
    distance = Sensor.Read();
    
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

      /*if(distanceRight > distanceLeft) {
          currentTime = millis();

          Gyro.Read(x_axis, y_axis, z_axis);

          while(x_axis != SIDE_POSITION) {
            Gyro.Read(x_axis, y_axis, z_axis);

            motors.TurnRight(190);
          }

          if(currentTime - previousTime >= 1500) {
            motors.Stop();

            previousTime = currentTime;
          }
      }*/
      /*else {
          currentTime = millis();

          Gyro.Read(x_axis, y_axis, z_axis);

          while(x_axis != SIDE_POSITION) {
            Gyro.Read(x_axis, y_axis, z_axis);

            motors.TurnLeft(190);
          }

          if(currentTime - previousTime >= 1500) {
            motors.Stop();

            previousTime = currentTime;
          }
      }*/
    }
    else {
      motors.Forward(190);
    }
  }

  counter = 0;
  return;
}

void Voice_control() {
  while(exitState == 0) {
    Serial.println("Voice");
    String Data = "";
    
    digitalWrite(LED_BUILTIN, HIGH);
    Data = bluetooth.ReadString();

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

        if(x_axis != SIDE_POSITION) {
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
      if(x_axis != SIDE_POSITION) {
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
  return;
}

void Line_follower() {
  while(exitState == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Line");

    if(analogRead(INFRA_SENSOR_RIGHT) <= 35 && analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.Forward(100);

    else if(analogRead(INFRA_SENSOR_RIGHT) <= 35 && !analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.TurnRight(255);

    else if(!analogRead(INFRA_SENSOR_RIGHT) <= 35 && analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.TurnLeft(255);

    else if(!analogRead(INFRA_SENSOR_RIGHT) <= 35 && !analogRead(INFRA_SENSOR_LEFT) <= 35)
      motors.Stop();
  }

  counter = 0;
  return;
}
