/**
 * @file actuators.h
 * @author Giacomo Simonetto
 * @brief Controllo degli attuatori dell'incubatrice neonatale
 *
 * Dichiarazione delle funzioni di controllo degli attuatori dell'incubatrice
 * neonatale e dei relativi led di stato del pannello frontale.
 */

#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "config.h"

/**
 * @brief Gestione della ventola di omogeneizzazione dell'aria.
 *
 * Il modo più semplice per gestire la ventola di omogeneizzazione dell'aria
 * è quello di accenderla quando almeno uno dei due attuatori (riscaldatore
 * o umidificatore) è acceso e di spegnerla quando entrambi sono spenti.
 *
 * Quando, però, tali attuatori sono controllati da un pid con un segnale pwm,
 * la ventola deve fare continui e rapidi cicli di accensione-spegnimento che
 * risultano totalmente inutili. Provocano, infatti, l'usura delle componenti
 * elettriche e meccaniche della ventola e non permettono un efficiente
 * ricircolo dell'aria all'interno dell'incubatrice.
 *
 * Per questo motivo, è stato deciso di non vincolare l'accensione della
 * ventola alla sola accensione degli attuatori, ma di mantenerla attiva
 * se uno dei due attuatori viene pilotato da un segnale pwm non nullo.
 *
 * La gestione dell'accensione e dello spegnimento avviene all'interno delle
 * funzioni di accensione e spegnimento del riscaldatore e dell'umidificatore,
 * in modo da eseguire tutto in maniera automatica e centralizzata.
 *
 * Siccome può capitare che il pwm del riscaldatore e dell'umidificatore
 * diventi nullo quando entrambi gli attuatori sono spenti, è stata creata
 * una funzione apposita per gestire lo spegnimento della ventola, che deve
 * essere invocata ogni volta che uno dei due pwm diventa nullo.
 */

/**
 * @brief Accensione del riscaldatore.
 *
 * Questa funzione accende il riscaldatore e il led di stato corrispondente.
 * Accende anche la ventola di omogeneizzazione dell'aria se è spenta.
 *
 * @return true riscaldatore acceso con successo
 * @return false riscaldatore già acceso, nessuna azione eseguita
 */
bool heat_turn_on();

/**
 * @brief Spegnimento del riscaldatore.
 *
 * Questa funzione spegne il riscaldatore e il led di stato corrispondente.
 * Spegne anche la ventola di omogeneizzazione dell'aria se è possibile farlo.
 *
 * @return true riscaldatore spento con successo
 * @return false riscaldatore già spento, nessuna azione eseguita
 */
bool heat_turn_off();

/**
 * @brief Accensione dell'umidificatore.
 *
 * Questa funzione accende l'umidificatore e il led di stato corrispondente.
 * Accende anche la ventola di omogeneizzazione dell'aria se è spenta.
 *
 * Se è necessario fare un refill dell'acqua, l'umidificatore non viene acceso
 * e la funzione restituisce false senza fare nulla.
 *
 * All'accensione dell'umidificatore, viene aggiornato il timer che tiene
 * traccia dell'ultima accensione dell'umidificatore, per poter gestire
 * il timer di refill dell'acqua.
 *
 * @return true umidificatore acceso con successo
 * @return false umidificatore già acceso, nessuna azione eseguita
 */
bool rh_turn_on();

/**
 * @brief Spegnimento dell'umidificatore.
 *
 * Questa funzione spegne l'umidificatore e il led di stato corrispondente.
 * Spegne anche la ventola di omogeneizzazione dell'aria se è possibile farlo.
 *
 * Aggiorna il timer di refill dell'acqua, aggiungendo il tempo trascorso
 * dall'ultima accensione dell'umidificatore.
 *
 * @return true umidificatore spento con successo
 * @return false umidificatore già spento, nessuna azione eseguita
 */
bool rh_turn_off();

/**
 * @brief Spegnimento della ventola di omogeneizzazione.
 *
 * Questa funzione spegne la ventola di omogeneizzazione dell'aria se vengono
 * soddisfatte le condizioni per farlo.
 *
 * @return true ventola spenta con successo
 * @return false ventola già spenta o non è possibile spegnerla
 */
bool fan_turn_off();

/**
 * @brief Accensione dell'illuminazione.
 *
 * @return true illuminazione accesa con successo
 * @return false illuminazione già accesa, nessuna azione eseguita
 */
bool light_turn_on();

/**
 * @brief Spegnimento dell'illuminazione.
 *
 * @return true illuminazione spenta con successo
 * @return false illuminazione già spenta, nessuna azione eseguita
 */
bool light_turn_off();

#endif // ACTUATORS_H
