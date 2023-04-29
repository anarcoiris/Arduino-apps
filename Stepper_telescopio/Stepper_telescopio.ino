const int dirPin = 8;
const int stepPin = 9;
const int LED = 13;
 
const int steps = 11500;
int microPausa = 13050;
 
void setup() {
 pinMode(dirPin, OUTPUT);
 pinMode(stepPin, OUTPUT);
}
 
void loop() {
 digitalWrite(dirPin, LOW);  // LOW = Hemisferio Norte, polaridad B2 B1 A1 A2
 for (int s = 0; s < 36000; s++) {
  digitalWrite(LED, HIGH);
  for (int x = 0; x < steps ; x++) {
     digitalWrite(stepPin, HIGH);
     delayMicroseconds(microPausa);
     delayMicroseconds(microPausa);
     digitalWrite(stepPin, LOW);
     delayMicroseconds(microPausa);
     delayMicroseconds(microPausa);
  }
  digitalWrite(LED, LOW);
  delayMicroseconds(100);
 }

}
