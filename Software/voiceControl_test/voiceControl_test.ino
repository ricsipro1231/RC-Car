String command;

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
    command = "";
    command = Serial.readString();
    Serial.println(command);
  }

  if(command == "*move forward#") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    delay(3000);

    command = "";
  }
  else if(command == "*move backwards#") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    delay(3000);

    command = "";
  }
  else if(command == "") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}
