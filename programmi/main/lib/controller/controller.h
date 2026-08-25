/**
 * @file controller.h
 * @author Giacomo Simonetto
 * @brief Controllo degli attuatori dell'incubatrice neonatale
 *
 * Dichiarazione delle funzioni dei controllori ad isteresi ON/OFF e PID
 * per la gestione degli attuatori dell'incubatrice neonatale.
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "config.h"

/**
 * @brief Inizializza i parametri per i controllori PID.
 */
void ctrl_begin();

/**
 * @brief Calcola l'output del controllore PID.
 *
 * Non modifica le variabili globali di stato dell'incubatrice,
 * come temp_pwm_value o rh_pwm_value. Serve per avere una procedura
 * generale e comune per il calcolo dell'output sia del PID per la
 * temperatura che per l'umidità.
 *
 * @param pid reference alla struct con i parametri del PID
 * @param error errore tra setpoint e valore misurato
 * @return float output del PID, limitato tra CTRL_MIN_OUTPUT e CTRL_MAX_OUTPUT
 */
float pid_compute(struct pid &pid, float error);

/**
 * @brief Aggiorna l'output del controllore di riscaldamento.
 *
 * Modifica la variabile globale di stato temp_pwm_value in base
 * all'output del controllore utilizzato.
 */
void temp_ctrl_update();

/**
 * @brief Aggiorna l'output del controllore di umidità.
 *
 * Modifica la variabile globale di stato rh_pwm_value in base
 * all'output del controllore utilizzato.
 */
void rh_ctrl_update();

#endif // CONTROLLER_H
