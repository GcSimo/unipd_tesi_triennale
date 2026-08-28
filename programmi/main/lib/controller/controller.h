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
 * ----------------------------------------------------------------------------
 * @brief Gestione del controllore PID
 *
 * L'output del pid viene aggiornato ogni volta che inizia un nuovo ciclo del
 * segnale PWM, quindi ogni CTRL_PWM_PERIOD millisecondi. Le misurazioni che
 * avvengono tra un aggiornamento dell'output e il successivo vengono salvate
 * all'interno di un buffer per calcolare correttamente le tre componenti del
 * PID (proporzionale, integrale e derivativa).
 *
 * Si avrà quindi una funzione invocata all'acquisizione dei nuovi dati e una
 * funzione invocata prima dell'inizio di ogni ciclo del PWM per aggiornare
 * l'output del controllore.
 *
 * ----------------------------------------------------------------------------
 * @brief Gestione del controllore ad isteresi ON/OFF
 *
 * Siccome si vuole che l'output del controllore sia immediatamente utilizzato
 * per il controllo degli attuatori, si utilizza un approccio diverso rispetto
 * a quello del PID. L'output del controllore ad isteresi viene aggiornato
 * dalla funzione invocata dopo ogni misurazione, mentre la funzione invocata
 * all'inizio del ciclo del PWM non fa nulla.
 *
 * ----------------------------------------------------------------------------
 */

/**
 * @brief Inizializza i parametri per i controllori PID.
 *
 * Inizializza i parametri presenti nelle struct dei controllori PID per la
 * temperatura e l'umidità, se tali grandezze sono controllate tramite PID.
 */
void ctrl_begin();

/**
 * @brief Elabora la nuova misurazione di temperatura.
 *
 * Invoca la funzione pid_add_data() per aggiornare gli accumulatori del PID
 * della temperatura oppure aggiorna direttamente l'output del controllore ad
 * isteresi ON/OFF della temperatura.
 *
 * Deve essere invocata ogni volta che è disponibile una nuova misurazione
 * di temperatura dal sensore SHT20.
 */
void temp_ctrl_new_measure();

/**
 * @brief Gestisce l'inizio del ciclo del PWM della temperatura.
 *
 * Invoca la funzione pid_update_output() per aggiornare l'output del PID
 * della temperatura oppure non fa nulla se la temperatura è controllata
 * tramite un controllore ad isteresi ON/OFF.
 *
 * Esegue un controllo per evitare accensioni o spegnimenti troppo brevi
 * del riscaldatore, rilassando il relè e l'attuatore.
 *
 * Deve essere invocata all'inizio di ogni ciclo del PWM per aggiornare
 * l'output del PID della temperatura per il prossimo ciclo del PWM.
 */
void temp_ctrl_new_pwm_cycle();

/**
 * @brief Elabora la nuova misurazione di umidità.
 *
 * Invoca la funzione pid_add_data() per aggiornare gli accumulatori del PID
 * dell'umidità oppure aggiorna direttamente l'output del controllore ad
 * isteresi ON/OFF dell'umidità.
 *
 * Deve essere invocata ogni volta che è disponibile una nuova misurazione
 * dell'umidità dal sensore SHT20.
 */
void rh_ctrl_new_measure();

/**
 * @brief Gestisce l'inizio del ciclo del PWM dell'umidità.
 *
 * Invoca la funzione pid_update_output() per aggiornare l'output del PID
 * dell'umidità oppure non fa nulla se l'umidità è controllata tramite un
 * controllore ad isteresi ON/OFF.
 *
 * Esegue un controllo per evitare accensioni o spegnimenti troppo brevi
 * dell'umidificatore, rilassando il relè e l'attuatore.
 *
 * Deve essere invocata all'inizio di ogni ciclo del PWM per aggiornare
 * l'output del PID dell'umidità per il prossimo ciclo del PWM.
 */
void rh_ctrl_new_pwm_cycle();


// ----------------------------------------------------------------------------
// ---- utility functions per evitare codice doppio nella gestione dei PID ----
// ----------------------------------------------------------------------------

/**
 * @brief Aggiorna gli accumulatori con la nuova misurazione.
 *
 * Aggiorna gli accumulatori della struct del PID in base ai nuovi dati
 * del sensore (errore e valore misurato). Non aggiorna l'output del PID.
 *
 * Viene invocata automaticamente dalle funzioni temp_ctrl_new_measure() e
 * rh_ctrl_new_measure() ogni volta che è disponibile una nuova misurazione
 * di temperatura o umidità dal sensore SHT20.
 *
 * Non modifica variabili globali di stato dell'incubatrice.
 *
 * @param pid reference alla struct del PID da aggiornare
 * @param error const reference all'errore tra setpoint e valore misurato
 * @param measure const reference al valore misurato dal sensore
 */
void pid_add_data(struct pid &pid, const int16_t &error, const uint16_t &measure);

/**
 * @brief Aggiorna l'output del controllore PID.
 *
 * Calcola l'output del PID in base allo stato corrente degli accumulatori
 * aggiornati dalla funzione pid_add_data().
 *
 * Viene invocata automaticamente dalle funzioni temp_ctrl_new_pwm_cycle()
 * e rh_ctrl_new_pwm_cycle() all'inizio di ogni ciclo del PWM per aggiornare
 * l'output del PID per il ciclo PWM successivo.
 *
 * Non modifica variabili globali di stato dell'incubatrice.
 *
 * @param pid reference alla struct del PID da aggiornare
 * @return float output del PID limitato tra CTRL_MIN_OUTPUT e CTRL_MAX_OUTPUT
 */
float pid_update_output(struct pid &pid);

#endif // CONTROLLER_H
