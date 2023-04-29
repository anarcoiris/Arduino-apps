const int LED = 13;
int numMeasurements = 10;

void f_Delay_Micro_Sec(unsigned int Delay){
  unsigned long useconds, useconds_Dif;
  useconds = micros();
  delayMicroseconds(Delay);
  useconds_Dif = micros() - useconds;
  Serial.print(Delay);
  Serial.print(",");
  Serial.println(useconds_Dif);
}

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(LED, HIGH);

  for (int i = 0; i <= 100; i++) {
    int delayTime = i * 500;
    Serial.print("Delay=");
    Serial.println(delayTime);
    
    unsigned long totalElapsed = 0;
    for (int j = 0; j < numMeasurements; j++) {
      f_Delay_Micro_Sec(delayTime);
      totalElapsed += micros();
    }
    unsigned long averageElapsed = totalElapsed / numMeasurements;
    Serial.print("Avg Elapsed=");
    Serial.println(averageElapsed);

    delay(100);
  }

  digitalWrite(LED, LOW);
  delay(5000);
}
