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

#include "config.h"
#include "error.h"
#include "utils.h"
#include "actuators.h"
#include "controller.h"
#include "serial.h"
#include "lcd.h"

// ----------------------------------------------------------------------------
// ----------------------- variabili globali temporanee -----------------------
// ----------------------------------------------------------------------------

int16_t sensor_read = 0;        // valore appena letto dal sensore SHT20
uint16_t pot_read = 0;          // valore appena letto dai potenziometri
uint16_t new_temp_setpoint = 0; // setpoint appena calcolato
uint16_t new_rh_setpoint = 0;   // setpoint appena calcolato


// ----------------------------------------------------------------------------
// ------------------------------ setup function ------------------------------
// ----------------------------------------------------------------------------

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
  digitalWrite(ALARM_LED, LED_OFF);
  digitalWrite(HEAT_LED, LED_OFF);
  digitalWrite(RH_LED, LED_OFF);
  digitalWrite(REFILL_LED, LED_OFF);

  digitalWrite(HEAT_RELAY, RELAY_OFF);
  digitalWrite(RH_RELAY, RELAY_OFF);
  digitalWrite(FAN_RELAY, RELAY_OFF);
  digitalWrite(LIGHT_RELAY, RELAY_OFF);

  // inizializzazione stato iniziale delle variabili globali
  memset(&status, 0, sizeof(status));
  memset(&timers, 0, sizeof(timers));
  memset(&temp_pid, 0, sizeof(temp_pid));
  memset(&rh_pid, 0, sizeof(rh_pid));

  // inizializzazione controllori PID
  ctrl_begin();

  // inizializzazione sensore SHT20 e controllo errori
  while (!sensor.begin()) {
    // stampa messaggi di errore su serial monitor e display lcd
    serial_sht20_error(sensor.get_error());
    lcd_sht20_error(sensor.get_error());

    digitalWrite(ALARM_LED, LED_ON); // accensione led di allarme
    delay(1000); // attesa di 1 secondo prima di riprovare
  }
  digitalWrite(ALARM_LED, LED_OFF); // spegnimento led in assenza di errori

  // inizializzazione setpoint di temperatura e umidità
  status.temp_setpoint = new_temp_setpoint = TEMP_DEF_SP;
  status.rh_setpoint = new_rh_setpoint = RH_DEF_SP;

  // inizializzazione valori potenziometri e switch
  status.temp_pot_value = analogRead(TEMP_POT);
  status.rh_pot_value = analogRead(RH_POT);
  status.heat_sw = digitalRead(HEAT_SW);
  status.rh_sw = digitalRead(RH_SW);
  status.light_sw = digitalRead(LIGHT_SW);

  delay(2000);
}


// ----------------------------------------------------------------------------
// ------------------------------ loop function -------------------------------
// ----------------------------------------------------------------------------

