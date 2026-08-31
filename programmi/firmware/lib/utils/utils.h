/**
 * @file utils.h
 * @author Giacomo Simonetto
 * @brief Funzioni di utilità per l'incubatrice neonatale
 *
 * Dichiarazione delle funzioni di utilità per l'incubatrice neonatale.
 */

#ifndef UTILS_H
#define UTILS_H

#include "config.h"

// ----------------------------------------------------------------------------
// --------- gestione della conversione da intero a 4 cifre a stringa ---------
// ----------------------------------------------------------------------------

// buffer per conversione da intero a 4 cifre a stringa
extern char float_buffer[6];

/**
 * @brief Converte una misurazione in stringa formattata correttamente.
 *
 * Converte i valori di temperatura ed umidità nella loro rappresentazione
 * in stringa con due cifre intere e due decimali per la stampa sul display
 * lcd e sul monitor seriale.
 *
 * @param value il numero da convertire come intero a 4 cifre
 * @return puntatore al float_buffer contenente la stringa formattata
 */
char *data_to_string(int16_t value);


// ----------------------------------------------------------------------------
// ------- calcolo dell'umidità relativa al raggiungimento del setpoint -------
// ----------------------------------------------------------------------------

/**
 * @brief Calcola l'umidità relativa risultante al raggiungimento del setpoint
 * di temperatura.
 *
 * Il valore di umidità relativa è calcolato in funzione della temperatura,
 * per cui variando la temperatura cambia anche il valore di umidità relativa.
 * Per gestire correttamente l'accensione e lo spegnimento dell'umidificatore
 * è necessario confrontare il setpoint di umidità impostato con l'umidità
 * relativa che si avrebbe al raggiungimento del setpoint di temperatura.
 * Per calcolare tale valore si utilizza la seguente formula:
 *
 *   rh2 = rh1 * exp(4283.78 * (t1 - t2) / (243.12 + t1) / (243.12 + t2))
 *
 *   - t1: temperatura dello stato 1 in °C (attuale - misurata dallo SHT20)
 *   - t2: temperatura dello stato 2 in °C (desiderata - dal setpoint)
 *   - rh1: umidità relativa dello stato 1 (attuale - misurata dallo SHT20)
 *   - rh2: umidità relativa dello stato 2 (desiderata)
 *
 * fonte: https://sensirion.com/media/documents/A419127A/6836C0D2/Sensirion_AppNotes_Humidity_Sensors_at_a_Glance.pdf
 *
 * Per rendere il calcolo più efficiente, impiegando solo numeri interi nella
 * rappresentazione a 4 cifre, la formula viene riscritta come segue:
 *
 * rh2 = rh1/100 * exp(4283.78 * (t1/100 - t2/100) / (243.12 + t1/100) / (243.12 + t2/100)) * 100
 *     = rh1 * exp(4283.78 * (t1 - t2)/100 / ((24312 + t1)/100) / ((24312 + t2)/100))
 *     = rh1 * exp(4283.78 * (t1 - t2) / 100 / (24312 + t1) * 100 / (24312 + t2) * 100)
 *     = rh1 * exp(4283.78 * (t1 - t2) / (24312 + t1) / (24312 + t2) * 100)
 *     = rh1 * exp(428378 * (t1 - t2) / (24312 + t1) / (24312 + t2))
 *
 * @param rh1 umidità attuale misurata dal sensore SHT20
 * @param t1 temperatura attuale misurata dal sensore SHT20
 * @param t2 setpoint di temperatura desiderato
 * @return int16_t umidità relativa al raggiungimento del setpoint di temperatura
 */
int16_t rh_at_temp_setpoint(int16_t rh1, int16_t t1, int16_t t2);

#endif // UTILS_H
