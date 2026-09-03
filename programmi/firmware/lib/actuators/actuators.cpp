/**
 * @file actuators.cpp
 * @author Giacomo Simonetto
 * @brief Controllo degli attuatori dell'incubatrice neonatale
 *
 * Implementazione delle funzioni di controllo degli attuatori dell'incubatrice
 * neonatale e dei relativi led di stato del pannello frontale.
 */

#include "actuators.h"
#include "utils.h"

/**
 * NOTE IMPLEMENTATIVE sullo spegnimento della ventola di omogeneizzazione:
 *
 * La condizione di spegnimento è la seguente:
 *
 *  ON = heat_on || rh_on || (auto_ctrl && TEMP_CTRL == 2 && temp_pwm_value != 0) || (auto_ctrl && RH_CTRL == 2 && rh_pwm_value != 0)
 *
 *  OFF = !ON =
 *      = !heat_on && !rh_on && !(auto_ctrl && TEMP_CTRL == 2 && temp_pwm_value != 0) && !(auto_ctrl && RH_CTRL == 2 && rh_pwm_value != 0) =
 *      = heat_off && rh_off && (manual_ctrl || TEMP_CTRL != 2 || temp_pwm_value == 0) && (manual_ctrl || RH_CTRL != 2 || rh_pwm_value == 0) =
 *      = heat_off && rh_off && (manual_ctrl || ((TEMP_CTRL != 2 || temp_pwm_value == 0) && (RH_CTRL != 2 || rh_pwm_value == 0)))
 *
 * Analizzando i 4 casi possibili per le configurazioni dei controllori degli
 * attuatori, si ottengono le seguenti condizioni di spegnimento:
 *
 * 1. entrambi gli attuatori sono controllati da un pid con segnale pwm,
 *    ovvero per TEMP_CTRL == PID && RH_CTRL == PID
 *
 *    OFF = heat_off && rh_off && (manual_ctrl || (temp_pwm_value == 0 && rh_pwm_value == 0))
 *
 * 2. solo il riscaldatore è controllato da un pid con segnale pwm, ovvero
 *    per TEMP_CTRL == PID && RH_CTRL != PID
 *
 *    OFF = heat_off && rh_off && (manual_ctrl || temp_pwm_value == 0))
 *
 * 3. solo l'umidificatore è controllato da un pid con segnale pwm, ovvero
 *    per TEMP_CTRL != PID && RH_CTRL == PID
 *
 *    OFF = heat_off && rh_off && (manual_ctrl || rh_pwm_value == 0))
 *
 * 4. nessuno dei due attuatori è controllato da un pid con segnale pwm,
 *    ovvero per TEMP_CTRL != PID && RH_CTRL != PID
 *
 *    OFF = heat_off && rh_off
 */

// accensione riscaldatore
bool heat_turn_on() {
  // se il riscaldatore è già acceso, restituisce false senza fare nulla
  if (status.heat_relay)
    return false;

  // accensione riscaldatore e LED di stato
  digitalWrite(HEAT_RELAY, RELAY_ON); // accensione riscaldatore
  digitalWrite(HEAT_LED, LED_ON);     // accensione LED riscaldatore
  status.heat_relay = true; // aggiornamento stato riscaldatore

  // accensione ventola di omogeneizzazione se spenta
  if (!status.fan_relay) {
    digitalWrite(FAN_RELAY, RELAY_ON); // accensione ventola di omogeneizzazione
    status.fan_relay = true; // aggiornamento stato ventola di omogeneizzazione
  }

  // accensione avvenuta con successo
  return true;
}

// spegnimento riscaldatore
bool heat_turn_off() {
  // se il riscaldatore è già spento, restituisce false senza fare nulla
  if (!status.heat_relay)
    return false;

  // spegnimento riscaldatore e LED di stato
  digitalWrite(HEAT_RELAY, RELAY_OFF); // spegnimento riscaldatore
  digitalWrite(HEAT_LED, LED_OFF);     // spegnimento LED riscaldatore
  status.heat_relay = false; // aggiornamento stato riscaldatore

  // richiesta di spegnimento ventola di omogeneizzazione
  fan_turn_off();

  // spegnimento avvenuto con successo
  return true;
}

