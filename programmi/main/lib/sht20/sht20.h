/**
 * @file sht20.h
 * @author Giacomo Simonetto
 * @brief Gestione del sensore SHT20
 *
 * Dichiarazione delle funzioni per la gestione del sensore SHT20.
 */

#ifndef SHT20_H
#define SHT20_H

#include "Arduino.h"
#include <SHT2x.h>

/**
 * @brief Classe per la gestione non bloccante del sensore SHT20.
 *
 * Per un corretto funzionamento, è necessario invocare la funzione update().
 * ad ogni iterazione del loop principale. Tale funzione gestisce in maniera
 * automatica le operazioni da eseguire in base allo stato del sensore.
 *
 * Al termine di ogni invocazione di update() in base al valore restituito
 * da tale funzione, si consigliano le seguenti azioni:
 *
 * - false: non sono disponibili nuovi dati o si è verificato un errore,
 *          è consigliato verificare il codice di errore tramite get_error()
 *
 * - true: sono disponibili nuovi dati per la temperatura e l'umidità,
 *         procedere con la lettura di entrambi i valori tramite le funzioni
 *         get_temperature() e get_humidity() il prima possibile, non verrà
 *         effettuata la richiesta di nuovi dati finché quelli disponibili
 *         non saranno stati letti
 *
 */
class sht20 {
  public:
    /**
     * @brief Costruttore della classe sht20.
     *
     * Riceve come parametro il periodo di lettura del sensore SHT20, espresso
     * in millisecondi. Di default viene impostato a 1000 ms (1 secondo).
     *
     * Dal datasheet è sconsigliato scendere sotto al secondo per evitare
     * fenomeni di self-heating del sensore.
     *
     * @param read_period periodo di lettura del sensore
     */
    sht20(uint32_t read_period = 1000);

    /**
     * @brief Inizializzazione del sensore SHT20.
     *
     * @return true se l'inizializzazione è andata a buon fine
     * @return false se si è verificato un errore
     */
    bool begin();

    /**
     * @brief Effettua l'aggiornamento dello stato del sensore SHT20.
     *
     * @return true nuovi dati disponibili per la temperatura e l'umidità
     * @return false nessun nuovo dato disponibile
     */
    bool update();

    /**
     * @brief Restituisce la temperatura misurata dal sensore SHT20.
     *
     * @return float temperatura misurata in gradi Celsius
     */
    float get_temperature();

    /**
     * @brief Restituisce l'umidità misurata dal sensore SHT20.
     *
     * @return float umidità misurata in percentuale
     */
    float get_humidity();

    /**
     * @brief Restituisce il codice di errore del sensore SHT20.
     *
     * @return int codice di errore
     */
    int get_error();

  private:
    SHT2x sensor; // oggetto della classe SHT2x per la gestione del sensore SHT20
    uint32_t read_period; // periodo di lettura del sensore SHT20
    uint32_t read_timer; // timer per le letture del sensore SHT20
    uint16_t error; // codice di errore del sensore SHT20
    uint8_t state; // stato del sensore SHT20
};

#endif // SHT20_H
