/**
 * @file serial.h
 * @author Giacomo Simonetto
 * @brief Gestione della comunicazione seriale dell'incubatrice neonatale
 *
 * Dichiarazione delle funzioni per la gestione della comunicazione seriale
 * tra computer e incubatrice neonatale.
 */

#ifndef SERIAL_H
#define SERIAL_H

#include "config.h"

/**
 * @brief Stampa il messaggio di avvio su serial monitor.
 */
void serial_boot_message();

/**
 * @brief Trasmissione temperatura e umidità misurate.
 */
void serial_datalog();

/**
 * @brief Accensione manuale del riscaldatore.
 */
void serial_man_heat_on();

/**
 * @brief Spegnimento manuale del riscaldatore.
 */
void serial_man_heat_off();

/**
 * @brief Accensione manuale dell'umidificatore.
 */
void serial_man_rh_on();

/**
 * @brief Spegnimento manuale dell'umidificatore.
 */
void serial_man_rh_off();

/**
 * @brief Accensione manuale dell'illuminazione.
 */
void serial_man_light_on();

/**
 * @brief Spegnimento manuale dell'illuminazione.
 */
void serial_man_light_off();

/**
 * @brief Accensione automatica del riscaldatore.
 */
void serial_auto_heat_on();

/**
 * @brief Spegnimento automatico del riscaldatore.
 */
void serial_auto_heat_off();

/**
 * @brief Accensione automatica dell'umidificatore.
 */
void serial_auto_rh_on();

/**
 * @brief Spegnimento automatico dell'umidificatore.
 */
void serial_auto_rh_off();

/**
 * @brief Visualizzazione del nuovo setpoint di temperatura.
 */
void serial_new_temp_setpoint();

/**
 * @brief Visualizzazione del nuovo setpoint di umidità.
 */
void serial_new_rh_setpoint();

/**
 * @brief Stampa il messaggio di refill sul serial monitor.
 */
void serial_refill_message();

/**
 * @brief Stampa il messaggio di errore associato al codice di errore
 * restituito dal sensore SHT20.
 *
 * @param errorCode codice di errore
 */
void serial_sht20_error(int errorCode);

#endif // SERIAL_H
