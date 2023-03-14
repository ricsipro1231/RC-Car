char character;

#define EN (3)
#define IN1 (4)
#define IN2 (5)

void setup() {
  Serial.begin(9600);

  pinMode(EN, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {
  analogWrite(EN, 255);

  if(Serial.available()) {
      character = Serial.read();
  }

  if(character == 'F') {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
  else if(character == 'B') {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  Serial.println(character);

  delay(10);

}
