
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <math.h>

// ========================================
// ROBOREGOS - PISTA B
// 2 IR FRONTALES + ULTRASONICO + MEMORIA
// ========================================

// ---------- MOTORES TB6612FNG ----------
const int PWMA = 5;
const int AIN1 = A0;
const int AIN2 = A1;

const int PWMB = 6;
const int BIN1 = A2;
const int BIN2 = A3;

const int STBY = 11;

// ---------- SENSORES ----------
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 4;

// Ambos IR miran hacia enfrente (mismos pines que Pista A)
const int IR_DERECHO = 7;
const int IR_IZQUIERDO = 10;

// 0 = pared, 1 = libre (confirmado en pruebas)
const int IR_PARED = LOW;

// ---------- LEDS ----------
const int pink_pin = 9;
const int orange_pin = 8;
const int yellow_pin = 3;
const int blue_pin = 12;

// ---------- SENSOR RGB ----------
Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// ========================================
// CONFIGURACION
// ========================================

const int VELOCIDAD = 210;
const int VELOCIDAD_GIRO = 220;
const int VELOCIDAD_RETROCESO = 160;

const bool INVERTIR_IZQUIERDO = true;
const bool INVERTIR_DERECHO = true;

// Ajusta segun pruebas fisicas
const unsigned long TIEMPO_CASILLA = 4000;
const unsigned long TIEMPO_GIRO_90 = 2000;
const unsigned long TIEMPO_RETROCESO = 300;

// Escape de esquinas / atasco
const unsigned long TIEMPO_RETROCESO_ATASCO = 500;
const unsigned long VENTANA_ATASCO = 700;   // cada cuanto revisa si avanzo
const float AVANCE_MINIMO_CM = 1.0;         // menos que esto = no se mueve
const int LIMITE_FALLOS = 2;                // intentos fallidos antes de escapar

// Distancias del ultrasonico (cm)
const float DISTANCIA_MUY_CERCA = 6.0;   // retrocede
const float DISTANCIA_PARED = 10.0;      // se detiene
const float DISTANCIA_LIBRE = 14.0;      // camino libre

const unsigned long INTERVALO_ULTRASONICO = 60;
const unsigned long INTERVALO_RGB = 500;

// ========================================
// MEMORIA
// ========================================

const int MAPA_SIZE = 15;

byte visitadas[MAPA_SIZE][MAPA_SIZE];

int posX = MAPA_SIZE / 2;
int posY = MAPA_SIZE / 2;

// 0 = NORTE, 1 = ESTE, 2 = SUR, 3 = OESTE
int orientacion = 0;

const int dx[4] = {0, 1, 0, -1};
const int dy[4] = {-1, 0, 1, 0};

// ========================================
// VARIABLES
// ========================================

float distancia = 200;
float ultimaDistanciaValida = 200;

bool colorDisponible = false;

unsigned long ultimaLecturaRGB = 0;

int fallosSeguidos = 0;
bool atascado = false;
bool escapeDerecha = true;   // alterna el lado de escape

// ========================================
// MOTORES
// ========================================

void moverMotores(int izquierda, int derecha) {
  izquierda = constrain(izquierda, -255, 255);
  derecha = constrain(derecha, -255, 255);

  if (INVERTIR_IZQUIERDO) izquierda = -izquierda;
  if (INVERTIR_DERECHO) derecha = -derecha;

  digitalWrite(AIN1, izquierda > 0 ? HIGH : LOW);
  digitalWrite(AIN2, izquierda < 0 ? HIGH : LOW);

  digitalWrite(BIN1, derecha > 0 ? HIGH : LOW);
  digitalWrite(BIN2, derecha < 0 ? HIGH : LOW);

  analogWrite(PWMA, abs(izquierda));
  analogWrite(PWMB, abs(derecha));
}