// accensione umidificatore
bool rh_turn_on() {
  // se l'umidificatore è già acceso o serve effettuare il refill dell'acqua,
  // restituisce false senza fare nulla
  if (status.rh_relay || status.refill_led)
    return false;

  // accensione umidificatore e LED di stato
  digitalWrite(RH_RELAY, RELAY_ON); // accensione umidificatore
  digitalWrite(RH_LED, LED_ON);     // accensione LED umidificatore
  status.rh_relay = true; // aggiornamento stato umidificatore

  // accensione ventola di omogeneizzazione se spenta
  if (!status.fan_relay) {
    digitalWrite(FAN_RELAY, RELAY_ON); // accensione ventola di omogeneizzazione
    status.fan_relay = true; // aggiornamento stato ventola di omogeneizzazione
  }

  // gestione contatore refill
  timers.last_rh_on = millis();

  // accensione avvenuta con successo
  return true;
}

// spegnimento umidificatore
bool rh_turn_off() {
  // se l'umidificatore è già spento, restituisce false senza fare nulla
  if (!status.rh_relay)
    return false;

  // spegnimento umidificatore e LED di stato
  digitalWrite(RH_RELAY, RELAY_OFF); // spegnimento umidificatore
  digitalWrite(RH_LED, LED_OFF);     // spegnimento LED umidificatore
  status.rh_relay = false; // aggiornamento stato umidificatore

  // richiesta di spegnimento ventola di omogeneizzazione
  fan_turn_off();

  // gestione contatore refill
  timers.refill_counter += millis() - timers.last_rh_on;

  // spegnimento avvenuto con successo
  return true;
}

// richiesta di spegnimento ventola di omogeneizzazione
bool fan_turn_off() {
  // se la ventola di omogeneizzazione è già spenta, restituisce false senza fare nulla
  if (!status.fan_relay)
    return false;

  // verifica se è possibile spegnere la ventola di omogeneizzazione
  #if TEMP_CTRL == PID && RH_CTRL == PID
  if (!status.heat_relay && (status.manual_ctrl || (!status.temp_pwm_value && !status.rh_pwm_value))) {
  #elif TEMP_CTRL == PID && RH_CTRL != PID
  if (!status.heat_relay && (status.manual_ctrl || !status.temp_pwm_value)) {
  #elif TEMP_CTRL != PID && RH_CTRL == PID
  if (!status.heat_relay && (status.manual_ctrl || !status.rh_pwm_value)) {
  #else
  if (!status.heat_relay) {
  #endif
    digitalWrite(FAN_RELAY, RELAY_OFF); // spegnimento ventola di omogeneizzazione
    status.fan_relay = false; // aggiornamento stato ventola di omogeneizzazione
    return true; // spegnimento avvenuto con successo
  }

  // ventola di omogeneizzazione non spenta, nessuna azione eseguita
  return false;
}

// accensione illuminazione
bool light_turn_on() {
  // se l'illuminazione è già accesa, restituisce false senza fare nulla
  if (status.light_relay)
    return false;

  digitalWrite(LIGHT_RELAY, RELAY_ON); // accensione illuminazione
  status.light_relay = true; // aggiornamento stato illuminazione

  // accensione avvenuta con successo
  return true;
}

// spegnimento illuminazione
bool light_turn_off() {
  // se l'illuminazione è già spenta, restituisce false senza fare nulla
  if (!status.light_relay)
    return false;

  digitalWrite(LIGHT_RELAY, RELAY_OFF); // spegnimento illuminazione
  status.light_relay = false; // aggiornamento stato illuminazione

  // spegnimento avvenuto con successo
  return true;
}
