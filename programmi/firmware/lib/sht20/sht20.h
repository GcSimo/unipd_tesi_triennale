/**
 * @file sht20.h
 * @author Giacomo Simonetto
 * @brief Gestione del sensore SHT20
 *
 * Dichiarazione delle funzioni per la gestione del sensore SHT20.
 */

#ifndef SHT20_H
#define SHT20_H

#include <Arduino.h>
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
 * Per un corretto funzionamento del timer di lettura del sensore SHT20, è
 * necessario resettare il timer di lettura del sensore tramite la funzione
 * init_timer() prima della prima lettura del sensore o se il sensore non è
 * stato utilizzato per un periodo di tempo superiore al periodo di lettura
 * impostato. La prima lettura successiva avverrà dopo il periodo di lettura
 * impostato dall'invocazione di init_timer().
 */
class sht20 {
  public:
    /**
     * @brief Costruttore della classe sht20.
     *
     * Riceve come parametro opzionale il tempo di campionamento, ovvero
     * l'intervallo di tempo tra due letture consecutive del sensore, espresso
     * in millisecondi. Di default viene impostato a 1000 ms (1 secondo).
     * Il datasheet consiglia di non scendere sotto al secondo per evitare
     * fenomeni di self-heating del sensore.
     *
     * È possibile indicare, sempre come parametro opzionale, il puntatore
     * all'oggetto TwoWire da utilizzare per la comunicazione I2C. Questo
     * è utile nel caso in cui il sensore sia attaccato ad un bus I2C diverso
     * da quello di default, ad esempio se in quello di default sono presenti
     * altri dispositivi I2C con lo stesso indirizzo (es. un altro SHT20).
     *
     * @param read_period periodo di lettura del sensore
     * @param wire puntatore all'oggetto TwoWire per la comunicazione I2C
     */
    sht20(uint32_t read_period = 1000, TwoWire *wire = &Wire);

    /**
     * @brief Inizializzazione del sensore SHT20.
     *
     * @return true se l'inizializzazione è andata a buon fine
     * @return false se si è verificato un errore
     */
    bool begin();

    /**
     * @brief Inizializza il timer per la lettura del sensore SHT20.
     *
     * La funzione inizializza il timer per la lettura del sensore SHT20,
     * in modo da eliminare eventuali offset temporali precedenti alla prima
     * lettura o dovuti ad un periodo di inattività del sensore superiore al
     * periodo di lettura impostato.
     *
     * La lettura successiva avverrà alla prima successiva invocazione della
     * funzione update().
     */
    void init_timer() {
      read_timer = millis() - read_period;
    }

    /**
     * @brief Effettua l'aggiornamento dello stato del sensore SHT20.
     *
     * La funzione gestisce in autonomia le operazioni da eseguire in base
     * allo stato del sensore, per cui va invocata ad ogni iterazione del
     * loop principale.
     *
     * Se non sono disponibili nuovi dati per la temperatura e l'umidità, la
     * funzione restituisce false ed è possibile verificare se si è verificato
     * un errore invocando la funzione get_error(). L'errore viene resettato
     * a 0 al successivo aggiornamento con successo dello stato del sensore,
     * per cui è necessario leggere il codice di errore prima di invocare
     * nuovamente la funzione update().
     *
     * Se sono disponibili nuovi dati per la temperatura e l'umidità, la
     * funzione restituisce true ed è necessario leggere entrambi i valori
     * il prima possibile, altrimenti non verranno effettuate nuove richieste
     * di lettura di nuovi dati.
     *
     * @return true nuovi dati disponibili per la temperatura e l'umidità
     * @return false nessun nuovo dato disponibile o si è verificato un errore
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
     * Il codice di errore è disponibile fino alla successiva invocazione
     * con successo della funzione update(), che lo resetta a 0. Quindi per
     * verificare l'esito di una invocazione di update() è necessario leggere
     * il codice di errore prima di invocare nuovamente tale funzione.
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
