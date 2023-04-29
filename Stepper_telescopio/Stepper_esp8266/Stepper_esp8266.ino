const int dirPin = 4;
const int stepPin = 5;
const int enablePin = 2;
const int steps = 1150;
int microPausa = 26500;
 
void setup() {
 pinMode(dirPin, OUTPUT);
 pinMode(stepPin, OUTPUT);
}
 
void loop() {
 digitalWrite(enablePin, LOW);
 digitalWrite(dirPin, LOW);  // LOW = Dirección (Sur) (Norte)??
 for (int s = 0; s <= 7200; s++) {
  for (int x = 0; x <= steps ; x++) {
     digitalWrite(stepPin, HIGH);
     delayMicroseconds(microPausa);
     digitalWrite(stepPin, LOW);
     delayMicroseconds(microPausa);
  }
 delay(10);
 }

}
