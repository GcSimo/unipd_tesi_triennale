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
 * segnale PWM, quindi ogni PID_UPDATE_PERIOD millisecondi. Le misurazioni che
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
 * @brief Calcola l'indice con offset per accedere al buffer degli errori.
 *
 * Restituisce l'indice corretto per accedere all'elemento del buffer degli
 * errori del PID che si trova ad un certo offset rispetto all'elemento più
 * vecchio presente nel buffer.
 *
 * Tale funzione permette di semplificare lo scorrimento del buffer circolare
 * nei cicli for presenti nel calcolo dell'output del PID.
 *
 * @tparam C template parameter per scegliere il PID da utilizzare (TEMP o RH)
 * @param pid const reference alla struct del PID da cui calcolare l'indice
 * @param offset int8_t offset rispetto all'elemento più vecchio del buffer
 * @return uint8_t indice per accedere all'elemento desiderato del buffer
 */
template <uint8_t C> uint8_t error_idx_offset(const pid<C> &pid, int8_t offset) {
  constexpr uint8_t size = (C == TEMP) ? max(TEMP_PID_P_SAMPLES, TEMP_PID_I_SAMPLES) : max(RH_PID_P_SAMPLES, RH_PID_I_SAMPLES);
  return (pid.error_idx + offset % size + size) % size;
}


/**
 * @brief Calcola l'indice con offset per accedere al buffer delle misurazioni.
 *
 * Restituisce l'indice corretto per accedere all'elemento del buffer delle
 * misurazioni del PID che si trova ad un certo offset rispetto all'elemento
 * più vecchio presente nel buffer.
 *
 * Tale funzione permette di semplificare lo scorrimento del buffer circolare
 * nei cicli for presenti nel calcolo dell'output del PID.
 *
 * @tparam C template parameter per scegliere il PID da utilizzare (TEMP o RH)
 * @param pid const reference alla struct del PID da cui calcolare l'indice
 * @param offset int8_t offset rispetto all'elemento più vecchio del buffer
 * @return uint8_t indice per accedere all'elemento desiderato del buffer
 */
