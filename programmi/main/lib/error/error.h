/**
 * @file error.h
 * @author Giacomo Simonetto
 * @brief Gestione degli errori dell'incubatrice neonatale
 *
 * Dichiarazione delle funzioni e dei flag necessari per il controllo degli
 * errori dell'incubatrice neonatale.
 */

#ifndef ERROR_H
#define ERROR_H

#include "config.h"

/**
 * @brief Gestione degli errori dell'incubatrice neonatale.
 *
 * Gli errori si dividono in due categorie:
 *  - errori senza timer - non temporizzati:
 *    errori che rimangono attivi fino a quando non vengono risolti, hanno
 *    specifiche condizioni di attivazione e di disattivazione, ad esempio
 *    gli errori di temperatura e umidità fuori dai range di sicurezza
 *
 *  - errori con timer - temporizzati:
 *    errori che vengono impostati quando si verifica un evento specifico
 *    e rimangono attivi per uno specifico tempo limitato, dato da un timer
 *    ad esempio gli errori di overflow nella stampa dei valori di temperatura
 *    e umidità su display lcd e serial monitor
 *
 * Gli errori sono gestiti tramite un codice di stato a 16 bit, dove ogni bit
 * rappresenta un errore specifico. I primi 10 bit sono riservati agli errori
 * non temporizzati, mentre gli ultimi 6 bit sono riservati agli errori
 * temporizzati. I bit sono definiti come segue:
 *
 * --- errori non temporizzati ---
 *
 * - bit0:  temperatura troppo bassa (1)
 *          la temperatura misurata dal sensore SHT20 è inferiore al range di
 *          sicurezza impostato dal setpoint di temperatura e dalla soglia di
 *          errore TEMP_ERR_THLD
 *
 * - bit1:  temperatura troppo alta (2)
 *          la temperatura misurata dal sensore SHT20 è superiore al range di
 *          di sicurezza impostato dal setpoint di temperatura e dalla soglia
 *          di errore TEMP_ERR_THLD
 *
 * - bit2:  umidità troppo bassa (3)
 *          l'umidità misurata dal sensore SHT20 è inferiore al range di
 *          sicurezza impostato dal setpoint di umidità e dalla soglia di
 *          di errore RH_ERR_THLD
 *
 * - bit3:  umidità troppo alta (4)
 *          l'umidità misurata dal sensore SHT20 è superiore al range di
 *          sicurezza impostato dal setpoint di umidità e dalla soglia di
 *          errore RH_ERR_THLD
 *
 * - bit4:  overflow nella lettura della temperatura (5)
 *          la temperatura misurata dal sensore SHT20 è minore di 0°C
 *          o superiore a 100°C, assumerà il valore valido più vicino:
 *          0°C se negativa o 99.99°C se maggiore di 100°C
 *
 * - bit5:  overflow nella lettura dell'umidità (6)
 *          l'umidità misurata dal sensore SHT20 è minore di 0% o maggiore
 *          di 100%, assumerà il valore valido più vicino: 0% se negativa
 *          o 99.99% se maggiore di 100%
 *
 * - bit6:  non utilizzato (7)
 *
 * - bit7:  non utilizzato (8)
 *
 * - bit8:  non utilizzato (9)
 *
 * - bit9:  non utilizzato (10)
 *
 *
 * --- errori temporizzati ---
 *
 * - bit10: errore di comunicazione con il sensore SHT20 (11)
 *          si sono verificati problemi di comunicazione con il sensore SHT20,
 *          ad esempio quando il sensore non risponde alle richieste I2C
 *
 * - bit11: overflow nella stampa dei valori (12)
 *          il valore di temperatura o umidità da stampare su display lcd
 *          o serial monitor è minore di 0 o maggiore di 100, verrà stampata
 *          la stringa "xx.xx" al posto del valore reale
 *
 * - bit12: non utilizzato (13)
 *
 * - bit13: non utilizzato (14)
 *
 * - bit14: non utilizzato (15)
 *
 * - bit15: non utilizzato (16)
 */

