/*
 * ============================================================================
 * PROYECTO: Telégrafo OSI
 * MÓDULO: Transceptor Bidireccional (Versión 3 Botones)
 * DESCRIPCIÓN: Implementación de comunicación punto a punto, estructurada 
 * bajo el Modelo de Interconexión de Sistemas Abiertos (OSI).
 * * AUTORES (EQUIPO DE DESARROLLO):
 * - David Aguilar
 * - Valeria Tocarruncho
 * - Oscar Roa
 * - Selene Daza
 * ============================================================================
 */

// ========================================================
// CAPA 1: FÍSICA (Hardware y Señales)
// ========================================================
const int PIN_BTN_PUNTO = 2;
const int PIN_BTN_RAYA  = 3;
const int PIN_BTN_ENVIA = 4;
const int PIN_TX      = 12;
const int PIN_RX      = 13;
const int PIN_BUZZER  = 8;
const int PIN_LED     = 9;
const int PIN_AVISO   = 10;

void transmitirVoltaje(int estado) { digitalWrite(PIN_TX, estado); }

// ========================================================
// CAPA 2: ENLACE DE DATOS (MAC y Tramado)
// ========================================================
int estadoBoton[3] = {LOW, LOW, LOW};
int ultimoEstadoBoton[3] = {LOW, LOW, LOW};
unsigned long ultimoTiempoRebote[3] = {0, 0, 0};
const unsigned long RETARDO_REBOTE = 50; 

bool leerClic(int indice, int pin) {
  bool clicDetectado = false;
  int lectura = digitalRead(pin);
  if (lectura != ultimoEstadoBoton[indice]) ultimoTiempoRebote[indice] = millis();
  if ((millis() - ultimoTiempoRebote[indice]) > RETARDO_REBOTE) {
    if (lectura != estadoBoton[indice]) {
      estadoBoton[indice] = lectura;
      if (estadoBoton[indice] == HIGH) clicDetectado = true;
    }
  }
  ultimoEstadoBoton[indice] = lectura;
  return clicDetectado;
}

// Tramado de Salida
void enviarPulso(int duracion) {
  transmitirVoltaje(HIGH); 
  digitalWrite(PIN_LED, HIGH); digitalWrite(PIN_BUZZER, HIGH); 
  delay(duracion);
  transmitirVoltaje(LOW);  
  digitalWrite(PIN_LED, LOW); digitalWrite(PIN_BUZZER, LOW);
}

void transmitirTrama(String payload, int tiempoBase) {
  // 1. INICIO (Pulso largo 5t - Solo avisa al RX)
  transmitirVoltaje(HIGH); digitalWrite(PIN_AVISO, HIGH);
  delay(tiempoBase * 5);
  transmitirVoltaje(LOW); digitalWrite(PIN_AVISO, LOW);
  delay(tiempoBase * 2); 

  // 2. PAYLOAD (Sonará sincronizado al otro lado)
  for (int i = 0; i < payload.length(); i++) {
    if (payload[i] == '.') enviarPulso(tiempoBase);     
    else if (payload[i] == '-') enviarPulso(tiempoBase * 3); 
    if (i < payload.length() - 1) delay(tiempoBase); 
  }
  // 3. PARADA (Ahora es un silencio natural que el RX detecta)
  // No enviamos voltaje extra para no ensuciar el zumbador del RX.
}

// ========================================================
// CAPA 4 (TRANSPORTE) y CAPA 5 (SESIÓN)
// ========================================================
const int TIEMPO_BASE = 300; 
bool esperandoACK = false;
unsigned long tiempoInicioEsperaACK = 0;
const int TIMEOUT_ACK = 3000; 

void enviarACK() {
  delay(150); 
  transmitirVoltaje(HIGH); delay(TIEMPO_BASE * 3.5); transmitirVoltaje(LOW);
}

void transmitirFinDeSesion() {
  transmitirVoltaje(HIGH); digitalWrite(PIN_AVISO, HIGH);
  delay(TIEMPO_BASE * 8); 
  transmitirVoltaje(LOW); digitalWrite(PIN_AVISO, LOW);
}

bool leerPresionLarga(int indice, int pin, int tiempoMinimo) {
    if (digitalRead(pin) == HIGH && estadoBoton[indice] == HIGH) {
        if ((millis() - ultimoTiempoRebote[indice]) > tiempoMinimo) {
            estadoBoton[indice] = LOW; return true;
        }
    }
    return false;
}

// ========================================================
// CAPA 6: PRESENTACIÓN (Traducción y Formato Interno)
// ========================================================
String bufferMorseTX = ""; 
String bufferMorseRX = "";
char ultimoCaracterRecibido = '?'; // Memoria RAM del equipo

const String tablaMorse[26] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..","--","-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--.."};
const char tablaASCII[26] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

void traducirCapa6() {
  ultimoCaracterRecibido = '?';
  for(int i = 0; i < 26; i++) {
    if(tablaMorse[i] == bufferMorseRX) {
      ultimoCaracterRecibido = tablaASCII[i];
      break;
    }
  }
  bufferMorseRX = ""; // Limpia para el siguiente
}

