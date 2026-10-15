#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ================================================
// MOTORES: PUENTE H TB6612FNG
// ================================================
const int PWMA = 5;
const int AIN1 = A0;
const int AIN2 = A1;

const int PWMB = 6;
const int BIN1 = A2;
const int BIN2 = A3;

// Conectar STBY del TB6612FNG al pin D11
const int STBY = 11;

const int velocidad = 230;

// ================================================
// SENSOR ULTRASONICO
// ================================================
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 4;

const float DISTANCIA_PARED = 15.0;

// ================================================
// SENSOR INFRARROJO
// ================================================
const int ir_pin = 7;
const int led_pin = 8;

int lecturaIR = HIGH;

// ================================================
// LEDS INDICADORES DE COLOR
// ================================================
const int purple_pin = 9;
const int orange_pin = 10;
const int yellow_pin = 3;
const int blue_pin = 12;

// ================================================
// SENSOR RGB TCS34725
// SDA -> A4
// SCL -> A5
// ================================================
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

bool colorDisponible = false;

float distancia = -1.0;
float rojo = 0;
float verde = 0;
float azul = 0;

String colorDetectado = "SIN LECTURA";

// ================================================
// MOTORES
// ================================================
void detener() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}

void avanzar() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, velocidad);
  analogWrite(PWMB, velocidad);
}

void retroceder() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, velocidad);
  analogWrite(PWMB, velocidad);
}

void izquierda() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, velocidad);
  analogWrite(PWMB, velocidad);
}

void derecha() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, velocidad);
  analogWrite(PWMB, velocidad);
}

// ================================================
// SENSOR ULTRASONICO
// ================================================
void leerUltrasonico() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  unsigned long tiempo =
    pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (tiempo == 0) {
    distancia = -1.0;
  } else {
    distancia = tiempo * 0.0343 / 2.0;
  }
}

// ================================================
// SENSOR INFRARROJO
// Supone que LOW significa obstaculo detectado
// ================================================
void leerInfrarrojo() {
  lecturaIR = digitalRead(ir_pin);

  digitalWrite(
    led_pin,
    lecturaIR == LOW ? HIGH : LOW
  );
}

// ================================================
// LEDS DE COLOR
// ================================================
void apagarLeds() {
  digitalWrite(purple_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}

// ================================================
// SENSOR RGB
// ================================================
void leerColor() {
  if (!colorDisponible) {
    colorDetectado = "SENSOR NO DISPONIBLE";
    apagarLeds();
    return;
  }

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {
    colorDetectado = "SIN LECTURA";
    apagarLeds();
    return;
  }

  rojo = (float)r / c;
  verde = (float)g / c;
  azul = (float)b / c;

  apagarLeds();

  // Conservar los umbrales del segundo codigo
  if (rojo > 0.50 &&
      verde < 0.22 &&
      azul >= 0.20) {

    colorDetectado = "MAGENTA";
    digitalWrite(purple_pin, HIGH);
  }
  else if (rojo > 0.45 &&
           verde > 0.25 &&
           azul < 0.25) {

    colorDetectado = "NARANJA";
    digitalWrite(orange_pin, HIGH);
  }
  else if (rojo > 0.35 &&
           verde > 0.35 &&
           azul < 0.25) {

    colorDetectado = "AMARILLO";
    digitalWrite(yellow_pin, HIGH);
  }
  else if (azul > rojo &&
           verde > rojo) {

    colorDetectado = "CELESTE";
    digitalWrite(blue_pin, HIGH);
  }
  else {
    colorDetectado = "NO IDENTIFICADO";
  }
}

// ================================================
// ACTUALIZAR TODOS LOS SENSORES
// ================================================
void actualizarSensores() {
  leerUltrasonico();
  leerInfrarrojo();
  leerColor();
}

// ================================================
// REPORTE POR MONITOR SERIAL
// ================================================
void mostrarSensores() {
  Serial.println("\n===== SENSORES =====");

  Serial.print("Ultrasonico: ");

  if (distancia < 0) {
    Serial.println("SIN ECO VALIDO");
  } else {
    Serial.print(distancia);
    Serial.println(" cm");
  }

  Serial.print("Pared enfrente: ");
  Serial.println(
    distancia >= 0 && distancia < DISTANCIA_PARED
      ? "SI"
      : "NO / SIN ECO"
  );

  Serial.print("IR digital: ");
  Serial.println(lecturaIR);

  Serial.print("Obstaculo IR: ");
  Serial.println(
    lecturaIR == LOW ? "SI" : "NO"
  );

  Serial.print("Color: ");
  Serial.println(colorDetectado);

  Serial.print("RGB normalizado: ");
  Serial.print(rojo, 3);
  Serial.print(", ");
  Serial.print(verde, 3);
  Serial.print(", ");
  Serial.println(azul, 3);

  Serial.println("====================");
}

// ================================================
// SETUP
// ================================================
void setup() {
  Serial.begin(9600);

  // Motores
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);

  // Sensores
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ir_pin, INPUT);
  pinMode(led_pin, OUTPUT);

  // LEDs
  pinMode(purple_pin, OUTPUT);
  pinMode(orange_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

  digitalWrite(TRIGGER_PIN, LOW);
  digitalWrite(led_pin, LOW);

  detener();
  apagarLeds();

  // Sensor RGB
  Serial.println("Inicializando sensores...");

  colorDisponible = tcs.begin();

  if (colorDisponible) {
    Serial.println("Sensor RGB listo");
  } else {
    Serial.println("ERROR: No se encontro TCS34725");
  }

  Serial.println("Sistema integrado listo");
  Serial.println("Motores detenidos al iniciar");

  delay(1000);
}

// ================================================
// LOOP PRINCIPAL
// ================================================
void loop() {
  actualizarSensores();
  mostrarSensores();

  // SEGURIDAD 1:
  // Sin eco valido, no avanzar.
  if (distancia < 0) {
    detener();
    Serial.println("STOP: sin eco ultrasonico");
  }

  // SEGURIDAD 2:
  // Detener si hay una pared a menos de 15 cm.
  else if (distancia < DISTANCIA_PARED) {
    detener();
    Serial.println("STOP: pared enfrente");
  }

  // SEGURIDAD 3:
  // Detener si el IR detecta un obstaculo.
  else if (lecturaIR == LOW) {
    detener();
    Serial.println("STOP: obstaculo infrarrojo");
  }

  // AVANCE:
  // Ambos sensores deben permitir el avance.
  else {
    Serial.println("Camino despejado: avanzando");
    avanzar();
  }

  delay(200);
}
