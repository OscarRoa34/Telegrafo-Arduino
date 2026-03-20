/*
 * ============================================================================
 * PROYECTO: Telégrafo OSI
 * MÓDULO: Transceptor Bidireccional
 * DESCRIPCIÓN: Implementación de comunicación punto a punto, estructurada 
 * bajo el Modelo de Interconexión de Sistemas Abiertos (OSI).
 * 
 * AUTORES (EQUIPO DE DESARROLLO):
 * - David Aguilar
 * - Valeria Tocarruncho
 * - Oscar Roa
 * - Selene Daza
 * ============================================================================
 */

// ========================================================
// CAPA 1: FÍSICA (Hardware, Medio y Señales)
// ========================================================
const int PIN_BOTON  = 4;
const int PIN_TX     = 8;
const int PIN_RX     = 9;

// ========================================================
// CAPA 2: ENLACE DE DATOS (Control de Acceso al Medio)
// ========================================================
int estadoBoton = LOW;
int ultimoEstadoBoton = LOW;
unsigned long ultimoTiempoRebote = 0;
const unsigned long RETARDO_REBOTE = 50; 

int estadoAnteriorRX = LOW;

unsigned long tiempoInicioPulso = 0;
unsigned long duracionPulso = 0;

unsigned long tiempoUltimaSenal = 0;
unsigned long duracionSilencio = 0;

// Función de debouncing
int leerBotonDebounce() {
  int lectura = digitalRead(PIN_BOTON);

  if (lectura != ultimoEstadoBoton) {
    ultimoTiempoRebote = millis();
  }

  if ((millis() - ultimoTiempoRebote) > RETARDO_REBOTE) {
    if (lectura != estadoBoton) {
      estadoBoton = lectura;
    }
  }

  ultimoEstadoBoton = lectura;
  return estadoBoton;
}

// [PENDIENTE - Fase 3] Máquinas de estado para tramado de datos.

// ========================================================
// CAPAS 3, 4 y 5: RED, TRANSPORTE Y SESIÓN
// ========================================================
// [NOTA ARQUITECTÓNICA] Enlace físico dedicado (punto a punto). 
// Enrutamiento implícito. Control de flujo gestionado por protocolo humano.

// ========================================================
// CAPA 6: PRESENTACIÓN (Traducción y Codificación)
// ========================================================
// [PENDIENTE - Tarea 2.3] Lógica de traducción alfanumérica a secuencias Morse.
// Unidad base (ajustable experimentalmente)
const unsigned long TIEMPO_PUNTO = 200;

// Umbrales de decisión
const unsigned long UMBRAL_PUNTO_RAYA = TIEMPO_PUNTO * 2;
const unsigned long UMBRAL_FIN_LETRA = TIEMPO_PUNTO * 3;
const unsigned long UMBRAL_FIN_PALABRA = TIEMPO_PUNTO * 7;

// Variables simbólicas (resultado de la capa 6)
char simboloActual = '\0';   // '.', '-', ' ', '/'
bool haySimboloNuevo = false;
// ========================================================
// CAPA 7: APLICACIÓN (Interfaz Humano-Máquina)
// ========================================================
const int PIN_BUZZER = 12;
const int PIN_LED    = 13;

void setup() {
  pinMode(PIN_BOTON, INPUT);
  pinMode(PIN_RX, INPUT);
  
  pinMode(PIN_TX, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  
  digitalWrite(PIN_TX, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
}

void loop() {

// --------------------------------------------------------
// PROCESAMIENTO DE SALIDA (TX)
// --------------------------------------------------------
  int lecturaActual = digitalRead(PIN_BOTON);

  if (lecturaActual != ultimoEstadoBoton) {
    ultimoTiempoRebote = millis();
  }

  if ((millis() - ultimoTiempoRebote) > RETARDO_REBOTE) {
    if (lecturaActual != estadoBoton) {
      estadoBoton = lecturaActual; 
      digitalWrite(PIN_TX, estadoBoton); 
    }
  }
  ultimoEstadoBoton = lecturaActual; 


// --------------------------------------------------------
// PROCESAMIENTO DE ENTRADA (RX)
// --------------------------------------------------------

  int estadoActualRX = digitalRead(PIN_RX);

  if (estadoActualRX == HIGH && estadoAnteriorRX == LOW) {
    tiempoInicioPulso = millis();
  }

  if (estadoActualRX == LOW && estadoAnteriorRX == HIGH) {
    duracionPulso = millis() - tiempoInicioPulso;
    tiempoUltimaSenal = millis();

    // Señal lista para interpretación (Capa 6)
    if (duracionPulso < UMBRAL_PUNTO_RAYA) {
      simboloActual = '.';
    } else {
      simboloActual = '-';
    }
    haySimboloNuevo = true;
  }

  // Medición de silencios
  if (estadoActualRX == LOW) {
    duracionSilencio = millis() - tiempoUltimaSenal;

    if (duracionSilencio > UMBRAL_FIN_PALABRA) {
      simboloActual = '/';  // espacio palabra
      haySimboloNuevo = true;
      tiempoUltimaSenal = millis();
    } 
    else if (duracionSilencio > UMBRAL_FIN_LETRA) {
      simboloActual = ' ';  // espacio letra
      haySimboloNuevo = true;
      tiempoUltimaSenal = millis();
    }
  }

  estadoAnteriorRX = estadoActualRX;


// --------------------------------------------------------
// CAPA 7: APLICACIÓN (SALIDA)
// --------------------------------------------------------

  // 🔹 Feedback inmediato (como versión original)
  int senalEntrante = digitalRead(PIN_RX);

  if (estadoBoton == HIGH || senalEntrante == HIGH) {
    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_BUZZER, HIGH);
  } 
  else {
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }

  // 🔹 Eventos interpretados (se mantiene para lógica Morse)
  if (haySimboloNuevo) {
    haySimboloNuevo = false;
  }

}