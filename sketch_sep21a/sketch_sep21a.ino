#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Arduino.h>


#define TRIGGER_PIN 9
#define ECHO_PIN 10

const int ir_pin = 6;
const int led_pin = 13;

const int purple_pin = 2;
const int orange_pin = 4;
const int yellow_pin = 8;
const int blue_pin = 12;




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


float leerUltrasonico() {

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER_PIN, LOW);

  long tiempo = pulseIn(ECHO_PIN, HIGH);

  if (tiempo == 0) {
    return 999.0;
  }

  float distancia = tiempo * 0.034 / 2.0;

  return distancia;
}


bool leerInfrarrojo() {

  int estado = digitalRead(ir_pin);

  if (estado == LOW) {
    digitalWrite(led_pin, LOW);
    return true;   // Hay obstáculo
  }

  digitalWrite(led_pin, HIGH);
  return false;    // Camino libre
}


String detectarColor() {

  uint16_t r, g, b, c;

  tcs.getRawData(&r, &g, &b, &c);

  apagarLeds();

  if (c == 0) {
    return "SIN LECTURA";
  }

  float rojo = (float)r / c;
  float verde = (float)g / c;
  float azul = (float)b / c;


  if (rojo > 0.50 && verde < 0.22 && azul >= 0.20) {

    digitalWrite(purple_pin, HIGH);

    return "MAGENTA";
  }


  else if (rojo > 0.45 && verde > 0.25 && azul < 0.25) {

    digitalWrite(orange_pin, HIGH);

    return "NARANJA";
  }


  else if (rojo > 0.35 && verde > 0.35 && azul < 0.25) {

    digitalWrite(yellow_pin, HIGH);

    return "AMARILLO";
  }


  else if (azul > rojo && verde > rojo) {

    digitalWrite(blue_pin, HIGH);

    return "CELESTE";
  }


  return "NO IDENTIFICADO";
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


  if (!tcs.begin()) {

    Serial.println("No se encontro el sensor TCS34725");

    while (1);
  }

  Serial.println("Sensores listos");
}



void loop() {

  float distanciaFrente = leerUltrasonico();

  bool obstaculoIR = leerInfrarrojo();

  String color = detectarColor();


  Serial.println("--- RESUMEN ---");

  Serial.print("Distancia: ");
  Serial.print(distanciaFrente);
  Serial.println(" cm");

  Serial.print("Infrarrojo: ");
  Serial.println(obstaculoIR ? "OBSTACULO" : "LIBRE");

  Serial.print("Color: ");
  Serial.println(color);

  Serial.println();

  delay(500);
}