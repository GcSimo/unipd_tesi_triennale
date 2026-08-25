/**
 * @file serial.cpp
 * @author Giacomo Simonetto
 * @brief Gestione della comunicazione seriale dell'incubatrice neonatale
 *
 * Implementazione delle funzioni per la gestione della comunicazione seriale
 * tra computer e incubatrice neonatale.
 */

#include "serial.h"

// messaggio di avvio su serial monitor
void serial_boot_message() {
  Serial.println(F("BOOT: avvio incubatrice neonatale"));
}

// trasmissione temperatura e umidità misurate
void serial_datalog() {
  Serial.print(F("CSV-: T: "));
  Serial.print(float_to_string(status.temp_sht20));
  Serial.print(F("°C | RH: "));
  Serial.print(float_to_string(status.rh_sht20));
  Serial.print(F("% | Set_T: "));
  Serial.print(float_to_string(status.temp_setpoint));
  Serial.print(F("°C | Set_RH: "));
  Serial.print(float_to_string(status.rh_setpoint));
  Serial.println(F("%"));
}

// visualizzazione del nuovo setpoint di temperatura
void serial_new_temp_setpoint() {
  Serial.print(F("SET0: New Set_T: "));
  Serial.print(float_to_string(status.temp_setpoint));
  Serial.println(F("°C"));
}

// visualizzazione del nuovo setpoint di umidità
void serial_new_rh_setpoint() {
  Serial.print(F("SET1: New Set_RH: "));
  Serial.print(float_to_string(status.rh_setpoint));
  Serial.println(F("%"));
}

// accensione manuale del riscaldatore
void serial_man_heat_on() {
  Serial.println(F("ACT0: Manual Heat On"));
}

// spegnimento manuale del riscaldatore
void serial_man_heat_off() {
  Serial.println(F("ACT1: Manual Heat Off"));
}

// accensione manuale dell'umidificatore
void serial_man_rh_on() {
  Serial.println(F("ACT2: Manual RH On"));
}

// spegnimento manuale dell'umidificatore
void serial_man_rh_off() {
  Serial.println(F("ACT3: Manual RH Off"));
}

// accensione manuale dell'illuminazione
void serial_man_light_on() {
  Serial.println(F("ACT4: Manual Light On"));
}

// spegnimento manuale dell'illuminazione
void serial_man_light_off() {
  Serial.println(F("ACT5: Manual Light Off"));
}

// accensione automatica del riscaldatore
void serial_auto_heat_on() {
  Serial.println(F("ACT6: Automatic Heat On"));
}

// spegnimento automatico del riscaldatore
void serial_auto_heat_off() {
  Serial.println(F("ACT7: Automatic Heat Off"));
}

// accensione automatica dell'umidificatore
void serial_auto_rh_on() {
  Serial.println(F("ACT8: Automatic RH On"));
}

// spegnimento automatico dell'umidificatore
void serial_auto_rh_off() {
  Serial.println(F("ACT9: Automatic RH Off"));
}

// stampa il messaggio di refill sul serial monitor
void serial_refill_message() {
  Serial.println(F("ERR-: Refill water in the humidifier"));
  Serial.println(F("ACT9: Automatic RH Off"));
}

// stampa il messaggio di errore del sensore SHT20
void serial_sht20_error(int errorCode) {
  switch (errorCode) {
    case SHT2x_ERR_WRITECMD:
      Serial.println(F("ERR-: SHT20 - Error during I2C write command"));
      break;
    case SHT2x_ERR_READBYTES:
      Serial.println(F("ERR-: SHT20 - Error during I2C read bytes"));
      break;
    case SHT2x_ERR_HEATER_OFF:
      Serial.println(F("ERR-: SHT20 - Failed to switch off the internal heater"));
      break;
    case SHT2x_ERR_NOT_CONNECT:
      Serial.println(F("ERR-: SHT20 - Sensor not connected or does not acknowledge on the I2C bus"));
      break;
    case SHT2x_ERR_CRC_TEMP:
      Serial.println(F("ERR-: SHT20 - CRC check failed for the temperature reading"));
      break;
    case SHT2x_ERR_CRC_HUM:
      Serial.println(F("ERR-: SHT20 - CRC check failed for the humidity reading"));
      break;
    case SHT2x_ERR_CRC_STATUS:
      Serial.println(F("ERR-: SHT20 - CRC check failed for the status register"));
      break;
    case SHT2x_ERR_HEATER_COOLDOWN:
      Serial.println(F("ERR-: SHT20 - Heater is in its mandatory cool-down period and cannot be re-enabled yet"));
      break;
    case SHT2x_ERR_HEATER_ON:
      Serial.println(F("ERR-: SHT20 - Failed to switch on the internal heater"));
      break;
    case SHT2x_ERR_RESOLUTION:
      Serial.println(F("ERR-: SHT20 - Invalid resolution parameter provided"));
      break;
    default:
      Serial.println(F("ERR-: SHT20 - Unknown error occurred"));
      break;
  }
}
