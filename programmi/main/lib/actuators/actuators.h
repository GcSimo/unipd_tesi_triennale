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
 * @brief Accensione del riscaldatore.
 *
 * Questa funzione accende il riscaldatore e il led di stato corrispondente.
 * Accende anche la ventola ausiliaria se è spenta.
 *
 * @return true riscaldatore acceso con successo
 * @return false riscaldatore già acceso, nessuna azione eseguita
 */
bool heat_turn_on();

/**
 * @brief Spegnimento del riscaldatore.
 *
 * Questa funzione spegne il riscaldatore e il led di stato corrispondente.
 * Spegne anche la ventola ausiliaria se l'umidificatore è spento.
 *
 * @return true riscaldatore spento con successo
 * @return false riscaldatore già spento, nessuna azione eseguita
 */
bool heat_turn_off();

/**
 * @brief Accensione dell'umidificatore.
 *
 * Questa funzione accende l'umidificatore e il led di stato corrispondente.
 * Accende anche la ventola ausiliaria se è spenta.
 *
 * @return true umidificatore acceso con successo
 * @return false umidificatore già acceso, nessuna azione eseguita
 */
bool rh_turn_on();

/**
 * @brief Spegnimento dell'umidificatore.
 *
 * Questa funzione spegne l'umidificatore e il led di stato corrispondente.
 * Spegne anche la ventola ausiliaria se il riscaldatore è spento.
 *
 * @return true umidificatore spento con successo
 * @return false umidificatore già spento, nessuna azione eseguita
 */
bool rh_turn_off();

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
