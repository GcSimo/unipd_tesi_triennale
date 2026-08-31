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

// accensione riscaldatore
bool heat_turn_on() {
  // se il riscaldatore è già acceso, restituisce false senza fare nulla
  if (status.heat_relay)
    return false;

  // accensione riscaldatore e LED di stato
  digitalWrite(HEAT_RELAY, RELAY_ON); // accensione riscaldatore
  digitalWrite(HEAT_LED, LED_ON);     // accensione LED riscaldatore
  status.heat_relay = true; // aggiornamento stato riscaldatore

  // accensione ventola ausiliaria se spenta
  if (!status.fan_relay) {
    digitalWrite(FAN_RELAY, RELAY_ON); // accensione ventola ausiliaria
    status.fan_relay = true; // aggiornamento stato ventola ausiliaria
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

  // spegnimento ventola ausiliaria se l'umidificatore è spento
  if (!status.rh_relay) {
    digitalWrite(FAN_RELAY, RELAY_OFF); // spegnimento ventola ausiliaria
    status.fan_relay = false; // aggiornamento stato ventola ausiliaria
  }

  // spegnimento avvenuto con successo
  return true;
}

// accensione umidificatore
bool rh_turn_on() {
  // se l'umidificatore è già acceso o serve refill, restituisce false senza fare nulla
  if (status.rh_relay || status.refill_led)
    return false;

  // accensione umidificatore e LED di stato
  digitalWrite(RH_RELAY, RELAY_ON); // accensione umidificatore
  digitalWrite(RH_LED, LED_ON);     // accensione LED umidificatore
  status.rh_relay = true; // aggiornamento stato umidificatore

  // accensione ventola ausiliaria se spenta
  if (!status.fan_relay) {
    digitalWrite(FAN_RELAY, RELAY_ON); // accensione ventola ausiliaria
    status.fan_relay = true; // aggiornamento stato ventola ausiliaria
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

  // spegnimento ventola ausiliaria se il riscaldatore è spento
  if (!status.heat_relay) {
    digitalWrite(FAN_RELAY, RELAY_OFF); // spegnimento ventola ausiliaria
    status.fan_relay = false; // aggiornamento stato ventola ausiliaria
  }

  // gestione contatore refill
  timers.refill_counter += millis() - timers.last_rh_on;

  // spegnimento avvenuto con successo
  return true;
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
