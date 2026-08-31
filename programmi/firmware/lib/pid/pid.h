/**
 * @file pid.h
 * @author Giacomo Simonetto
 * @brief Implementazione dei controllori PID.
 *
 * Dichiarazione delle funzioni per la gestione dei controllori PID per la
 * temperatura e l'umidità dell'incubatrice neonatale.
 */

#ifndef PID_H
#define PID_H

#include "config.h"

/**
 * @brief Gestione del controllore PID
 *
 * L'output del pid viene aggiornato ogni volta che inizia un nuovo ciclo del
 * segnale PWM, quindi ogni CTRL_PWM_PERIOD millisecondi. Le misurazioni che
 * avvengono tra un aggiornamento dell'output e il successivo vengono salvate
 * all'interno di accumulatori interni per calcolare successivamente le tre
 * componenti del PID (proporzionale, integrale e derivativa).
 *
 * Si avrà quindi una funzione invocata all'acquisizione dei nuovi dati per
 * fornire le nuove misurazioni da utilizzare nel futuro calcolo dell'output
 * del PID e una funzione invocata prima dell'inizio di ogni ciclo del PWM
 * per aggiornare l'output del controllore in base ai dati accumulati fino
 * a quel momento.
 *
 * Per evitare che vecchi dati accumulati prima della disattivazione del
 * controllo automatico influenzino il calcolo dell'output del PID alla
 * riattivazione del controllo automatico, è necessario resettare gli
 * accumulatori interni del PID tramite la funzione apposita.
 */


/**
 * @brief Inizializza i parametri del PID.
 *
 * Salva i parametri del PID all'interno della struct, inizializzando le
 * varie variabili interne del PID.
 *
 * Il parametro anti_windup è opzionale, se non specificato viene impostato
 * a 1 (clamping).
 *
 * @param pid reference alla struct del PID da inizializzare
 * @param kp const reference al guadagno proporzionale
 * @param ki const reference al guadagno integra
 * @param kd const reference al guadagno derivativo
 * @param kw const reference al guadagno della back calculation (anti-windup)
 * @param anti_windup const reference all'opzione di anti-windup da utilizzare
 */
void pid_init(struct pid &pid, const float &kp, const float &ki, const float &kd, const float &kw, const uint8_t &anti_windup = 1);

/**
 * @brief Salva una nuova misurazione negli accumulatori del PID.
 *
 * Salva l'errore e la misurazione in apposite variabili interne del PID
 * che verranno utilizzate per calcolare l'output del PID alla successiva
 * invocazione della funzione pid_update_output().
 *
 * Si assume che i dati forniti alla funzione vengano acquisiti dai sensori
 * ad intervalli di tempo costanti ogni PID_DATA_PERIOD millisecondi.
 * All'interno della funzione non si effettua nessun controllo temporale per
 * cui il rispetto di tale condizione è responsabilità del chiamante.
 *
 * Non ci sono vincoli sul tempo che intercorre da quando i dati vengono
 * acquisiti dal sensore e quando vengono passati alla funzione.
 *
 * @param pid reference alla struct del PID da aggiornare
 * @param error const reference all'errore tra setpoint e valore misurato
 * @param measure const reference al valore misurato dal sensore
 */
void pid_add_data(struct pid &pid, const int16_t &error, const int16_t &measure);

/**
 * @brief Aggiorna l'output del controllore PID.
 *
 * Calcola il nuovo output del PID in base ai dati salvati negli accumulatori
 * interni dalla funzione pid_add_data() nel tempo intercorso dall'ultimo
 * aggiornamento dell'output del PID.
 *
 * Si prevede che l'output del PID venga aggiornato ad intervalli di tempo
 * costanti ogni PID_UPDATE_PERIOD millisecondi. All'interno della funzione
 * non si effettua nessun controllo temporale per cui il rispetto di tale
 * condizione è responsabilità del chiamante.
 *
 * @param pid reference alla struct del PID da aggiornare
 * @return float output del PID limitato tra CTRL_MIN_OUTPUT e CTRL_MAX_OUTPUT
 */
float pid_update_output(struct pid &pid);

/**
 * @brief Resetta gli accumulatori del PID.
 *
 * Resetta gli accumulatori interni del pid che contengono i dati delle
 * misurazioni ricevute dall'ultimo aggiornamento dell'output del PID.
 *
 * Deve essere invocata ogni volta che si riattiva il controllo automatico
 * del PID dopo un periodo di controllo manuale, per evitare che i vecchi
 * dati memorizzati influenzino il successivo calcolo dell'output del PID.
 *
 * @param pid reference alla struct del PID da resettare
 */
void pid_reset_accumulators(struct pid &pid);

#endif // PID_H
