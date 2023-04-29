const int dirPin = 8;
const int stepPin = 9;

const int steps = 1150;
int microPausa = 13050;

void f_Delay_Micro_Sec(unsigned int Delay){
  unsigned long useconds, useconds_Dif;
  char Buffout[40];
  useconds = micros();
  delayMicroseconds(Delay);
  delayMicroseconds(Delay);
  useconds_Dif = micros() - useconds;
  sprintf(Buffout, "\nRequested=%6u Real=%6lu", 2*Delay+400, useconds_Dif);
  Serial.print(Buffout);
}

void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(dirPin, LOW);
  for (int s = 0; s <= 36000; s++) {
    digitalWrite(LED, HIGH);
    for (int x = 0; x <= steps; x++) {
      digitalWrite(stepPin, HIGH);
      f_Delay_Micro_Sec(microPausa);
      digitalWrite(stepPin, LOW);
      f_Delay_Micro_Sec(microPausa);
      if (x % 32 == 0) {
        Serial.print("\nSteps=");
        Serial.print(x);
        Serial.print(" microPausa=");
        Serial.print(2*microPausa);
      }
    }
    digitalWrite(LED, LOW);
    delayMicroseconds(100);
  }
}