// ========================================================
// CAPA 7: APLICACIÓN (UI y Estado de Recepción)
// ========================================================
bool estadoAnteriorRX = LOW;
unsigned long tiempoInicioPulsoRX = 0;
unsigned long ultimoTiempoRX_LOW = 0;
bool tramaActiva = false; // "Switch" para activar el zumbador en tiempo real

void retroalimentacionCorta() {
  digitalWrite(PIN_BUZZER, HIGH); digitalWrite(PIN_LED, HIGH); delay(30);
  digitalWrite(PIN_BUZZER, LOW); digitalWrite(PIN_LED, LOW);
}

void setup() {
  pinMode(PIN_BTN_PUNTO, INPUT); pinMode(PIN_BTN_RAYA, INPUT); pinMode(PIN_BTN_ENVIA, INPUT);
  pinMode(PIN_RX, INPUT); pinMode(PIN_TX, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT); pinMode(PIN_LED, OUTPUT); pinMode(PIN_AVISO, OUTPUT);
  transmitirVoltaje(LOW);
  digitalWrite(PIN_BUZZER, LOW); digitalWrite(PIN_LED, LOW); digitalWrite(PIN_AVISO, LOW); 
}

void loop() {
  // --------------------------------------------------------
  // PROCESAMIENTO DE SALIDA (TX)
  // --------------------------------------------------------
  if (!esperandoACK) { 
    if (leerClic(0, PIN_BTN_PUNTO)) { bufferMorseTX += "."; retroalimentacionCorta(); }
    if (leerClic(1, PIN_BTN_RAYA))  { bufferMorseTX += "-"; retroalimentacionCorta(); }
    
    if (leerClic(2, PIN_BTN_ENVIA)) {
      if (bufferMorseTX.length() > 0) {
        transmitirTrama(bufferMorseTX, TIEMPO_BASE); 
        bufferMorseTX = ""; 
        esperandoACK = true; // L4: Bloquea esperando confirmación
        tiempoInicioEsperaACK = millis();
      }
    }
    if (leerPresionLarga(2, PIN_BTN_ENVIA, 1500)) { // L5: Fin de sesión
        transmitirFinDeSesion();
        retroalimentacionCorta(); delay(100); retroalimentacionCorta();
    }
  } else {
    if (millis() - tiempoInicioEsperaACK > TIMEOUT_ACK) { // L4: Timeout
      esperandoACK = false;
      digitalWrite(PIN_AVISO, HIGH); delay(100); digitalWrite(PIN_AVISO, LOW); 
    }
  }

  // --------------------------------------------------------
  // PROCESAMIENTO DE ENTRADA (RX) 
  // --------------------------------------------------------
  int senalEntrante = digitalRead(PIN_RX); 
  
  // Flanco de subida
  if (senalEntrante == HIGH && estadoAnteriorRX == LOW) {
    tiempoInicioPulsoRX = millis();
    estadoAnteriorRX = HIGH;
    
    // SINCRONIZACIÓN 
    if (tramaActiva) {
      digitalWrite(PIN_BUZZER, HIGH); digitalWrite(PIN_LED, HIGH);
    }
  } 
  // Flanco de bajada
  else if (senalEntrante == LOW && estadoAnteriorRX == HIGH) {
    unsigned long duracion = millis() - tiempoInicioPulsoRX;
    estadoAnteriorRX = LOW;
    ultimoTiempoRX_LOW = millis(); // Marca inicio del silencio
    
    digitalWrite(PIN_BUZZER, LOW); digitalWrite(PIN_LED, LOW);

    // Si NO estamos en trama, analizamos señales de control L
    if (!tramaActiva) {
        if (duracion >= (TIEMPO_BASE * 7)) {
            // FIN DE SESIÓN 
            digitalWrite(PIN_AVISO, HIGH); delay(200); digitalWrite(PIN_AVISO, LOW); delay(200);
            digitalWrite(PIN_AVISO, HIGH); delay(200); digitalWrite(PIN_AVISO, LOW);
        } 
        else if (duracion >= (TIEMPO_BASE * 4.5)) {
            // INICIO DE TRAMA
            tramaActiva = true;
            digitalWrite(PIN_AVISO, HIGH); delay(200); digitalWrite(PIN_AVISO, LOW);
        } 
        else if (esperandoACK && duracion >= (TIEMPO_BASE * 2.5)) {
            // ACK RECIBIDO 
            esperandoACK = false; 
            digitalWrite(PIN_AVISO, HIGH); delay(300); digitalWrite(PIN_AVISO, LOW);
        }
    } 
    // Si ESTAMOS en trama, guardamos los pulsos para la Capa 6
    else {
        if (duracion > (TIEMPO_BASE * 2)) bufferMorseRX += "-";
        else if (duracion > 20) bufferMorseRX += ".";
    }
  }

  if (tramaActiva && senalEntrante == LOW) {
      if (millis() - ultimoTiempoRX_LOW > (TIEMPO_BASE * 3)) {
         tramaActiva = false; // Cierra la trama

         digitalWrite(PIN_AVISO, HIGH);
         delay(300); 
         digitalWrite(PIN_AVISO, LOW);
         
         // Ejecuta traducción interna a memoria
         traducirCapa6();
         
         // Emite Acuse de Recibo Automático
         enviarACK(); 
      }
  }
}