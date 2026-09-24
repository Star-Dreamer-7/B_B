#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Arduino.h>
#define TRIGGER_PIN 9
#define ECHO_PIN 10
int ir_pin = 6;
int led_pin =13;
int hasObstacle =LOW;
// Pines de los LEDs
const int purple_pin = 2;
const int orange_pin = 4;
const int yellow_pin = 8;
const int blue_pin = 12;
float ut= 0;
String rb = "";
int ij = 0;
const int wait = 500;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);
void apagarLeds() {
  digitalWrite(purple_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}
void setup() {
 Serial.begin(9600);

  pinMode(ir_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(purple_pin, OUTPUT);
  pinMode(orange_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  
  apagarLeds();

  Serial.println("Inicializando...");

  if (!tcs.begin()) {
    Serial.println("No se encontro el sensor TCS34725");
    while (1); // Se congela aquí si no detecta el sensor RGB
  }
  Serial.println("Sensor RGB listo");
}


void loop() {
  sensorUltrasonico();
  sensorRgb();
  sensorInfrarrojo();

  Serial.println("---Resumen sensores---");
  Serial.print("Ultrasonico: "); Serial.print(ut); Serial.print("cm | ");
  Serial.print("Rgb: "); Serial.print(rb); Serial.print(" | ");
  Serial.print("Infrarrojo: "); Serial.print(ij == LOW ? "STOP" : "GO AHEAD THE PATH IS CLEAR");
  Serial.println("----------------\n");
delay(2000);
}
void sensorUltrasonico(){
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long time = pulseIn(ECHO_PIN, HIGH);
  if (time == 0) {
    ut = 999.0; // Valor fuera de rango
  } 
  else {
    ut = time * 0.034 / 2.0; // Distancia en cm
  }
}
void sensorRgb(){
   uint16_t r, g, b, c;

  // Leer sensor
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {

    rb="Sin lectura";
    apagarLeds();
    return;
  }

  // Convertir a proporciones
  float rojo = (float)r / c;
  float verde = (float)g / c;
  float azul = (float)b / c;


  apagarLeds();

  // DETECTAR COLOR

  if (rojo > 0.50 && verde < 0.22 && azul >= 0.20) {

    rb="MAGENTA";

    digitalWrite(purple_pin, HIGH);
  }

  else if (rojo > 0.45 && verde > 0.25 && azul < 0.25) {

    rb="NARANJA";

    digitalWrite(orange_pin, HIGH);
  }

  else if (rojo > 0.35 && verde > 0.35 && azul < 0.25) {

    rb="AMARILLO";

    digitalWrite(yellow_pin, HIGH);
  }

  else if (azul > rojo && verde > rojo) {

    rb="CELESTE";

    digitalWrite(blue_pin, HIGH);
  }

  else {

    rb="NO IDENTIFICADO";
  }
}
void sensorInfrarrojo(){
  hasObstacle = digitalRead(ir_pin);
  ij=hasObstacle;

if(hasObstacle ==LOW){
  digitalWrite(led_pin,LOW);

}
else{
  digitalWrite(led_pin,HIGH);
  
 }
}







