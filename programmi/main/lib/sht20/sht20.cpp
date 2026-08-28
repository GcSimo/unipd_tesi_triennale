/**
 * @file sht20.cpp
 * @author Giacomo Simonetto
 * @brief Gestione del sensore SHT20
 *
 * Implementazione delle funzioni per la gestione del sensore SHT20.
 */

#include "sht20.h"

// ----------------------------------------------------------------------------
// ------------------ gestione dello stato del sensore SHT20 ------------------
// ----------------------------------------------------------------------------

/**
 * @brief Gestione dello stato del sensore SHT20.
 *
 * Il sensore viene trattato come una macchina con i seguenti 6 stati finiti
 * che si susseguono in sequenza ciclica, con il seguente ordine:
 *
 * 0. SHT20_IDLE:
 *   - il sensore è in pausa e non è in corso alcuna lettura
 *   - passa a SHT20_READY_FOR_TEMP_REQ dopo 1 secondo dall'ultima lettura
 *
 * 1. SHT20_READY_FOR_TEMP_REQ:
 *   - il sensore è pronto per ricevere la richiesta di lettura della temperatura
 *   - passa a SHT20_WAIT_TEMP dopo aver inviato la richiesta di lettura della temperatura
 *
 * 2. SHT20_WAIT_TEMP:
 *    - il sensore è in attesa della risposta alla richiesta di lettura della temperatura
 *    - passa a SHT20_READY_FOR_RH_REQ dopo aver ricevuto la risposta
 *
 * 3. SHT20_READY_FOR_RH_REQ:
 *    - il sensore è pronto per ricevere la richiesta di lettura dell'umidità
 *    - passa a SHT20_WAIT_RH dopo aver inviato la richiesta di lettura dell'umidità
 *
 * 4. SHT20_WAIT_RH:
 *    - il sensore è in attesa della risposta alla richiesta di lettura dell'umidità
 *    - passa a SHT20_NEW_DATA dopo aver ricevuto la risposta
 *
 * 5. SHT20_NEW_DATA:
 *    - il sensore ha nuovi dati disponibili per la temperatura e l'umidità
 *    - passa a SHT20_TEMP_READ o SHT20_RH_READ dopo aver letto uno dei due valori
 *
 * 6. SHT20_TEMP_READ:
 *    - la temperatura è stata letta dal sensore
 *    - passa a SHT20_IDLE dopo aver letto anche l'umidità
 *
 * 7. SHT20_RH_READ:
 *    - l'umidità è stata letta dal sensore
 *    - passa a SHT20_IDLE dopo aver letto anche la temperatura
 */

// stati del sensore SHT20
#define SHT20_IDLE 0               // sensore in pausa
#define SHT20_READY_FOR_TEMP_REQ 1 // pronto per richiedere la temperatura
#define SHT20_WAIT_TEMP 2          // in attesa di ricevere la temperatura
#define SHT20_READY_FOR_RH_REQ 3   // pronto per richiedere l'umidità
#define SHT20_WAIT_RH 4            // in attesa di ricevere l'umidità
#define SHT20_NEW_DATA 5           // nuovi dati disponibili dal sensore
#define SHT20_TEMP_READ 6         // temperatura letta dal sensore
#define SHT20_RH_READ 7           // umidità letta dal sensore


// ----------------------------------------------------------------------------
// ------------------- implementazione della classe sht20 ---------------------
// ----------------------------------------------------------------------------

// costruttore della classe sht20
sht20::sht20(uint32_t p) {
  sensor = SHT2x();   // inizializzazione oggetto della classe SHT2x
  state = SHT20_IDLE; // stato del sensore SHT20
  read_period = p;    // periodo di lettura del sensore SHT20
  read_timer = 0U;    // timer per le letture del sensore SHT20
  error = 0;          // codice di errore del sensore SHT20
}

// inizializzazione del sensore SHT20
bool sht20::begin() {
  return sensor.begin();
}

