/**
 * @file main.cpp
 * @author Giacomo Simonetto
 * @brief Firmware per incubatrice neonatale
 * @version 0.1
 * @date 26-07-2026
 *
 * @copyright Copyright (c) 2026
 *
 * Prima implementazione di un semplice controllore ad isteresi del firmware
 * per l'incubatrice neonatale con incluse le funzionalità del pannello
 * di controllo frontale.
 */

// --- dipendenze ---
#include "config.h"
#include "actuators.h"
#include "controller.h"
#include "serial.h"
#include "lcd.h"

// --- variabili globali temporanee ---
int pot_read = 0;       // valore appena letto dai potenziometri
float new_setpoint = 0; // setpoint appena calcolato


// --- helper functions ---
/**
 * @brief Verifica se c'è un errore nel sensore SHT20, converte il codice
 * di errore in una stringa leggibile e stampa il messaggio su serial monitor
 * e display lcd.
 *
 * @param errorCode codice di errore restituito dal sensore SHT20
 * @return true se è stato generato un errore, false se non ci sono errori
 */
bool sht20_error(int errorCode);


// --- setup function ---
void setup() {
  // inizializzazione seriale e I2C
  Serial.begin(115200);

  // inizializzazione protocollo Wire per I2C
  Wire.begin();

  // inizializzazione display lcd e stampa messaggio di avvio
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // messaggio di avvio su serial monitor e display lcd
  serial_boot_message();
  lcd_boot_message();

  // inizializzazione sensore SHT20
  SHT20.begin();
  if (sht20_error(SHT20.getError())){
    digitalWrite(ALARM_LED, HIGH);
    while(1);
  }

  // inizializzazione pin
  pinMode(ALARM_LED, OUTPUT);
  pinMode(HEAT_LED, OUTPUT);
  pinMode(RH_LED, OUTPUT);
  pinMode(REFILL_LED, OUTPUT);

  pinMode(HEAT_RELAY, OUTPUT);
  pinMode(RH_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LIGHT_RELAY, OUTPUT);

  pinMode(HEAT_SW, INPUT);
  pinMode(RH_SW, INPUT);
  pinMode(LIGHT_SW, INPUT);

  // inizializzazione stato iniziale dei pin con led, relè
  digitalWrite(ALARM_LED, LOW);
  digitalWrite(HEAT_LED, LOW);
  digitalWrite(RH_LED, LOW);
  digitalWrite(REFILL_LED, LOW);

  digitalWrite(HEAT_RELAY, HIGH);
  digitalWrite(RH_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(LIGHT_RELAY, HIGH);

  // inizializzazione stato iniziale delle variabili globali
  memset(&status, 0, sizeof(status));
  memset(&timers, 0, sizeof(timers));
  memset(&temp_pid, 0, sizeof(temp_pid));
  memset(&rh_pid, 0, sizeof(rh_pid));

  // inizializzazione controllori PID
  ctrl_begin();

  delay(2000);
}

// --- loop function ---
void loop() {
  // --- cambio stato switch riscaldatore ---
  if (status.heat_sw != digitalRead(HEAT_SW)) {
    status.heat_sw = !status.heat_sw; // aggiornamento stato switch
    status.manual_ctrl = true; // abilitazione controllo manuale

    // switch riscaldatore attivo
    if (status.heat_sw) {
      heat_turn_on(); // accensione riscaldatore e led associato
      lcd_man_heat_on(); // visualizzazione accensione riscaldatore sul display
      serial_man_heat_on(); // visualizzazione accensione riscaldatore su serial monitor
    }

    // switch riscaldatore spento
    else {
      heat_turn_off(); // spegnimento riscaldatore e led associato
      lcd_man_heat_off(); // visualizzazione spegnimento riscaldatore sul display
      serial_man_heat_off(); // visualizzazione spegnimento riscaldatore su serial monitor
    }
  }

  // --- cambio stato switch umidificatore ---
  if (status.rh_sw != digitalRead(RH_SW)) {
    status.rh_sw = !status.rh_sw; // aggiornamento stato switch
    status.manual_ctrl = true; // abilitazione controllo manuale

    // avviso di refill acqua
    if (status.refill_led == true && status.rh_sw == true) {
      lcd_refill_message(); // visualizzazione messaggio refill sul display
    }

    // switch umidificatore attivo
    else if (!status.refill_led && status.rh_sw == true) {
      rh_turn_on(); // accensione umidificatore e led associato
      lcd_man_rh_on(); // visualizzazione accensione umidificatore sul display
      serial_man_rh_on(); // visualizzazione accensione umidificatore su serial monitor
    }

    // switch umidificatore spento
    else if (!status.refill_led && status.rh_sw == false) {
      rh_turn_off(); // spegnimento umidificatore e led associato
      lcd_man_rh_off(); // visualizzazione spegnimento umidificatore sul display
      serial_man_rh_off(); // visualizzazione spegnimento umidificatore su serial monitor
    }
  }

  // --- cambio stato switch illuminazione ---
  if (status.light_sw != digitalRead(LIGHT_SW)) {
    status.light_sw = !status.light_sw; // aggiornamento stato switch

    // switch illuminazione attivo
    if (status.light_sw) {
      light_turn_on(); // accensione illuminazione
      lcd_man_light_on(); // visualizzazione accensione illuminazione sul display
      serial_man_light_on(); // visualizzazione accensione illuminazione su serial monitor
    }
    // switch illuminazione spento
    else {
      light_turn_off(); // spegnimento illuminazione
      lcd_man_light_off(); // visualizzazione spegnimento illuminazione sul display
      serial_man_light_off(); // visualizzazione spegnimento illuminazione su serial monitor
    }
  }

  /**
   * @brief Aggiornamento del setpoint di temperatura.
   *
   * Il processo di aggiornamento del setpoint di temperatura avviene nelle
   * seguenti fasi:
   *
   * 1. lettura del potenziometro ogni TEMP_POT_TIMER millisecondi
   * 2. controllo di soglia per eliminare disturbi e rumore elettrico
   * 3. calcolo del nuovo setpoint di temperatura
   * 4. aggiornamento del setpoint se il nuovo valore è diverso dal precedente
   * 5. visualizzazione del nuovo setpoint sul display lcd e su serial monitor
   */

  // controllo timer per lettura potenziometro temperatura
  if (millis() - timers.temp_pot >= TEMP_POT_TIMER) {
    pot_read = analogRead(TEMP_POT); // lettura potenziometro temperatura
    timers.temp_pot = millis();  // aggiornamento timer

    // controllo threshold
    if (abs(status.temp_pot_value - pot_read) >= TEMP_POT_THLD) {
      status.temp_pot_value = pot_read; // aggiornamento valore potenziometro temperatura

      // calcolo nuovo setpoint di temperatura
      new_setpoint = map_16bit(pot_read, TEMP_POT_MIN, TEMP_POT_MAX, TEMP_MIN / TEMP_STEP, TEMP_MAX / TEMP_STEP) * TEMP_STEP;
      if (new_setpoint < TEMP_MIN) new_setpoint = TEMP_MIN; // controllo limite inferiore
      if (new_setpoint > TEMP_MAX) new_setpoint = TEMP_MAX; // controllo limite superiore

      // controllo variazioni del setpoint di temperatura
      if (status.temp_setpoint != new_setpoint) {
        lcd_new_temp_setpoint(); // stampa su display lcd
        serial_new_temp_setpoint(); // stampa su serial monitor
        status.temp_setpoint = new_setpoint; // aggiornamento setpoint precedente
        status.manual_ctrl = false; // disabilitazione controllo manuale
      }
    }
  }

  /**
   * @brief Aggiornamento del setpoint di temperatura.
   *
   * Il processo di aggiornamento del setpoint di temperatura avviene nelle
   * seguenti fasi:
   *
   * 1. lettura del potenziometro ogni TEMP_POT_TIMER millisecondi
   * 2. controllo di soglia per eliminare disturbi e rumore elettrico
   * 3. calcolo del nuovo setpoint di temperatura
   * 4. aggiornamento del setpoint se il nuovo valore è diverso dal precedente
   * 5. visualizzazione del nuovo setpoint sul display lcd e su serial monitor
   */

  // controllo timer per lettura potenziometro umidità
  if (millis() - timers.rh_pot >= RH_POT_TIMER) {
    pot_read = analogRead(RH_POT); // lettura potenziometro umidità
    timers.rh_pot = millis(); // aggiornamento timer

    // controllo threshold
    if (abs(status.rh_pot_value - pot_read) >= RH_POT_THLD) {
      status.rh_pot_value = pot_read; // aggiornamento valore potenziometro umidità

      // calcolo nuovo setpoint di umidità
      new_setpoint = map_16bit(pot_read, RH_POT_MIN, RH_POT_MAX, RH_MIN / RH_STEP, RH_MAX / RH_STEP) * RH_STEP;
      if (new_setpoint < RH_MIN) new_setpoint = RH_MIN; // controllo limite inferiore
      if (new_setpoint > RH_MAX) new_setpoint = RH_MAX; // controllo limite superiore

      // controllo variazioni del setpoint di umidità
      if (status.rh_setpoint != new_setpoint) {
        lcd_new_rh_setpoint(); // stampa su display lcd
        serial_new_rh_setpoint(); // stampa su serial monitor
        status.rh_setpoint = new_setpoint; // aggiornamento setpoint precedente
        status.manual_ctrl = false; // disabilitazione controllo manuale
      }
    }
  }

  /**
   * --- Gestione non bloccante del sensore SHT20 ---
   *
   * Il sensore viene trattato come una macchina con i seguenti 6 stati finiti
   * che si susseguono in sequenza ciclica, con il seguente ordine:
   *
   * 0. SHT20_IDLE:
   *   - il sensore è in pausa e non è in corso alcuna lettura
   *   - passa a SHT20_READY_FOR_TEMP_REQ dopo 1 secondo dall'ultima lettura
   *
   * 1. SHT20_READY_FOR_TEMP_REQ:
   *   - il sensore è pronto per ricevere la richiesta di lettura della temperatura
   *   - passa a SHT20_WAIT_TEMP dopo aver inviato la richiesta di lettura della temperatura
   *
   * 2. SHT20_WAIT_TEMP:
   *    - il sensore è in attesa della risposta alla richiesta di lettura della temperatura
   *    - passa a SHT20_READY_FOR_RH_REQ dopo aver ricevuto la risposta
   *
   * 3. SHT20_READY_FOR_RH_REQ:
   *    - il sensore è pronto per ricevere la richiesta di lettura dell'umidità
   *    - passa a SHT20_WAIT_RH dopo aver inviato la richiesta di lettura dell'umidità
   *
   * 4. SHT20_WAIT_RH:
   *    - il sensore è in attesa della risposta alla richiesta di lettura dell'umidità
   *    - passa a SHT20_NEW_DATA dopo aver ricevuto la risposta
   *
   * 5. SHT20_NEW_DATA:
   *    - il sensore ha nuovi dati disponibili per la temperatura e l'umidità
   *    - passa a SHT20_IDLE dopo aver letto e processato i dati
   */

  switch (status.sht20_state) {
    // sensore in attesa
    case SHT20_IDLE:
      if (millis() - timers.sht20_read >= SHT20_READ_INTERVAL) {
        timers.sht20_read = millis(); // aggiornamento timer richiesta
        status.sht20_state = SHT20_READY_FOR_TEMP_REQ; // aggiornamento stato
      }
      break;

    // sensore pronto per inviare la richiesta di lettura della temperatura
    case SHT20_READY_FOR_TEMP_REQ:
      SHT20.requestTemperature(); // invio richiesta al sensore
      if (sht20_error(SHT20.getError())) return; // verifica errori
      status.sht20_state = SHT20_WAIT_TEMP; // aggiornamento stato
      break;

    // sensore in attesa della risposta alla richiesta di lettura della temperatura
    case SHT20_WAIT_TEMP:
      if (SHT20.reqTempReady()) {
        SHT20.readTemperature(); // recupero temperatura dal sensore
        if (sht20_error(SHT20.getError())) return; // verifica errori
        status.sht20_state = SHT20_READY_FOR_RH_REQ; // aggiornamento stato
      }
      break;

    // sensore pronto per inviare la richiesta di lettura dell'umidità
    case SHT20_READY_FOR_RH_REQ:
      SHT20.requestHumidity(); // invio richiesta al sensore
      if (sht20_error(SHT20.getError())) return; // verifica errori
      status.sht20_state = SHT20_WAIT_RH; // aggiornamento stato
      break;

    // sensore in attesa della risposta alla richiesta di lettura dell'umidità
    case SHT20_WAIT_RH:
      if (SHT20.reqHumReady()) {
        SHT20.readHumidity(); // recupero umidità dal sensore
        if (sht20_error(SHT20.getError())) return; // verifica errori
        status.sht20_state = SHT20_NEW_DATA; // aggiornamento stato
      }
      break;

    // sensore ha nuovi dati disponibili per la temperatura e l'umidità
    case SHT20_NEW_DATA:
      // lettura valori dal sensore
      status.temp_sht20 = SHT20.getTemperature();
      status.rh_sht20 = SHT20.getHumidity();

      // aggiornamento dell'output dei controllori
      if (!status.manual_ctrl) {
        temp_ctrl_update();
        rh_ctrl_update();
      }

      // stampa valori di temperatura e umidità su seriale
      serial_datalog();

      // aggiornamento stato
      status.sht20_state = SHT20_IDLE;
      break;

    // stato non valido, non si verifica mai
    default:
      break;
  }

  /**
   * @brief Implementazione del controllo PWM per gli attuatori.
   *
   * Siccome gli attuatori hanno un output binario (acceso/spento), mentre
   * un controllore PID produce un output discreto (0-255), si implementa
   * un sistema di conversione dell'output discreto del PID in un segnale
   * PWM binario per gli attuatori.
   *
   * Le variabili di stato "temp_pwm_value" e "rh_pwm_value" contengono
   * rispettivamente la larghezza del duty cycle in millisecondi per il
   * controllo tramite PWM del riscaldatore e dell'umidificatore. È stato
   * scelto di memorizzare il valore del duty cycle in millisecondi per
   * evitare di dover eseguire calcoli di conversione da un intervallo
   * discreto (0-255) a un intervallo di tempo in millisecondi.
   */

  // controllo del duty cycle per il riscaldatore
  if (!status.heat_relay && millis() % CTRL_PWM_PERIOD <= status.temp_pwm_value) {
    heat_turn_on();
    serial_auto_heat_on();
  }
  else if (status.heat_relay && millis() % CTRL_PWM_PERIOD > status.temp_pwm_value) {
    heat_turn_off();
    serial_auto_heat_off();
  }

  // controllo del duty cycle per l'umidificatore
  if (!status.rh_relay && !status.refill_led && millis() % CTRL_PWM_PERIOD <= status.rh_pwm_value) {
    rh_turn_on();
    serial_auto_rh_on();
  }
  else if (status.rh_relay && !status.refill_led && millis() % CTRL_PWM_PERIOD > status.rh_pwm_value) {
    rh_turn_off();
    serial_auto_rh_off();
  }

  /**
   * @brief Gestione dello spegnimento automatico del led di allarme.
   *
   * Il led di allarme si spegne dopo ALARM_INTERVAL millisecondi dalla sua
   * accensione e corrisponde a quando il display lcd viene aggiornato e non
   * mostra più il messaggio di errore.
   */

  if (status.alarm_led && millis() - timers.alarm >= ALARM_INTERVAL) {
    digitalWrite(ALARM_LED, LOW);
    status.alarm_led = false;
  }

  /**
   * @brief Gestione del refill dell'acqua.
   *
   * Il refill dell'acqua viene gestito tramite un contatore che tiene traccia
   * del tempo di accensione dell'umidificatore. Quando il contatore supera
   * REFILL_INTERVAL millisecondi, il led rosso di refill si accende e
   * l'umidificatore viene spento automaticamente.
   *
   * Per disattivare il blocco automatico dell'umidificatore, è necessario
   * riavviare l'incubatrice neonatale. In questo modo tutti i contatori
   * vengono azzerati.
   *
   * Il controllo coinvolge sia il tempo in cui l'umidificatore è stato acceso
   * nelle precedenti accensioni (status.refill_counter), sia il tempo passato
   * dall'ultima accensione (millis() - status.last_rh_on) nel caso in cui
   * l'umidificatore sia ancora acceso (... * status.rh_relay).
   */

  if (!status.refill_led && status.refill_counter + (millis() - status.last_rh_on) * status.rh_relay >= REFILL_INTERVAL) {
    rh_turn_off(); // spegnimento umidificatore
    lcd_refill_message();
    serial_refill_message();
    digitalWrite(REFILL_LED, HIGH);
    status.refill_led = true;
  }

  // stampa a video su display lcd lo stato dell'incubatrice neonatale
  lcd_print_status();
}

// --- implementazione funzioni ausiliarie ---

// traduce e stampa i messaggi associati agli errori del sensore SHT20
bool sht20_error(int errorCode) {
  if (errorCode == 0)
    return false;

  // stampa messaggi di errore su serial monitor e display lcd
  serial_sht20_error(errorCode);
  lcd_sht20_error(errorCode);

  // accensione led di allarme
  digitalWrite(ALARM_LED, HIGH); // accensione led di allarme
  timers.alarm = millis(); // aggiornamento timer allarme
  status.alarm_led = true; // aggiornamento variabile di stato allarme

  return true;
}
