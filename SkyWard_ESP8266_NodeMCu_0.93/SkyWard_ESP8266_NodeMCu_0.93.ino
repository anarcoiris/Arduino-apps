#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoOTA.h>

const char* ssid = "SkyWard";   // Nombre del punto de acceso
const char* password = "(UseYourPwd)";  // Contraseña del punto de acceso
const int dirPin = 12;  //steper pins
const int stepPin = 13;
const int camPin = 14;  //optocoupler ON pin
const int enablePin = 2; // buscando el LED
const int steps = 512;
float expTime = 60;
int totalPhoto = 150;
int totalSecs = 0;
int arcMinutes = 0;
int totalSteps = 0; 
int stepCount = 0;
int photoCount = 0;
int microPausa = 25850;
unsigned long lastStepTime = 0;     // Tiempo del último paso del motor
unsigned long lastPhotoTime = 0;    // Tiempo de la última fotografía tomada
bool takingPhoto = false;           // Indica si se está tomando una fotografía
unsigned long shutterTime = 0;      // Timer del obturador

ESP8266WebServer server(80);   // Servidor web en el puerto 80


void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(camPin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  Serial.begin(115200);

  // Iniciar el modo de punto de acceso
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("Observatorio astronómico iniciado");
  Serial.print("IP del punto de acceso: ");
  Serial.println(WiFi.softAPIP());

  // Inicializar el sistema de archivos SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Error al iniciar el sistema de archivos SPIFFS");
    return;
  }
  // Asignar el controlador de ruta del servidor para la raíz "/"
  server.on("/", handleRoot);
    // Route for serving the image
  server.on("/background.jpeg", [](){
    File file = SPIFFS.open("/background.jpeg", "r");
    if (!file) {
      Serial.println("Failed to open file for reading");
      server.send(404, "text/plain", "File not found");
      return;
    }
    size_t sent = server.streamFile(file, "image/jpeg");
    file.close();
    Serial.printf("Sent %d bytes\n", sent);
  });
  server.on("/move", handleMove);
  server.on("/currentMicros", [](){
      unsigned long currentMicros = microPausa;
      server.send(200, "text/plain", String(currentMicros));
  });
  server.on("/currentPhoto", [](){
      unsigned long currentPhoto = totalPhoto;
      server.send(200, "text/plain", String(currentPhoto));
  });
  server.on("/currentPhotoCount", [](){
      unsigned long currentPhotoCount = photoCount;
      server.send(200, "text/plain", String(currentPhotoCount));
  });    
  server.on("/currentExpTime", [](){
    //if (digitalRead(enablePin) == HIGH) {
      unsigned long currentExpTime = expTime;
      server.send(200, "text/plain", String(currentExpTime));
    //}
  });
  server.on("/hemisferio", [](){
    if (digitalRead(dirPin) == LOW) {
      String hemisferio = "Norte";
      server.send(200, "text/plain", String(hemisferio));
    }
    if (digitalRead(dirPin) == HIGH) {
      String hemisferio = "Sur";
      server.send(200, "text/plain", String(hemisferio));
    }
  });
  server.on("/estado", [](){
    if (digitalRead(enablePin) == HIGH) {
      String estado = "Apagado";
      server.send(200, "text/plain", String(estado));
    }
    if (digitalRead(enablePin) == LOW) {
      String estado = "Encendido";
      server.send(200, "text/plain", String(estado));
    }
  });
   server.on("/setMicroPausa", handleSetValue);
   server.on("/setTotalPhoto", handleSetValue);
   server.on("/targetHRh", handleSetValue);
   server.on("/targetHRm", handleSetValue);
  // Iniciar el servidor
  server.begin();
  // Start ArduinoOTA
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
  // Revisar si el motor debe moverse
  char Buffout[40];
  if (digitalRead(enablePin) == LOW) {
    unsigned long currentTime = micros();
    // Controlar el intervalo entre pasos del motor
    if (currentTime - lastStepTime >= microPausa) {
      // Generar un pulso del motor
      digitalWrite(stepPin, !digitalRead(stepPin));
      //DEBUG de tiempos
      sprintf(Buffout, "\nRequested=%6u Real=%6lu", microPausa, currentTime - lastStepTime);
      Serial.print(Buffout);
      lastStepTime = currentTime;
      stepCount++;
      return;
    }
    unsigned long shutterTime = millis();
    // Tomar una fotografía si es el momento adecuado
    if (takingPhoto && shutterTime - lastPhotoTime >= (expTime+0.6) * 1000) {
        // utilizando el pin camPin
        digitalWrite(camPin, LOW);
    }
    if (takingPhoto && shutterTime - lastPhotoTime >= (expTime+2.1) * 1000) {
        digitalWrite(camPin, HIGH);
        lastPhotoTime = shutterTime;
        photoCount++;
        return;
    }
  } else {
    ArduinoOTA.handle();
    digitalWrite(camPin, LOW);
    }
  // Manejar las solicitudes entrantes
  server.handleClient();
  if (photoCount > totalPhoto) {
    digitalWrite(enablePin, LOW);
    digitalWrite(camPin, LOW);
    takingPhoto = false;
    photoCount = 0;
  }
}

