/**
 * @file error.cpp
 * @author Giacomo Simonetto
 * @brief Gestione degli errori dell'incubatrice neonatale
 *
 * Implementazione delle funzioni di controllo degli errori dell'incubatrice
 * neonatale.
 */

#include "error.h"
#include "utils.h"

// timer per gli errori temporizzati, in millisecondi
uint32_t err_timers[6] = {0, 0, 0, 0, 0, 0};

// imposta un errore nel codice di stato
void err_set(uint8_t error_code) {
  // verifica validità del codice di errore (0-15)
  if (error_code > 15)
    return;

  // imposta l'errore nel codice di stato
  status.error_code |= (1U << error_code);

  // imposta il timer per gli errori temporizzati
  if (error_code >= 10)
    err_timers[error_code - 10] = millis();
}

// rimuove un errore dal codice di stato
void err_clear(uint8_t error_code) {
  // verifica validità del codice di errore (0-15)
  if (error_code > 15)
    return;

  // rimuove l'errore dal codice di stato
  status.error_code &= ~(1U << error_code);
}

// verifica se un errore è presente nel codice di stato
bool err_check(uint8_t error_code) {
  // verifica validità del codice di errore (0-15)
  if (error_code > 15)
    return false;

  // verifica se l'errore è presente nel codice di stato
  return (status.error_code & (1U << error_code)) != 0;
}

// aggiorna lo stato del led di allarme
void update_alarm_led() {
  if (!status.alarm_led && status.error_code != 0) {
    digitalWrite(ALARM_LED, LED_ON);
    status.alarm_led = true;
  }
  else if (status.alarm_led && status.error_code == 0) {
    digitalWrite(ALARM_LED, LED_OFF);
    status.alarm_led = false;
  }
}

// verifica se ci sono errori temporizzati da rimuovere
void update_temporized_errors() {
  for (uint8_t i = 10; i < 16; i++) {
    if (err_check(i) && millis() - err_timers[i - 10] >= ERR_TIMER) {
      err_timers[i - 10] = 0; // resetta il timer
      err_clear(i); // rimuove l'errore temporizzato
    }
  }
}

// verifica se la temperatura misurata rientra nel range di sicurezza
void check_temp_range() {
  // temperatura troppo bassa
  if (!err_check(ERR_LOW_TEMP) && status.temp_sht20 < status.temp_setpoint - TEMP_ERR_THLD - TEMP_ERR_HYST)
    err_set(ERR_LOW_TEMP); // errore temperatura troppo bassa
  else if (err_check(ERR_LOW_TEMP) && status.temp_sht20 >= status.temp_setpoint - TEMP_ERR_THLD + TEMP_ERR_HYST)
    err_clear(ERR_LOW_TEMP); // rimozione errore temperatura troppo bassa

  // temperatura troppo alta
  else if (!err_check(ERR_HIGH_TEMP) && (status.temp_sht20 > status.temp_setpoint + TEMP_ERR_THLD + TEMP_ERR_HYST || status.temp_sht20 > TEMP_ERR_MAX))
    err_set(ERR_HIGH_TEMP); // errore temperatura troppo alta
  else if (err_check(ERR_HIGH_TEMP) && (status.temp_sht20 <= status.temp_setpoint + TEMP_ERR_THLD - TEMP_ERR_HYST && status.temp_sht20 <= TEMP_ERR_MAX))
    err_clear(ERR_HIGH_TEMP); // rimozione errore temperatura troppo alta
}

// verifica se l'umidità misurata rientra nel range di sicurezza
void check_rh_range() {
  // umidità troppo bassa
  if (!err_check(ERR_LOW_RH) && rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) < status.rh_setpoint - RH_ERR_THLD - RH_ERR_HYST)
    err_set(ERR_LOW_RH); // errore umidità troppo bassa
  else if (err_check(ERR_LOW_RH) && rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) >= status.rh_setpoint - RH_ERR_THLD + RH_ERR_HYST)
    err_clear(ERR_LOW_RH); // rimozione errore umidità troppo bassa

  // umidità troppo alta
  else if (!err_check(ERR_HIGH_RH) && rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) > status.rh_setpoint + RH_ERR_THLD + RH_ERR_HYST)
    err_set(ERR_HIGH_RH); // errore umidità troppo alta
  else if (err_check(ERR_HIGH_RH) && rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) <= status.rh_setpoint + RH_ERR_THLD - RH_ERR_HYST)
    err_clear(ERR_HIGH_RH); // rimozione errore umidità troppo alta
}