// aggiorna lo stato del sensore SHT20
bool sht20::update() {
  switch (state) {
    // --- 0. sensore in attesa
    case SHT20_IDLE:
      // verifica se è trascorso il periodo di lettura del sensore
      if (millis() - read_timer >= read_period) {
        state = SHT20_READY_FOR_TEMP_REQ; // aggiornamento stato
        read_timer = millis(); // aggiornamento timer richiesta
      }

      // nessun nuovo dato disponibile
      return false;

    // --- 1. sensore pronto per inviare la richiesta di lettura della temperatura
    case SHT20_READY_FOR_TEMP_REQ:
      // invio richiesta al sensore e verifica errori
      if (!sensor.requestTemperature()) {
        error = sensor.getError(); // recupero codice di errore
        return false; // nessun nuovo dato disponibile
      }

      state = SHT20_WAIT_TEMP; // aggiornamento stato
      return false;  // nessun nuovo dato disponibile

    // --- 2. sensore in attesa della risposta alla richiesta di lettura della temperatura
    case SHT20_WAIT_TEMP:
      // verifica presenza di nuovi dati disponibili per la temperatura
      // reqTempReady() non lancia errori (controlla solo un timer interno)
      if (sensor.reqTempReady()) {

        // recupero temperatura dal sensore e verifica errori
        if (!sensor.readTemperature()) {
          error = sensor.getError(); // recupero codice di errore
          return false; // nessun nuovo dato disponibile
        }

        // aggiornamento stato
        state = SHT20_READY_FOR_RH_REQ;
      }
      return false; // nessun nuovo dato disponibile

    // --- 3. sensore pronto per inviare la richiesta di lettura dell'umidità
    case SHT20_READY_FOR_RH_REQ:
      // invio richiesta al sensore e verifica errori
      if (!sensor.requestHumidity()) {
        error = sensor.getError(); // recupero codice di errore
        return false; // nessun nuovo dato disponibile
      }

      state = SHT20_WAIT_RH; // aggiornamento stato
      return false; // nessun nuovo dato disponibile

    // --- 4. sensore in attesa della risposta alla richiesta di lettura dell'umidità
    case SHT20_WAIT_RH:
      // verifica presenza di nuovi dati disponibili per l'umidità
      // reqHumReady() non lancia errori (controlla solo un timer interno)
      if (sensor.reqHumReady()) {

        // recupero umidità dal sensore e verifica errori
        if (!sensor.readHumidity()) {
          error = sensor.getError(); // recupero codice di errore
          return false; // nessun nuovo dato disponibile
        }

        // aggiornamento stato
        state = SHT20_NEW_DATA;
      }
      return false; // nessun nuovo dato disponibile

    // --- 5. sensore ha nuovi dati disponibili per la temperatura e l'umidità
    case SHT20_NEW_DATA:
      // l'aggiornamento dello stato avviene dopo la lettura dei dati
      // tramite le funzioni get_temperature() e get_humidity()

      // nuovi dati disponibili per la temperatura e l'umidità
      return true;

    // --- x. stato non valido, non si verifica mai
    default:
      return false;
  }
}

// restituisce la temperatura misurata dal sensore SHT20
float sht20::get_temperature() {
  // aggiornamento stato dopo aver letto solo la temperatura
  if (state == SHT20_NEW_DATA)
    state = SHT20_TEMP_READ;

  // aggiornamento stato dopo aver letto temperatura e umidità
  else if (state == SHT20_RH_READ)
    state = SHT20_IDLE;

  // stato non valido per la lettura della temperatura
  else if (state != SHT20_TEMP_READ)
    return NAN;

  // restituisce la temperatura misurata dal sensore SHT20
  // getTemperature() non lancia errori, fa solo calcoli matematici
  return sensor.getTemperature();
}

// restituisce l'umidità misurata dal sensore SHT20
float sht20::get_humidity() {
  // aggiornamento stato dopo aver letto solo l'umidità
  if (state == SHT20_NEW_DATA)
    state = SHT20_RH_READ;

  // aggiornamento stato dopo aver letto temperatura e umidità
  else if (state == SHT20_TEMP_READ)
    state = SHT20_IDLE;

  // stato non valido per la lettura dell'umidità
  else if (state != SHT20_RH_READ)
    return NAN;

  // restituisce l'umidità misurata dal sensore SHT20
  // getHumidity() non lancia errori, fa solo calcoli matematici
  return sensor.getHumidity();
}

// restituisce il codice di errore del sensore SHT20
int sht20::get_error() {
  return error;
}
