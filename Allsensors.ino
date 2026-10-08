#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Arduino.h>

// -------------------- PINES --------------------

#define TRIGGER_PIN 9
#define ECHO_PIN 10

const int ir_pin = 6;
const int led_pin = 13;

// Pines de los LEDs
const int purple_pin = 2;
const int orange_pin = 4;
const int yellow_pin = 8;
const int blue_pin = 12;


// -------------------- CONFIGURACION WALL FOLLOWER --------------------

// Distancia provisional para considerar que existe una pared.
// Este valor se debe calibrar posteriormente con el robot.
const float DISTANCIA_PARED = 15.0;


// -------------------- VARIABLES DE SENSORES --------------------

float ut = 0;
String rb = "";
int ij = 0;
int hasObstacle = LOW;


// -------------------- SENSOR DE COLOR --------------------

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);


// -------------------- FUNCIONES DE APOYO --------------------

void apagarLeds() {

  digitalWrite(purple_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}


// -------------------- SENSOR ULTRASONICO --------------------

void sensorUltrasonico() {

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER_PIN, LOW);

  long time = pulseIn(ECHO_PIN, HIGH);

  if (time == 0) {

    ut = 999.0;

  }
  else {

    ut = time * 0.034 / 2.0;
  }
}


// -------------------- SENSOR INFRARROJO --------------------

void sensorInfrarrojo() {

  hasObstacle = digitalRead(ir_pin);

  ij = hasObstacle;

  if (hasObstacle == LOW) {

    digitalWrite(led_pin, LOW);

  }
  else {

    digitalWrite(led_pin, HIGH);
  }
}


// -------------------- SENSOR DE COLOR --------------------

void sensorRgb() {

  uint16_t r, g, b, c;

  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {

    rb = "SIN LECTURA";

    apagarLeds();

    return;
  }

  // Normalizacion de valores RGB

  float rojo = (float)r / c;
  float verde = (float)g / c;
  float azul = (float)b / c;

  apagarLeds();


  // ---------------- DETECCION DE COLOR ----------------

  if (rojo > 0.50 && verde < 0.22 && azul >= 0.20) {

    rb = "MAGENTA";

    digitalWrite(purple_pin, HIGH);
  }

  else if (rojo > 0.45 && verde > 0.25 && azul < 0.25) {

    rb = "NARANJA";

    digitalWrite(orange_pin, HIGH);
  }

  else if (rojo > 0.35 && verde > 0.35 && azul < 0.25) {

    rb = "AMARILLO";

    digitalWrite(yellow_pin, HIGH);
  }

  else if (azul > rojo && verde > rojo) {

    rb = "CELESTE";

    digitalWrite(blue_pin, HIGH);
  }

  else {

    rb = "NO IDENTIFICADO";
  }
}


// -------------------- WALL FOLLOWER --------------------

// Determina si el ultrasonico esta detectando una pared
bool hayPared(float distancia) {

  return distancia < DISTANCIA_PARED;
}


// Funcion principal de navegacion del Maze
void resolverMaze() {

  // Primero actualizamos todos los sensores

  sensorUltrasonico();

  sensorInfrarrojo();

  sensorRgb();


  // ---------------- INFORMACION DE SENSORES ----------------

  Serial.println("----- WALL FOLLOWER -----");


  Serial.print("Distancia ultrasonico: ");

  Serial.print(ut);

  Serial.println(" cm");


  Serial.print("Infrarrojo: ");

  if (ij == LOW) {

    Serial.println("OBSTACULO");

  }
  else {

    Serial.println("LIBRE");
  }


  Serial.print("Color detectado: ");

  Serial.println(rb);


  // ---------------- PRIMERA DECISION DEL ALGORITMO ----------------

  if (hayPared(ut)) {

    Serial.println("Pared detectada");

  }
  else {

    Serial.println("No se detecta pared");
  }


  Serial.println("-------------------------");

  Serial.println();
}


// -------------------- SETUP --------------------

void setup() {

  Serial.begin(9600);


  // Infrarrojo

  pinMode(ir_pin, INPUT);

  pinMode(led_pin, OUTPUT);


  // Ultrasonico

  pinMode(TRIGGER_PIN, OUTPUT);

  pinMode(ECHO_PIN, INPUT);


  // LEDs de colores

  pinMode(purple_pin, OUTPUT);

  pinMode(orange_pin, OUTPUT);

  pinMode(yellow_pin, OUTPUT);

  pinMode(blue_pin, OUTPUT);


  apagarLeds();


  // Inicializacion del sensor TCS34725

  Serial.println("Inicializando sensores...");


  if (!tcs.begin()) {

    Serial.println("No se encontro el sensor TCS34725");

    while (1);
  }


  Serial.println("Sensores listos");

  Serial.println();
}


// -------------------- LOOP PRINCIPAL --------------------

void loop() {

  resolverMaze();

  delay(500);
}
