
#include <Arduino.h>

// ======================================
// ROBOREGOS - PRUEBA DE MOTORES
// ======================================

// MOTOR IZQUIERDO
const int PWMA = 5;
const int AIN1 = A0;
const int AIN2 = A1;

// MOTOR DERECHO
const int PWMB = 6;
const int BIN1 = A2;
const int BIN2 = A3;

// DRIVER
const int STBY = 11;

// ======================================
// CONFIGURACION
// ======================================

const int VELOCIDAD = 190;

// Direcciones que teniamos antes
const bool INVERTIR_IZQUIERDO = true;
const bool INVERTIR_DERECHO = false;

// ======================================
// MOTORES
// ======================================

void moverMotores(int izquierda, int derecha) {

  izquierda = constrain(izquierda, -255, 255);
  derecha = constrain(derecha, -255, 255);

  if (INVERTIR_IZQUIERDO) {
    izquierda = -izquierda;
  }

  if (INVERTIR_DERECHO) {
    derecha = -derecha;
  }

  digitalWrite(AIN1, izquierda > 0 ? HIGH : LOW);
  digitalWrite(AIN2, izquierda < 0 ? HIGH : LOW);

  digitalWrite(BIN1, derecha > 0 ? HIGH : LOW);
  digitalWrite(BIN2, derecha < 0 ? HIGH : LOW);

  analogWrite(PWMA, abs(izquierda));
  analogWrite(PWMB, abs(derecha));
}

void detener() {
  moverMotores(0, 0);
}

void avanzar() {
  moverMotores(VELOCIDAD, VELOCIDAD);
}

void retroceder() {
  moverMotores(-VELOCIDAD, -VELOCIDAD);
}

void girarDerecha() {
  moverMotores(VELOCIDAD, -VELOCIDAD);
}

void girarIzquierda() {
  moverMotores(-VELOCIDAD, VELOCIDAD);
}

// ======================================
// SETUP
// ======================================

void setup() {

  Serial.begin(9600);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, LOW);
  detener();

  // Activar driver
  digitalWrite(STBY, HIGH);

  Serial.println("PRUEBA DE MOTORES");
  Serial.println("El robot avanzara una sola vez");

  // Esperar antes de iniciar
  delay(3000);

  // PRUEBA: AVANZAR
  Serial.println("AVANZANDO");

  avanzar();
  delay(700);

  detener();

  Serial.println("PRUEBA TERMINADA");
}

// ======================================
// LOOP
// ======================================

void loop() {

  detener();
}