void detener() { moverMotores(0, 0); }
void avanzar() { moverMotores(VELOCIDAD, VELOCIDAD); }
void retroceder() { moverMotores(-VELOCIDAD_RETROCESO, -VELOCIDAD_RETROCESO); }
void girarDerecha() { moverMotores(VELOCIDAD_GIRO, -VELOCIDAD_GIRO); }
void girarIzquierda() { moverMotores(-VELOCIDAD_GIRO, VELOCIDAD_GIRO); }

// ========================================
// SENSORES
// ========================================

void leerUltrasonico() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  unsigned long tiempo = pulseIn(ECHO_PIN, HIGH, 25000UL);

  if (tiempo == 0) {
    // Sin eco: si la ultima lectura valida era cercana,
    // asumimos pared pegada al sensor; si no, espacio abierto.
    if (ultimaDistanciaValida <= 30.0) {
      distancia = 1.0;
    } else {
      distancia = 200.0;
    }
  } else {
    distancia = tiempo * 0.0343 / 2.0;
    ultimaDistanciaValida = distancia;
  }
}

bool paredDerecha() {
  return digitalRead(IR_DERECHO) == IR_PARED;
}

bool paredIzquierda() {
  return digitalRead(IR_IZQUIERDO) == IR_PARED;
}

bool obstaculoIR() {
  return paredDerecha() || paredIzquierda();
}

bool caminoLibre() {
  leerUltrasonico();

  if (distancia < DISTANCIA_LIBRE) return false;
  if (obstaculoIR()) return false;

  return true;
}

// ========================================
// LEDS Y SENSOR RGB
// ========================================