void loop() {

  // --------------------------------------------------------------------------
  // ------ gestione switch riscaldatore, umidificatore e illuminazione -------
  // --------------------------------------------------------------------------

  // --- cambio stato switch riscaldatore ---
  if (millis() - timers.heat_sw >= SW_READ_PERIOD && status.heat_sw != digitalRead(HEAT_SW)) {
    timers.heat_sw = millis(); // aggiornamento timer
    status.heat_sw = !status.heat_sw; // aggiornamento stato switch
    status.manual_ctrl = MANUAL_CTRL; // abilitazione controllo manuale

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
  if (millis() - timers.rh_sw >= SW_READ_PERIOD && status.rh_sw != digitalRead(RH_SW)) {
    timers.rh_sw = millis(); // aggiornamento timer
    status.rh_sw = !status.rh_sw; // aggiornamento stato switch
    status.manual_ctrl = MANUAL_CTRL; // abilitazione controllo manuale

    // avviso di refill acqua
    if (status.refill_led && status.rh_sw) {
      lcd_refill_message(); // visualizzazione messaggio refill sul display
    }

    // switch umidificatore attivo
    else if (!status.refill_led && status.rh_sw) {
      rh_turn_on(); // accensione umidificatore e led associato
      lcd_man_rh_on(); // visualizzazione accensione umidificatore sul display
      serial_man_rh_on(); // visualizzazione accensione umidificatore su serial monitor
    }

    // switch umidificatore spento
    else if (!status.refill_led && !status.rh_sw) {
      rh_turn_off(); // spegnimento umidificatore e led associato
      lcd_man_rh_off(); // visualizzazione spegnimento umidificatore sul display
      serial_man_rh_off(); // visualizzazione spegnimento umidificatore su serial monitor
    }
  }

  // --- cambio stato switch illuminazione ---
  if (millis() - timers.light_sw >= SW_READ_PERIOD && status.light_sw != digitalRead(LIGHT_SW)) {
    timers.light_sw = millis(); // aggiornamento timer
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


  // --------------------------------------------------------------------------
  // ------------ aggiornamento setpoint di temperatura e umidità -------------
  // --------------------------------------------------------------------------

  /**
   * @brief Aggiornamento del setpoint di temperatura.
   *
   * Il processo di aggiornamento del setpoint di temperatura avviene nelle
   * seguenti fasi:
   *
   * 1. lettura del potenziometro ogni TEMP_POT_TIMER millisecondi
   * 2. controllo di soglia per eliminare disturbi e rumore elettrico
   * 3. calcolo del nuovo setpoint di temperatura
   * 4. visualizzazione del nuovo setpoint sul display lcd e su serial monitor
   * 5. aggiornamento del setpoint se il nuovo valore è diverso dal precedente
   *    e se è trascorso il tempo di ritardo previsto SETPOINT_UPDATE_DELAY
   */

  // controllo timer per lettura potenziometro temperatura
  if (millis() - timers.temp_pot >= TEMP_POT_READ_PERIOD) {
    pot_read = analogRead(TEMP_POT); // lettura potenziometro temperatura
    timers.temp_pot = millis();  // aggiornamento timer

    // controllo threshold
    if (abs(status.temp_pot_value - pot_read) >= TEMP_POT_THLD) {
      status.temp_pot_value = pot_read; // aggiornamento valore potenziometro temperatura

      // calcolo nuovo setpoint di temperatura
      new_temp_setpoint = map(pot_read, TEMP_POT_MIN, TEMP_POT_MAX, TEMP_MIN_SP / TEMP_STEP_SP, TEMP_MAX_SP / TEMP_STEP_SP) * TEMP_STEP_SP;
      if (new_temp_setpoint < TEMP_MIN_SP) new_temp_setpoint = TEMP_MIN_SP; // controllo limite inferiore
      if (new_temp_setpoint > TEMP_MAX_SP) new_temp_setpoint = TEMP_MAX_SP; // controllo limite superiore

      // controllo variazioni del setpoint di temperatura
      if (status.temp_setpoint != new_temp_setpoint) {
        lcd_new_temp_setpoint(new_temp_setpoint); // stampa su display lcd
        timers.temp_setpoint = millis(); // aggiornamento timer
      }
    }
  }

  // aggiornamento effettivo del setpoint di temperatura
  if (status.temp_setpoint != new_temp_setpoint && millis() - timers.temp_setpoint >= SP_UPDATE_DELAY) {
    status.temp_setpoint = new_temp_setpoint; // aggiornamento setpoint precedente
    status.manual_ctrl = AUTOM_CTRL; // disabilitazione controllo manuale
    serial_new_temp_setpoint(); // stampa su serial monitor
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
   * 4. visualizzazione del nuovo setpoint sul display lcd e su serial monitor
   * 5. aggiornamento del setpoint se il nuovo valore è diverso dal precedente
   *    e se è trascorso il tempo di ritardo previsto SETPOINT_UPDATE_DELAY
   */

  // controllo timer per lettura potenziometro umidità
  if (millis() - timers.rh_pot >= RH_POT_READ_PERIOD) {
    pot_read = analogRead(RH_POT); // lettura potenziometro umidità
    timers.rh_pot = millis(); // aggiornamento timer

    // controllo threshold
    if (abs(status.rh_pot_value - pot_read) >= RH_POT_THLD) {
      status.rh_pot_value = pot_read; // aggiornamento valore potenziometro umidità

      // calcolo nuovo setpoint di umidità
      new_rh_setpoint = map(pot_read, RH_POT_MIN, RH_POT_MAX, RH_MIN_SP / RH_STEP_SP, RH_MAX_SP / RH_STEP_SP) * RH_STEP_SP;
      if (new_rh_setpoint < RH_MIN_SP) new_rh_setpoint = RH_MIN_SP; // controllo limite inferiore
      if (new_rh_setpoint > RH_MAX_SP) new_rh_setpoint = RH_MAX_SP; // controllo limite superiore

      // controllo variazioni del setpoint di umidità
      if (status.rh_setpoint != new_rh_setpoint) {
        lcd_new_rh_setpoint(new_rh_setpoint); // stampa su display lcd
        timers.rh_setpoint = millis(); // aggiornamento timer
      }
    }
  }

  // aggiornamento effettivo del setpoint di umidità
  if (status.rh_setpoint != new_rh_setpoint && millis() - timers.rh_setpoint >= SP_UPDATE_DELAY) {
    status.rh_setpoint = new_rh_setpoint; // aggiornamento setpoint precedente
    status.manual_ctrl = AUTOM_CTRL; // disabilitazione controllo manuale
    serial_new_rh_setpoint(); // stampa su serial monitor
  }


  // --------------------------------------------------------------------------
  // ----------- lettura dati dal sensore SHT20 - vedi classe sht20 -----------
  // --------------------------------------------------------------------------

  if (!sensor.update()) { // nessun dato disponibile o presenza di errori
    // verifica presenza di errori nel sensore SHT20
    if (sensor.get_error() != 0) {
      // stampa messaggi di errore su serial monitor e display lcd
      serial_sht20_error(sensor.get_error());
      lcd_sht20_error(sensor.get_error());

      // impostazione bit di errore per il sensore SHT20
      err_set(ERR_SHT20);
    }

  } else { // nuove misure disponibili per la temperatura e l'umidità
    // lettura valori dal sensore e conversione in intero a 4 cifre
    sensor_read = (int16_t)(sensor.get_temperature() * 100 + 0.5);

    // verifica overflow della temperatura e gestione errori
    if (sensor_read == NAN) {     // verifica temperatura non valida
      err_set(ERR_TEMP_OVERFLOW); //  - impostazione bit di errore
      status.temp_sht20 = 0;      //  - assegnazione valore minimo
    }
    else if (sensor_read < 0) {   // verifica temperatura negativa
      err_set(ERR_TEMP_OVERFLOW); //  - impostazione bit di errore
      status.temp_sht20 = 0;      //  - assegnazione valore minimo
    }
    else if (sensor_read > 10000) { // verifica temperatura troppo alta
      err_set(ERR_TEMP_OVERFLOW);   //  - impostazione bit di errore
      status.temp_sht20 = 9999;     //  - assegnazione valore massimo
    }
    else {                                       // temperatura valida
      err_rm(ERR_TEMP_OVERFLOW);                 //  - rimozione bit di errore
      status.temp_sht20 = (uint16_t)sensor_read; //  - conversione a uint16_t
    }

    // lettura valore di umidità dal sensore e conversione in intero a 4 cifre
    sensor_read = (int16_t)(sensor.get_humidity() * 100 + 0.5);

    // verifica overflow dell'umidità e gestione errori
    if (sensor_read == NAN) {   // verifica umidità non valida
      err_set(ERR_RH_OVERFLOW); //  - impostazione bit di errore
      status.rh_sht20 = 0;      //  - assegnazione valore minimo
    }
    else if (sensor_read < 0) { // verifica umidità negativa
      err_set(ERR_RH_OVERFLOW); //  - impostazione bit di errore
      status.rh_sht20 = 0;      //  - assegnazione valore minimo
    }
    else if (sensor_read > 10000) { // verifica umidità troppo alta
      err_set(ERR_RH_OVERFLOW);     //  - impostazione bit di errore
      status.rh_sht20 = 9999;       //  - assegnazione valore massimo
    }
    else {                                     // umidità valida
      err_rm(ERR_RH_OVERFLOW);                 //  - rimozione bit di errore
      status.rh_sht20 = (uint16_t)sensor_read; // - conversione a uint16_t
    }

    // aggiornamento dei controllori con le nuove misurazioni
    if (!status.manual_ctrl && !err_check(ERR_TEMP_OVERFLOW) && !err_check(ERR_RH_OVERFLOW)) {
      temp_ctrl_new_measure();
      rh_ctrl_new_measure();
    }

    // stampa valori di temperatura e umidità su seriale
    serial_datalog();
  }


  // --------------------------------------------------------------------------
  // ------------------- controllo PWM per attuatori binari -------------------
  // --------------------------------------------------------------------------

  /**
   * @brief Implementazione del controllo PWM per gli attuatori.
   *
   * Siccome gli attuatori hanno un output binario (acceso/spento), mentre
   * un controllore PID produce un output discreto (0-100), si implementa
   * un sistema di conversione dell'output discreto del PID in un segnale
   * PWM binario per gli attuatori.
   *
   * Le variabili di stato "temp_pwm_value" e "rh_pwm_value" contengono
   * rispettivamente la larghezza del duty cycle in millisecondi per il
   * controllo tramite PWM del riscaldatore e dell'umidificatore. È stato
   * scelto di memorizzare il valore del duty cycle in millisecondi per
   * evitare di dover eseguire calcoli di conversione da un intervallo
   * discreto (0-100) a un intervallo di tempo in millisecondi.
   */

  // controllo del duty cycle per il riscaldatore

  // inizio del ciclo di controllo PWM ogni CTRL_PWM_PERIOD millisecondi
  if (millis() - timers.start_pwm >= CTRL_PWM_PERIOD) {
    // aggiornamento timer
    timers.start_pwm = millis();

    // aggiornamento dei valori del duty cycle in millisecondi
    temp_ctrl_new_pwm_cycle();
    rh_ctrl_new_pwm_cycle();
  }

  // controllo
  if (!status.manual_ctrl && !status.heat_relay && millis() - timers.start_pwm <= status.temp_pwm_value) {
    heat_turn_on();
    serial_auto_heat_on();
  }
  else if (!status.manual_ctrl && status.heat_relay && millis() - timers.start_pwm > status.temp_pwm_value) {
    heat_turn_off();
    serial_auto_heat_off();
  }

  // controllo del duty cycle per l'umidificatore
  if (!status.manual_ctrl && !status.rh_relay && !status.refill_led && millis() - timers.start_pwm <= status.rh_pwm_value) {
    rh_turn_on();
    serial_auto_rh_on();
  }
  else if (!status.manual_ctrl && status.rh_relay && millis() - timers.start_pwm > status.rh_pwm_value) {
    rh_turn_off();
    serial_auto_rh_off();
  }


  // --------------------------------------------------------------------------
  // -------------------- gestione errori e led di allarme --------------------
  // --------------------------------------------------------------------------

  check_temp_range(); // controllo temperatura fuori dal range di sicurezza
  check_rh_range();   // controllo umidità fuori dal range di sicurezza
  update_temporized_errors(); // aggiornamento stato errori temporizzati
  update_alarm_led(); // aggiornamento stato led di allarme


  // --------------------------------------------------------------------------
  // ----------------- gestione refill acqua e led di refill ------------------
  // --------------------------------------------------------------------------

  // gestione refill acqua e led di refill
  if (check_refill()) {
    lcd_refill_message();
    serial_refill_message();
  }


  // --------------------------------------------------------------------------
  // ------------ aggiornamento display lcd con stato incubatrice -------------
  // --------------------------------------------------------------------------

  // aggiorna lo stato dell'incubatrice sul display lcd
  lcd_print_status();
}