// ----------------------------------------------------------------------------
// ----------------------- definizione codici di errore -----------------------
// ----------------------------------------------------------------------------

// codici di errore non temporizzati
#define ERR_LOW_TEMP uint8_t(0)        // temperatura troppo bassa
#define ERR_HIGH_TEMP uint8_t(1)       // temperatura troppo alta
#define ERR_LOW_RH uint8_t(2)          // umidità troppo bassa
#define ERR_HIGH_RH uint8_t(3)         // umidità troppo alta
#define ERR_TEMP_OVERFLOW uint8_t(4)   // overflow nella lettura della temperatura
#define ERR_RH_OVERFLOW uint8_t(5)     // overflow nella lettura dell'umidità
#define ERR_UNUSED1 uint8_t(6)         // non utilizzato
#define ERR_UNUSED2 uint8_t(7)         // non utilizzato
#define ERR_UNUSED3 uint8_t(8)         // non utilizzato
#define ERR_UNUSED4 uint8_t(9)         // non utilizzato

// codici di errore temporizzati
#define ERR_SHT20 uint8_t(10)          // errore sensore SHT20
#define ERR_PRINT_OVERFLOW uint8_t(11) // overflow nella stampa dei valori
#define ERR_UNUSED5 uint8_t(12)        // non utilizzato
#define ERR_UNUSED6 uint8_t(13)        // non utilizzato
#define ERR_UNUSED7 uint8_t(14)        // non utilizzato
#define ERR_UNUSED8 uint8_t(15)        // non utilizzato


// ----------------------------------------------------------------------------
// ----------------------- definizione timer errori ---------------------------
// ----------------------------------------------------------------------------

// tempo di durata degli errori temporizzati, in ms
#define ERR_TIMER uint32_t(1100)

// array dei timer per gli errori temporizzati, in millisecondi
extern uint32_t err_timers[6];

// ----------------------------------------------------------------------------
// ----------------------- dichiarazione funzioni errori ----------------------
// ----------------------------------------------------------------------------

/**
 * @brief Imposta un errore nel codice di stato.
 *
 * @param error_code flag del codice di errore da impostare
 * @return true errore impostato correttamente
 * @return false errore già presente, nessuna azione eseguita
 */
void err_set(uint8_t error_code);

/**
 * @brief Rimuove un errore dal codice di stato.
 *
 * @param error_code flag del codice di errore da rimuovere
 * @return true errore rimosso correttamente
 * @return false errore non presente, nessuna azione eseguita
 */
void err_rm(uint8_t error_code);

/**
 * @brief Verifica se un errore è presente nel codice di stato.
 *
 * @param error_code flag del codice di errore da verificare
 * @return true errore presente nel codice di stato
 * @return false errore non presente nel codice di stato
 */
bool err_check(uint8_t error_code);

/**
 * @brief Aggiorna lo stato del led di allarme.
 *
 * Il led di allarme viene acceso se è presente almeno un errore nel codice
 * di stato, altrimenti viene spento.
 */
void update_alarm_led();

/**
 * @brief Verifica se ci sono errori temporizzati da rimuovere.
 *
 * Verifica ogni timer degli errori temporizzati presenti
 */
void update_temporized_errors();

/**
 * @brief Controlla se la temperatura misurata rientra nel range di sicurezza.
 *
 * Verifica se la temperatura misurata rientra nel range di sicurezza definito
 * dal setpoint di temperatura e dalla soglia di errore TEMP_ERR_THLD.
 */
void check_temp_range();

/**
 * @brief Controlla se l'umidità misurata rientra nel range di sicurezza.
 *
 * Verifica se l'umidità misurata rientra nel range di sicurezza definito
 * dal setpoint di umidità e dalla soglia di errore RH_ERR_THLD.
 */
void check_rh_range();

#endif // ERROR_H