void apagarLeds() {
  digitalWrite(pink_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}

void leerColor() {
  if (!colorDisponible) {
    apagarLeds();
    return;
  }

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {
    apagarLeds();
    return;
  }

  float rojo = (float)r / c;
  float verde = (float)g / c;
  float azul = (float)b / c;

  apagarLeds();

  if (rojo > verde * 1.20 &&
      rojo > azul * 1.10 &&
      azul > verde * 0.85) {
    digitalWrite(pink_pin, HIGH);
  }
  else if (rojo > azul * 1.35 &&
           verde > azul * 1.35 &&
           fabs(rojo - verde) < 0.20 &&
           verde > 0.20) {
    digitalWrite(yellow_pin, HIGH);
  }
  else if (rojo > verde * 1.30 &&
           verde > azul * 1.15 &&
           verde > 0.15) {
    digitalWrite(orange_pin, HIGH);
  }
  else if (azul > rojo * 1.20 &&
           verde > rojo * 1.10) {
    digitalWrite(blue_pin, HIGH);
  }
}

void actualizarRGB() {
  if (millis() - ultimaLecturaRGB >= INTERVALO_RGB) {
    ultimaLecturaRGB = millis();
    leerColor();
  }
}

// ========================================
// MEMORIA DE CASILLAS
// ========================================

bool posicionValida(int x, int y) {
  return x >= 0 && x < MAPA_SIZE &&
         y >= 0 && y < MAPA_SIZE;
}

byte vecesVisitada(int direccion) {
  int nx = posX + dx[direccion];
  int ny = posY + dy[direccion];

  if (!posicionValida(nx, ny)) return 255;

  return visitadas[ny][nx];
}

void registrarCasilla() {
  if (!posicionValida(posX, posY)) return;

  if (visitadas[posY][posX] < 254) {
    visitadas[posY][posX]++;
  }

  Serial.print("POSICION: ");
  Serial.print(posX);
  Serial.print(",");
  Serial.println(posY);
}

void actualizarPosicion() {
  int nx = posX + dx[orientacion];
  int ny = posY + dy[orientacion];

  if (posicionValida(nx, ny)) {
    posX = nx;
    posY = ny;
    registrarCasilla();
  } else {
    Serial.println("LIMITE DEL MAPA");
  }
}

// ========================================
// GIROS
// ========================================

void girar90Derecha() {
  girarDerecha();
  delay(TIEMPO_GIRO_90);
  detener();
  delay(100);

  orientacion = (orientacion + 1) % 4;
}

void girar90Izquierda() {
  girarIzquierda();
  delay(TIEMPO_GIRO_90);
  detener();
  delay(100);

  orientacion = (orientacion + 3) % 4;
}

void girar180() {
  girar90Derecha();
  girar90Derecha();
}

// ========================================
// RETROCESO
// ========================================

void retrocederDePared() {
  detener();
  delay(100);

  Serial.println("RETROCEDIENDO");

  // El robot NO tiene sensor trasero: retroceso corto
  retroceder();
  delay(TIEMPO_RETROCESO);

  detener();
  delay(150);
}

// ========================================
// AVANZAR UNA CASILLA
// ========================================

// Si se interrumpe despues de media casilla,
// cuenta como casilla recorrida para no desfasar el mapa.
void cerrarAvance(unsigned long transcurrido) {
  if (transcurrido >= TIEMPO_CASILLA / 2) {
    actualizarPosicion();
    fallosSeguidos = 0;
  }
}

bool avanzarCasilla() {
  unsigned long inicio = millis();
  unsigned long ultimaRevision = 0;
  unsigned long inicioVentana = inicio;
  float distanciaVentana = -1;
  int ventanasSinAvance = 0;

  atascado = false;
  avanzar();

  while (millis() - inicio < TIEMPO_CASILLA) {
    unsigned long ahora = millis();

    actualizarRGB();

    // Cualquiera de los dos IR (frontales) detiene el avance
    if (obstaculoIR()) {
      detener();
      Serial.println("OBSTACULO IR");
      cerrarAvance(millis() - inicio);
      return false;
    }

    if (ahora - ultimaRevision >= INTERVALO_ULTRASONICO) {
      ultimaRevision = ahora;
      leerUltrasonico();

      if (distancia <= DISTANCIA_PARED) {
        detener();
        Serial.println("OBSTACULO ULTRASONICO");
        cerrarAvance(millis() - inicio);
        return false;
      }

      // Deteccion de atasco: si avanza pero la distancia
      // al frente no cambia, las ruedas estan empujando una pared.
      // Se ignora el arranque (500 ms) y se exigen 2 ventanas
      // seguidas sin avance para evitar falsos positivos.
      if (ahora - inicio >= 500) {
        if (distanciaVentana < 0) {
          distanciaVentana = distancia;
          inicioVentana = ahora;
        } else if (ahora - inicioVentana >= VENTANA_ATASCO) {
          if (distancia < 100.0 &&
              fabs(distanciaVentana - distancia) < AVANCE_MINIMO_CM) {
            ventanasSinAvance++;
          } else {
            ventanasSinAvance = 0;
          }

          if (ventanasSinAvance >= 2) {
            detener();
            atascado = true;
            Serial.println("ATASCO DETECTADO");
            return false;
          }

          distanciaVentana = distancia;
          inicioVentana = ahora;
        }
      }
    }
  }

  detener();
  actualizarPosicion();
  fallosSeguidos = 0;

  return true;
}

// ========================================
// BUSCAR SALIDA
// ========================================

// Gira 90 hacia el lado indicado y revisa;
// luego el lado contrario; por ultimo atras.
// Al terminar sin salida, el robot queda mirando hacia atras.
bool buscarSalida(bool derechaPrimero) {
  Serial.println("BUSCANDO SALIDA");

  // 1) Lado preferido
  if (derechaPrimero) girar90Derecha();
  else girar90Izquierda();

  if (caminoLibre()) {
    Serial.println("SALIDA LADO 1");
    return true;
  }

  // 2) Lado contrario (180 desde el lado 1)
  if (derechaPrimero) {
    girar90Izquierda();
    girar90Izquierda();
  } else {
    girar90Derecha();
    girar90Derecha();
  }

  if (caminoLibre()) {
    Serial.println("SALIDA LADO 2");
    return true;
  }

  // 3) Atras
  if (derechaPrimero) girar90Izquierda();
  else girar90Derecha();

  if (caminoLibre()) {
    Serial.println("SALIDA ATRAS");
    return true;
  }

  Serial.println("SIN SALIDA");
  return false;
}

// ========================================
// NAVEGACION PISTA B
// ========================================

// Maniobra ciega: retroceso largo y giros alternando de lado
void escaparDeAtasco() {
  Serial.println("ESCAPANDO DE ESQUINA");

  detener();
  delay(100);

  retroceder();
  delay(TIEMPO_RETROCESO_ATASCO);
  detener();
  delay(150);

  bool derecha = escapeDerecha;
  escapeDerecha = !escapeDerecha;

  // Hasta 3 giros de 90 buscando camino libre
  for (int i = 0; i < 3; i++) {
    if (derecha) girar90Derecha();
    else girar90Izquierda();

    if (caminoLibre()) break;
  }

  atascado = false;
  fallosSeguidos = 0;

  if (caminoLibre()) {
    avanzarCasilla();
  }
}

void explorarPistaB() {
  actualizarRGB();

  if (atascado || fallosSeguidos >= LIMITE_FALLOS) {
    escaparDeAtasco();
    return;
  }

  leerUltrasonico();

  bool irI = paredIzquierda();
  bool irD = paredDerecha();

  bool frenteLibre =
    distancia >= DISTANCIA_LIBRE && !irI && !irD;

  // Camino libre y casilla nueva: avanzar
  if (frenteLibre && vecesVisitada(orientacion) == 0) {
    Serial.println("AVANZANDO");
    avanzarCasilla();
    return;
  }

  detener();

  // Decidir lado de giro ANTES de retroceder/girar:
  // pared izquierda -> derecha; pared derecha -> izquierda;
  // si no hay IR, ir hacia la casilla menos visitada.
  bool derechaPrimero;

  if (irI && !irD) {
    derechaPrimero = true;
  } else if (irD && !irI) {
    derechaPrimero = false;
  } else {
    byte vDer = vecesVisitada((orientacion + 1) % 4);
    byte vIzq = vecesVisitada((orientacion + 3) % 4);
    derechaPrimero = (vDer < vIzq);
  }

  // Retroceder si esta muy cerca
  bool muyCerca =
    irI || irD || distancia <= DISTANCIA_MUY_CERCA;

  if (muyCerca) {
    retrocederDePared();
  }

  if (buscarSalida(derechaPrimero)) {
    avanzarCasilla();
  } else if (frenteLibre) {
    // Todo bloqueado menos el frente (ya visitado): regresar a el
    girar180();
    avanzarCasilla();
  } else {
    fallosSeguidos++;
    detener();
    delay(200);
  }
}

// ========================================
// SETUP
// ========================================

void setup() {
  Serial.begin(9600);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(IR_DERECHO, INPUT);
  pinMode(IR_IZQUIERDO, INPUT);

  pinMode(pink_pin, OUTPUT);
  pinMode(orange_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

  digitalWrite(TRIGGER_PIN, LOW);
  digitalWrite(STBY, LOW);

  detener();
  apagarLeds();

  for (int y = 0; y < MAPA_SIZE; y++) {
    for (int x = 0; x < MAPA_SIZE; x++) {
      visitadas[y][x] = 0;
    }
  }

  registrarCasilla();

  Wire.begin();
  colorDisponible = tcs.begin();

  Serial.println(colorDisponible ? "RGB LISTO" : "RGB NO DETECTADO");

  digitalWrite(STBY, HIGH);

  Serial.println("PISTA B - 2 IR FRONTALES");
  Serial.println("INICIANDO EN 3 SEGUNDOS");

  delay(3000);
}

// ========================================
// LOOP
// ========================================

void loop() {
  explorarPistaB();
}