template <uint8_t C> uint8_t measure_idx_offset(const pid<C> &pid, int8_t offset) {
  constexpr uint8_t size = (C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES;
  return (pid.measure_idx + offset % size + size) % size;
}


/**
 * @brief Resetta i buffer del PID per l'attivazione del controllo automatico.
 *
 * Quando il controllo automatico del PID viene riattivato dopo un periodo di
 * tempo in cui il controllo è stato manuale, i buffer degli errori e delle
 * misurazioni del PID devono essere resettati per evitare che i vecchi dati
 * influenzino il calcolo dell'output del PID.
 *
 * Se venissero resettati tutti a zero, si avrebbero enormi fluttuazioni
 * nell'output dovuti alle componenti proporzionale e derivativa del PID
 * che vengono calcolate su dati non reali. Per risolvere questo problema,
 * i buffer vengono riempiti con l'errore e la misurazione passati come
 * parametri alla funzione
 *
 * A seguito del ripopolamento dei buffer si consiglia di eseguire anche un
 * aggiornamento dell'output del PID con la funzione pid_update_output() per
 * avere un output coerente con lo stato attuale del sistema al momento della
 * riattivazione del controllo automatico.
 *
 * @tparam C template parameter per scegliere il PID da utilizzare (TEMP o RH)
 * @param pid reference alla struct del PID da resettare
 * @param error errore tra setpoint e valore misurato
 * @param measure valore misurato dal sensore
 */
template <uint8_t C> void pid_reset_buffers(pid<C> &pid, int16_t error, int16_t measure) {
  // recupero dei parametri del PID
  constexpr uint8_t error_size = (C == TEMP) ? max(TEMP_PID_P_SAMPLES, TEMP_PID_I_SAMPLES) : max(RH_PID_P_SAMPLES, RH_PID_I_SAMPLES);
  constexpr uint8_t measure_size = (C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES;

  // inizializzazione dei buffer degli errori con l'errore misurato
  for (uint8_t i = 0; i < error_size; i++)
    pid.errors[i] = error;

  // inizializzazione del buffer delle misurazioni con la misura misurata
  for (uint8_t i = 0; i < measure_size; i++)
    pid.measures[i] = measure;

  // aggiornamento degli indici dei buffer
  pid.error_idx = 0;
  pid.measure_idx = 0;

  // aggiornamento della flag di inizializzazione
  pid.initialized = true;

  // reset delle componenti del PID
  pid.proportional = 0.0f;
  pid.integral = 0.0f;
  pid.derivative = 0.0f;
  pid.output = 0.0f;
}


/**
 * @brief Inizializza i parametri del PID.
 *
 * @tparam C template parameter per selezionare il PID da utilizzare (TEMP o RH)
 * @param pid reference alla struct del PID da inizializzare
 */
template <uint8_t C> void pid_init(pid<C> &pid) {
  // inizializzazione delle componenti del PID
  pid.proportional = 0.0f;
  pid.integral = 0.0f;
  pid.derivative = 0.0f;
  pid.output = 0.0f;

  // inizializzazione buffer e indici per errori e misurazioni passate
  pid.error_idx = 0;
  pid.measure_idx = 0;

  // inizializzazione flag per primi dati forniti
  pid.initialized = false;
}


/**
 * @brief Salva una nuova misurazione nei buffer del PID.
 *
 * Salva l'errore e la misurazione nei buffer interni del PID che verranno
 * utilizzati per calcolare l'output del PID alla successiva invocazione
 * della funzione pid_update_output().
 *
 * Si assume che i dati forniti alla funzione vengano acquisiti dai sensori
 * ad intervalli di tempo costanti ogni PID_DATA_PERIOD millisecondi.
 * All'interno della funzione non si effettua nessun controllo temporale per
 * cui il rispetto di tale condizione è responsabilità del chiamante.
 *
 * Non ci sono vincoli sul tempo che intercorre da quando i dati vengono
 * acquisiti dal sensore e quando vengono passati alla funzione.
 *
 * Se è la prima volta che viene invocata la funzione dall'avvio del programma
 * o a seguito di una richiesta di reset dei buffer del PID, i buffer vengono
 * riempiti con l'errore e la misurazione passati come parametri. In questo
 * modo i dati sono aggiornati con lo stato del sistema e l'output del PID
 * sarà più stabile e realistico.
 *
 * --------------------------- NOTE IMPLEMENTATIVE ----------------------------
 *
 * I buffer sono gestiti come array circolari in cui i dati sono ordinati dal
 * più vecchio al più recente al crescere dell'indice.
 *
 * Gli indici "error_idx" e "measure_idx" puntano all'elemento più vecchio
 * presente nel buffer, l'elemento più recente è quello all'indice precedente.
 *
 * @tparam C template parameter per scegliere il PID da utilizzare (TEMP o RH)
 * @param pid reference alla struct del PID da aggiornare
 * @param error errore tra setpoint e valore misurato
 * @param measure valore misurato dal sensore
 */
template <uint8_t C> void pid_add_data(pid<C> &pid, int16_t error, int16_t measure) {
  // inizializza i buffer se è la prima invocazione
  if (!pid.initialized) {
    pid_reset_buffers<C>(pid, error, measure);
  }

  // inserisce il nuovo valore se i buffer sono già stati inizializzati
  else {
    // recupero dei parametri del PID
    constexpr uint8_t error_size = (C == TEMP) ? max(TEMP_PID_P_SAMPLES, TEMP_PID_I_SAMPLES) : max(RH_PID_P_SAMPLES, RH_PID_I_SAMPLES);
    constexpr uint8_t measure_size = (C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES;

    // inserimento dei nuovi dati al posto del dato più vecchio
    pid.errors[pid.error_idx] = error;
    pid.measures[pid.measure_idx] = measure;

    // aggiornamento degli indici
    pid.error_idx = (pid.error_idx + 1) % error_size;
    pid.measure_idx = (pid.measure_idx + 1) % measure_size;
  }
}


/**
 * @brief Aggiorna l'output del controllore PID.
 *
 * Calcola il nuovo output del PID in base ai dati salvati nei buffer interni
 * del pid attraverso la funzione pid_add_data().
 *
 * Si prevede che l'output del PID venga aggiornato ad intervalli di tempo
 * costanti ogni PID_UPDATE_PERIOD millisecondi. All'interno della funzione
 * non si effettua nessun controllo temporale per cui il rispetto di tale
 * condizione è responsabilità del chiamante.
 *
 * Se il pid non contiene dati sufficienti per calcolare l'output, la funzione
 * restituisce 0 senza effettuare alcun calcolo. Si consiglia, quindi, di
 * invocare sempre la funzione pid_add_data() prima di aggiornare l'output
 * con pid_update_output().
 *
 * --------------------------- NOTE IMPLEMENTATIVE ----------------------------
 *
 * Per calcolare l'output del PID, si effettuano i seguenti passaggi:
 *  1. calcolo della componente proporzionale
 *  2. calcolo della componente integrale
 *  3. calcolo della componente derivativa
 *  4. anti-windup della componente integrale
 *  5. limitazioni dell'output
 *
 * --------------- 1. calcolo della componente proporzionale ----------------
 *
 * La componente proporzionale è calcolata come il prodotto tra il guadagno
 * proporzionale kp e l'errore medio calcolato sugli ultimi "p_samples" errori
 * più recenti, salvati nel buffer degli errori del PID.
 *
 *   mean_error = sum of last p_samples errors / p_samples
 *   pid.proportional = kp * mean_error
 *
 *
 * ----------------- 2. calcolo della componente integrale ------------------
 *
 * La componente integrale è calcolata come il prodotto tra il guadagno
 * integrale ki, l'integrale dell'errore calcolata sugli ultimi "i_samples"
 * errori più recenti, salvati nel buffer degli errori del PID.
 *
 * L'integrale dell'errore si calcola come la somma degli "i_samples" errori
 * più recenti, moltiplicata per il tempo di campionamento dei dati definito
 * come PID_DATA_PERIOD convertito in secondi.
 *
 * Il numero di campioni "i_samples" corrisponde al numero di misurazioni che
 * vengono effettuate tra un aggiornamento e l'altro dell'output del PID e
 * viene calcolato automaticamente nel file di configurazione.
 *
 *   sum_integral = sum of last i_samples errors * PID_DATA_PERIOD / 1000.0f
 *   pid.integral = ki * sum_integral
 *
 *
 * ----------------- 3. calcolo della componente derivativa -----------------
 *
 * La componente derivativa è calcolata come il prodotto tra il guadagno
 * derivativo kd e la derivata della grandezza da controllare, calcolata
 * tramite la formula ottenuta applicando il metodo dei minimi quadrati
 * alla regressione lineare sulle misurazioni.
 *
 *
 *  pid.derivative = kd * ( -deriv_c1 * sum1 + deriv_c2 * sum2);
 *
 *                   6         N-1                 12          N-1
 *  dy/dt = - -------------- * SUM  y_i  +  ---------------- * SUM  (i * y_i)
 *            Ts * N (N + 1)   i=0          Ts * N (N^2 - 1)   i=0
 *          \______________/   \______/     \______________/   \___________/
 *              deriv_c1         sum1           deriv_c2           sum2
 * con:
 *  - N   = numero di dati passati considerati (d_samples)
 *  - Ts  = periodo di campionamento dei dati (PID_DATA_PERIOD)
 *  - y_i = valore misurato i-esimo istante
 *  - i   = indice del dato passato (0 = più vecchio, N-1 = più recente)
 *
 * Siccome i due coefficienti davanti alle sommatorie sono costanti, vengono
 * precalcolati in fase di compilazione e memorizzati in due apposite costanti
 * deriv_c1 e deriv_c2.
 *
 * È stato scelto di calcolare la derivata sulle misurazioni della grandezza
 * da controllare e non dell'errore tra setpoint e valore misurato in modo da
 * evitare i "derivative kick" al cambiamento del setpoint.
 *
 *
 * --------------- 4. anti-windup della componente integrale ----------------
 *
 * Si implementano tre opzioni per la gestione del windup della componente
 * integrale del PID (no windup, clamping e back calculation) selezionabili
 * tramite la costante di configurazione PID_WINDUP.
 *
 *  0. NO_WINDUP:
 *      la componente integrale viene sempre aggiornata e sommata all'output
 *      del PID provocando windup in caso di saturazione
 *
 *  1. CLAMPING:
 *      se l'output del PID è superiore al limite massimo e l'errore è
 *      positivo, o viceversa se l'output del PID è inferiore al limite
 *      minimo e l'errore è negativo, il nuovo contributo della componente
 *      integrale viene scartato sia per il calcolo dell'output del PID che
 *      per l'aggiornamento della componente integrale
 *
 *  2. BACK_CALCULATION:
 *      la componente integrale viene compensata dalla differenza tra l'output
 *      puro del PID e l'output limitato, pesata per il guadagno kw, in modo
 *      da far riportare l'output del PID all'interno dei limiti stabiliti
 *
 *
 * --------------- 5. calco dell'output e opportune limitazioni ---------------
 *
 * L'output del PID viene calcolato come la somma delle tre componenti del PID
 * (proporzionale, integrale e derivativa) e di un ulteriore contributo detto
 * bias, che corrisponde all'output che ci si aspetta di avere in condizioni
 * di equilibrio per mantenere la grandezza da controllare al setpoint.
 *
 * Prima di essere restituito, l'output del pid viene limitato tra le due
 * costanti PID_MIN_OUTPUT e PID_MAX_OUTPUT.
 *
 * @tparam C template parameter per scegliere il PID da utilizzare (TEMP o RH)
 * @param pid reference alla struct del PID da aggiornare
 * @return float output del PID limitato tra PID_MIN_OUTPUT e PID_MAX_OUTPUT
 */
template <uint8_t C> float pid_update_output(pid<C> &pid) {
  // verifica che il PID sia stato inizializzato, altrimenti restituisce 0.0f
  if (!pid.initialized)
    return 0.0f;

  // recupero dei parametri del PID
  // NOTA: le costanti sono divise per 100, siccome le misure del sensore sono
  // state moltiplicate per 100 per memorizzarle in interi con 2 cifre decimali
  constexpr float bias = (C == TEMP) ? TEMP_PID_BIAS : RH_PID_BIAS;
  constexpr float kp = ((C == TEMP) ? TEMP_PID_KP : RH_PID_KP) / 100.0f;
  constexpr float ki = ((C == TEMP) ? TEMP_PID_KI : RH_PID_KI) / 100.0f;
  constexpr float kd = ((C == TEMP) ? TEMP_PID_KD : RH_PID_KD) / 100.0f;
  constexpr float kw = ((C == TEMP) ? TEMP_PID_KW : RH_PID_KW) / 100.0f;
  constexpr float min_integral = (C == TEMP) ? TEMP_MIN_INTEGRAL : RH_MIN_INTEGRAL;
  constexpr float max_integral = (C == TEMP) ? TEMP_MAX_INTEGRAL : RH_MAX_INTEGRAL;
  constexpr uint8_t p_samples = (C == TEMP) ? TEMP_PID_P_SAMPLES : RH_PID_P_SAMPLES;
  constexpr uint8_t i_samples = (C == TEMP) ? TEMP_PID_I_SAMPLES : RH_PID_I_SAMPLES;
  constexpr uint8_t d_samples = (C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES;
  constexpr float deriv_c1 = -6.0f / d_samples / (d_samples + 1.0f) / (PID_DATA_PERIOD / 1000.0f);
  constexpr float deriv_c2 = 12.0f / d_samples / (d_samples * d_samples - 1.0f) / (PID_DATA_PERIOD / 1000.0f);
  constexpr uint8_t windup = (C == TEMP) ? TEMP_PID_WINDUP : RH_PID_WINDUP;

  // --------------- 1. calcolo della componente proporzionale ----------------
  // somma dei "p_samples" errori passati
  float mean_error = 0.0f;
  for (uint8_t i = 0; i < p_samples; i++)
    mean_error += pid.errors[error_idx_offset(pid, -(i+1))];

  // aggiornamento della componente proporzionale del PID
  pid.proportional = kp * mean_error / p_samples;

  // ---------- 2. calcolo del contributo della componente integrale ----------
  // somma degli "i_samples" errori passati
  float sum_integral = 0.0f;
  for (uint8_t i = 0; i < i_samples; i++)
    sum_integral += pid.errors[error_idx_offset(pid, -(i+1))];

  // aggiornamento della componente integrale del PID
  float integral_contrib = ki * sum_integral * (PID_DATA_PERIOD / 1000.0f);

  // ----------------- 3. calcolo della componente derivativa -----------------
  // somma delle "d_samples" misure passate
  float sum1 = 0.0f;
  float sum2 = 0.0f;
  for (uint8_t i = 0; i < d_samples; i++) {
    sum1 += pid.measures[measure_idx_offset(pid, i)];
    sum2 += (int32_t)i * (int32_t)pid.measures[measure_idx_offset(pid, i)];
  }

  // aggiornamento della componente derivativa del PID
  pid.derivative = kd * (deriv_c1 * sum1 + deriv_c2 * sum2);

  // ----------- 4. anti-windup della componente integrale del PID ------------
  // calcolo dell'output ipotetico del PID per le verifiche dell'anti-windup
  pid.output = bias + pid.proportional + pid.integral + integral_contrib - pid.derivative;

  // gestione del windup della componente integrale
  if (windup == 0) { // no anti-windup
    pid.integral += integral_contrib;
  } else if (windup == 1) { // clamping
    if (!(pid.output > PID_MAX_OUTPUT && mean_error > 0.0f) && !(pid.output < PID_MIN_OUTPUT && mean_error < 0.0f))
      pid.integral += integral_contrib;
  } else if (windup == 2) { // limiting
      pid.integral += integral_contrib;
      pid.integral = constrain(pid.integral, min_integral, max_integral);
  } else if (windup == 3) { // back calculation
    pid.integral += integral_contrib + kw * (constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT) - pid.output) * (PID_UPDATE_PERIOD / 1000.0f);
  }

  // -------------- 5. calco dell'output e opportune limitazioni --------------
  // ricalcolo dell'output finale del PID
  pid.output = bias + pid.proportional + pid.integral - pid.derivative;

  // limitazione dell'output del PID tra PID_MIN_OUTPUT e PID_MAX_OUTPUT
  return constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT);
}

#endif // PID_H
