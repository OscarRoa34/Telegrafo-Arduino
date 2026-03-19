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
const int PIN_BOTON  = 2;
const int PIN_TX     = 8;
const int PIN_RX     = 9;

// ========================================================
// CAPA 2: ENLACE DE DATOS (Control de Acceso al Medio)
// ========================================================
int estadoBoton = LOW;
int ultimoEstadoBoton = LOW;
unsigned long ultimoTiempoRebote = 0;
const unsigned long RETARDO_REBOTE = 50; 

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

// [PENDIENTE - Tarea 2.3] Parámetros de temporización para el protocolo Morse.
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
  int botonEstable = leerBotonDebounce(); // Capa 2
  digitalWrite(PIN_TX, botonEstable);     // Capa 1

  // --------------------------------------------------------
  // PROCESAMIENTO DE ENTRADA (RX)
  // --------------------------------------------------------
  int senalEntrante = digitalRead(PIN_RX);
  
  if (senalEntrante == HIGH || botonEstable == HIGH) {
    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_BUZZER, HIGH);
  } else {
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }
}