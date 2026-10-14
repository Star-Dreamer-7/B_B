#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ===== MOTORES =====
const int PWMA = 5;
const int AIN1 = 3;
const int AIN2 = 7;

const int PWMB = 11;
const int BIN1 = A0;
const int BIN2 = A1;

// IMPORTANTE: cambiar por el pin real de STBY
const int STBY = -1;

const int velocidad = 230;

// ===== ULTRASONICO =====
const int TRIGGER_PIN = 9;
const int ECHO_PIN = 10;
const float DISTANCIA_PARED = 15.0;

// ===== INFRARROJO =====
const int ir_pin = 6;
const int led_pin = 13;

// ===== LEDS DE COLORES =====
const int purple_pin = 2;
const int orange_pin = 4;
const int yellow_pin = 8;
const int blue_pin = 12;

// ===== SENSOR DE COLOR =====
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

bool colorDisponible = false;

// ===== VARIABLES =====
float distancia = -1;
int lecturaIR = HIGH;

float rojo = 0;
float verde = 0;
float azul = 0;

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

// ===== ULTRASONICO =====
void leerUltrasonico() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  unsigned long tiempo = pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (tiempo == 0) {
    distancia = -1;
  } else {
    distancia = tiempo * 0.0343 / 2.0;
  }
}

// ===== INFRARROJO =====
void leerInfrarrojo() {
  lecturaIR = digitalRead(ir_pin);
  digitalWrite(led_pin, lecturaIR == LOW ? HIGH : LOW);
}

// ===== LEDS =====
void apagarLeds() {
  digitalWrite(purple_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}

// ===== SENSOR RGB =====
void leerColor() {
  if (!colorDisponible) return;

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {
    Serial.println("Color: SIN LECTURA");
    return;
  }

  rojo = (float)r / c;
  verde = (float)g / c;
  azul = (float)b / c;

  Serial.print("RGB -> R: ");
  Serial.print(rojo, 3);

  Serial.print(" | G: ");
  Serial.print(verde, 3);

  Serial.print(" | B: ");
  Serial.println(azul, 3);
}

// ===== ACTUALIZAR SENSORES =====
void actualizarSensores() {
  leerUltrasonico();
  leerInfrarrojo();
  leerColor();
}

// ===== MOSTRAR LECTURAS =====
void mostrarSensores() {
  Serial.println("===== SENSORES =====");

  if (distancia < 0) {
    Serial.println("Ultrasonico: SIN ECO VALIDO");
  } else {
    Serial.print("Ultrasonico: ");
    Serial.print(distancia);
    Serial.println(" cm");

    Serial.print("Pared enfrente: ");
    Serial.println(
      distancia < DISTANCIA_PARED ? "SI" : "NO"
    );
  }

  Serial.print("IR digital: ");
  Serial.println(lecturaIR);

  Serial.print("Pared derecha: ");
  Serial.println(lecturaIR == LOW ? "SI" : "NO");

  Serial.println("====================");
}

// ===== PRUEBA DE MOVIMIENTOS =====
// No es todavía el algoritmo del laberinto.
void probarMovimiento() {
  detener();
  actualizarSensores();
  mostrarSensores();

  // No mover si no hay lectura válida del ultrasónico
  if (distancia < 0) {
    Serial.println("Movimiento cancelado: sin eco");
    return;
  }

  // No avanzar si hay un obstáculo cerca
  if (distancia < DISTANCIA_PARED) {
    Serial.println("Movimiento cancelado: pared enfrente");
    return;
  }

  Serial.println("Prueba: avanzar");
  avanzar();
  delay(600);
  detener();

  delay(100);
}

// ===== SETUP =====
void setup() {
  Serial.begin(9600);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  if (STBY >= 0) {
    pinMode(STBY, OUTPUT);
    digitalWrite(STBY, HIGH);
  }

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ir_pin, INPUT);
  pinMode(led_pin, OUTPUT);

  pinMode(purple_pin, OUTPUT);
  pinMode(orange_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

  detener();
  apagarLeds();

  colorDisponible = tcs.begin();

  if (!colorDisponible) {
    Serial.println("No se encontro TCS34725");
  }

  Serial.println("Sistema integrado listo");
  delay(2000);
}

// ===== LOOP =====
void loop() {
  probarMovimiento();
  delay(50);
}