void handleImageRequest() {
  // Generate the JPEG image data
  // Replace this with your own code to generate the image
  // For example, you could use the Arduino JPEGDecoder library to decode a JPEG file and generate the image data
  byte imageData[] = { 0xFF, 0xD8, 0xFF, 0xE0, /* ... */ 0xFF, 0xD9 };

  // Send the image data in the response
  server.sendHeader("Content-Type", "image/jpeg");
  server.send(200, "", imageData, sizeof(imageData));
}
void handleMove() {
  String direction = server.arg("move");

  if (direction == "sur") {
    digitalWrite(dirPin, HIGH);
    Serial.println("dir sur");
  } else if (direction == "norte") {
    digitalWrite(dirPin, LOW);
    Serial.println("dir norte");
  } else if (direction == "stop") {
    // Detener el motor
    digitalWrite(enablePin, HIGH);
    Serial.println("stop");
  } else if (direction == "start") {
    // Iniciar el motor
    Serial.println("start");
    photoCount = 0;
    digitalWrite(enablePin, LOW);
  } else if (direction == "mas") {
    // Aumentar delay
    Serial.println("mas");
    microPausa = microPausa + 50;
  } else if (direction == "menos") {
    // Decrem. delay
    Serial.println("menos");
    microPausa = microPausa - 50;
  }  else if (direction == "arriba") {
    // Decrem. delay
    Serial.println("arriba");
    microPausa = microPausa/1.41421;
  } else if (direction == "abajo") {
    // Decrem. delay
    Serial.println("abajo");
    microPausa = microPausa*1.41421;
  } else if (direction == "shot") {
    // Toggle cam
    Serial.println("shot");
    takingPhoto = true;   
  } else if (direction == "noshot") {
    // Toggle cam
    Serial.println("noshot");
    if (takingPhoto == true) {
      // utilizando el pin camPin
      digitalWrite(camPin, LOW);
      photoCount = 0;
      takingPhoto = false;
    }    
  } else if (direction == "tmas") {
    // Toggle cam
    Serial.println("tmas");
    expTime = expTime + 5;
  } else if (direction == "tmenos") {
    // Toggle cam
    Serial.println("tmas");
    expTime = expTime - 5;
  }
 
  
}

// Servir archivos
void handleRoot() {
  // Serve the index.html file
  File indexFile = SPIFFS.open("/index.html", "r");
  if (!indexFile) {
    Serial.println("Error al abrir el archivo index.html");
    server.send(500, "text/plain", "Error interno del servidor");
    return;
  }
  server.streamFile(indexFile, "text/html");
  indexFile.close();

  // Serve the background.jpeg image
  File imageFile = SPIFFS.open("/background.jpeg", "r");
  if (!imageFile) {
    Serial.println("Error al abrir el archivo background.jpeg");
    server.send(500, "text/plain", "Error interno del servidor");
    return;
  }
  server.streamFile(imageFile, "image/jpeg");
  imageFile.close();
}

// Función para manejar las peticiones POST en la ruta /submit
void handleSubmit() {
// Obtener el valor enviado en el formulario
  String message = server.arg("message");

// Guardar el mensaje en un archivo
  File file = SPIFFS.open("/message.txt", "w");
  if (!file) {
    Serial.println("Error al abrir el archivo message.txt");
    server.send(500, "text/plain", "Error interno del servidor");
    return;
  }
  file.println(message);
  file.close();

// Redireccionar al cliente a la página principal
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}
void rightAscension(int deltaRA) {
  // Obtener el número total de pasos
  totalSteps = stepCount * 32;

  // Calcular el tiempo total en microsegundos
  totalSecs = totalSteps * microPausa*2/(10^6);

  // Calcular los minutos de arco desplazados
  arcMinutes = totalSecs / 60;
}

void stepCalculator(){
  int totalSecs = 60 * arcMinutes;
  int totalSteps = totalSecs*(10^6)/(microPausa*2); 
}
void handleTargetn() {
  if (targetHRm > 60) {
    targetHRm=0;
    targetHRh=targetHRh+1;
  }
  targetMinutes = 60*targetHRh+targetHRm;
  minutesMoved = ;
  currentMinutes = 60*setHRh+setHRm + elapsedGotoTime;
  
}
void handleCurrentPosition () {
  cPosition = iPosition + stepCount*microPausa/(60000000)
}
void handleSetValue() {
  if (server.hasArg("microPausa")&&(server.arg("microPausa").toInt()!=microPausa)) {
    // Update the microPausa variable with the new value
    microPausa = server.arg("microPausa").toInt();
  }
  if (server.hasArg("totalPhoto")&&(server.arg("totalPhoto").toInt()!=totalPhoto)) {
    // Update the microPausa variable with the new value
    totalPhoto = server.arg("totalPhoto").toInt();
  }
  if (server.hasArg("targetHRh")&&(server.arg("targetHRh").toInt()!=targetHR)) {
    // Update the microPausa variable with the new value
    targetHRh = server.arg("targetHRh").toInt();
  }
  if (server.hasArg("targetHRm")&&(server.arg("targetHRm").toInt()!=targetHRm)) {
    // Update the microPausa variable with the new value
    targetHRm = server.arg("targetHRm").toInt();
  }
  if (server.hasArg("initialHRh")&&(server.arg("initialHRh").toInt()!=initialHR)) {
    // Update the microPausa variable with the new value
    initialHRh = server.arg("initialHRh").toInt();
  }
  if (server.hasArg("initialHRm")&&(server.arg("initialHRm").toInt()!=initialHRm)) {
    // Update the microPausa variable with the new value
    initialHRm = server.arg("initialHRm").toInt();
  }
  // Return a response to the client
  server.send(200, "text/plain", "OK");
}
