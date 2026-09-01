/**
 * @file pid.cpp
 * @author Giacomo Simonetto
 * @brief Implementazione dei controllori PID.
 *
 * Implementazione delle funzioni per la gestione dei controllori PID per la
 * temperatura e l'umidità dell'incubatrice neonatale.
 */

#include "pid.h"

// inizializza i parametri per i controllori PID
void pid_init(struct pid &pid, const float &kp, const float &ki, const float &kd, const float &kw, const uint8_t &anti_windup) {
  // inizializzazione dei guadagni del PID
  pid.kp = kp;
  pid.ki = ki;
  pid.kd = kd;
  pid.kw = kw;

  // inizializzazione delle componenti del PID
  pid.proportional = 0.0f;
  pid.integral = 0.0f;
  pid.derivative = 0.0f;
  pid.output = 0.0f;

  // inizializzazione del numero di misurazioni
  pid.data_count = 0;
  pid.expected_data_count = PID_UPDATE_PERIOD / PID_DATA_PERIOD;

  // inizializzazione delle variabili e accumulatori intermedi per calcolo PID
  pid.deriv_c1 = -6.0f / pid.expected_data_count / (pid.expected_data_count + 1) / (PID_DATA_PERIOD / 1000.0f);
  pid.deriv_c2 = 12.0f / pid.expected_data_count / (pid.expected_data_count * pid.expected_data_count - 1) / (PID_DATA_PERIOD / 1000.0f);
  pid.sum1 = 0L;
  pid.sum2 = 0L;
  pid.sum3 = 0L;

  // inizializzazione del metodo di anti-windup da utilizzare
  pid.anti_windup = anti_windup;
}


// salva una nuova misurazione negli accumulatori del PID
void pid_add_data(struct pid &pid, const int16_t &error, const int16_t &measure) {

  /**
   * NOTE IMPLEMENTATIVE:
   *
   * Le grandezze richieste per aggiornare l'output sono:
   * - sum1:
   *   somma degli errori tra setpoint e valore misurato, utilizzato per
   *   la componente proporzionale, integrale e derivativa del PID
   *
   * - sum2:
   *   somma dei valori misurati, utilizzato per la componente derivativa
   *   del PID
   *
   * - sum3:
   *   somma dei prodotti tra indice della misurazione e valore misurato,
   *   utilizzato per la componente derivativa del PID
   *
   * - data_count:
   *   numero di misurazioni ricevute ed elaborate, utilizzato per il calcolo
   *   delle componenti proporzionale, integrale e derivativa del PID
   */

  pid.sum1 += error;
  pid.sum2 += measure;
  pid.sum3 += (int32_t)pid.data_count * (int32_t)measure;
  pid.data_count++;
}

// aggiorna l'output del controllore PID
float pid_update_output(struct pid &pid) {

  /**
   * NOTE IMPLEMENTATIVE:
   *
   * L'output "puro" è calcolato dalla successiva formula:
   *
   *   pid.output = pid.proportional + pid.integral + pid.derivative
   *
   * Per calcolare l'output del PID, si effettuano i seguenti passaggi:
   *  1. verifica assenza di dati da elaborare
   *  2. calcolo della componente proporzionale
   *  3. calcolo della componente integrale
   *  4. calcolo della componente derivativa
   *  5. anti-windup della componente integrale
   *  6. limitazioni dell'output
   *
   * ------------- 1. verifica dell'assenza di dati da elaborare --------------
   *
   * In caso di problemi come mancanza totale di misurazioni, l'output del PID
   * viene impostato al valore minimo, in modo da spegnere l'attuatore e non
   * far peggiorare la situazione.
   *
   *
   * --------------- 2. calcolo della componente proporzionale ----------------
   *
   * La componente proporzionale è calcolata come il prodotto tra il guadagno
   * proporzionale kp e l'errore medio del ciclo di aggiornamento del PID.
   *
   * Se ci dovessero essere problemi come mancanza di misurazioni, l'errore
   * medio calcolato con i dati disponibili è comunque una buona stima di
   * quello reale e il disturbo introdotto scompare totalmente al successivo
   * aggiornamento dell'output del PID.
   *
   *   mean_error = sum_error / data_count
   *   pid.proportional = kp * mean_error
   *
   *
   * ----------------- 3. calcolo della componente integrale ------------------
   *
   * La componente integrale è calcolata come il prodotto tra il guadagno
   * integrale ki, l'errore accumulato nel ciclo di aggiornamento del PID e
   * l'intervallo tra due misurazioni consecutive.
   *
   * Se ci dovessero essere problemi come mancanza di misurazioni, i valori
   * mancanti vengono sostituiti con l'errore medio calcolato per la parte
   * proporzionale, essendo una buona stima. Il disturbo introdotto verrà
   * propagato anche nei cicli successivi, ma verrà corretto grazie alla
   * presenza del feedback del controllore PID.
   *
   *   pid.integral = ki * mean_error * expected_data_count * (PID_DATA_PERIOD / 1000.0f)
   *
   *
   * ----------------- 4. calcolo della componente derivativa -----------------
   *
   * La componente derivativa è calcolata come il prodotto tra il guadagno
   * derivativo kd e la derivata della grandezza da controllare, calcolata
   * tramite la formula ottenuta applicando il metodo dei minimi quadrati
   * alla regressione lineare sulle misurazioni passate.
   *
   * Si calcola la derivata della grandezza da controllare e non dell'errore
   * tra setpoint e valore misurato in modo da evitare i "derivative kick"
   * che si verificano al cambiamento del setpoint.
   *
   *   pid.derivative = kd * ( -deriv_c1 * sum2 + deriv_c2 * sum3);
   *
   *                    6         N-1               12          N-1
   *   dy/dt = - -------------- * SUM  y_i + ---------------- * SUM  (i * y_i)
   *             Ts * N (N + 1)   i=0        Ts * N (N^2 - 1)   i=0
   *           \______________/   \______/   \______________/   \___________/
   *               deriv_c1         sum2        deriv_c2           sum3
   *   con:
   *    - N   = numero di dati passati
   *    - Ts  = intervallo tra due misurazioni consecutive
   *    - y_i = valore misurato i-esimo istante
   *    - i   = indice del dato passato (0 = più vecchio, N-1 = più recente)
   *
   * Siccome i due coefficienti coeff1 e coeff2 sono costanti, vengono
   * precalcolati in fase di inizializzazione e memorizzati in due apposite
   * costanti deriv_c1 e deriv_c2.
   *
   * Se ci dovessero essere problemi come mancanza di misurazioni, vengono
   * utilizzati solo i valori disponibili siccome è parecchio complesso
   * interpolare e integrare i valori mancanti all'interno delle sommatorie.
   * Inoltre il disturbo introdotto scompare totalmente all'aggiornamento
   * successivo dell'output e la regressione rimane sempre una buona stima
   * della derivata reale. L'unico svantaggio è che bisogna ricalcolare le
   * costanti deriv_c1 e deriv_c2 in funzione del numero di dati disponibili.
   * Nel caso in cui si ha un solo dato, la componente derivativa viene
   * impostata a zero.
   *
   *
   * --------------- 5. anti-windup della componente integrale ----------------
   *
   * Si implementano tre opzioni per la gestione del windup della componente
   * integrale del PID (no windup, clamping e back calculation) selezionabili
   * tramite la flag anti_windup della struct del PID:
   *
   *  0. no anti-windup:
   *      la componente integrale viene sempre sommata all'output del PID
   *
   *  1. clamping:
   *      se l'output del PID è superiore al limite massimo e l'errore è
   *      positivo, o viceversa se l'output del PID è inferiore al limite
   *      minimo e l'errore è negativo, il nuovo contributo della componente
   *      integrale viene scartato sia per il calcolo dell'output del PID che
   *      per l'accumulo dell'errore integrale
   *
   *  2. back calculation:
   *      la componente integrale viene compensata da un termine proporzionale
   *      alla differenza tra l'output puro del PID e l'output limitato in modo
   *      da far riportare l'output del PID all'interno dei limiti
   *
   *
   * ----------------------- 6. limitazioni dell'output -----------------------
   *
   * Prima di essere restituito al chiamante, l'output del PID viene limitato
   * tra le due costanti PID_MIN_OUTPUT e PID_MAX_OUTPUT.
   */

  // 1. verifica dell'assenza di dati da elaborare
  if (pid.data_count == 0) {
    pid.sum1 = 0L;
    pid.sum2 = 0UL;
    pid.sum3 = 0UL;
    return PID_MIN_OUTPUT;
  }

  // 2. calcolo della componente proporzionale
  float mean_error = (float)pid.sum1 / pid.data_count;
  pid.proportional = pid.kp * mean_error;

  // 3. calcolo del contributo della componente integrale
  float integral_contrib = pid.ki * mean_error * pid.expected_data_count * (PID_DATA_PERIOD / 1000.0f);

  // 4. calcolo della componente derivativa
  if (pid.data_count == pid.expected_data_count) {
    pid.derivative = pid.kd * (pid.deriv_c1 * pid.sum2 + pid.deriv_c2 * pid.sum3);
  } else if (pid.data_count == 1) {
    pid.derivative = 0.0f;
  } else {
    float c1 = -6.0f / pid.data_count / (pid.data_count + 1.0f) / (PID_DATA_PERIOD / 1000.0f);
    float c2 = 12.0f / pid.data_count / (pid.data_count * pid.data_count - 1.0f) / (PID_DATA_PERIOD / 1000.0f);
    pid.derivative = pid.kd * (c1 * pid.sum2 + c2 * pid.sum3);
  }

  // calcolo dell'output temporaneo del PID
  pid.output = pid.proportional + pid.integral + integral_contrib - pid.derivative;

  // 5. anti-windup della componente integrale del PID
  switch (pid.anti_windup) {
    case NO_WINDUP: // no anti-windup
      pid.integral += integral_contrib;
      break;

    case CLAMPING: // clamping
      if (!(pid.output > PID_MAX_OUTPUT && mean_error > 0.0f) && !(pid.output < PID_MIN_OUTPUT && mean_error < 0.0f))
        pid.integral += integral_contrib;
      break;

    case BACK_CALC: // back calculation
      pid.integral += integral_contrib + pid.kw * (constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT) - pid.output) * (PID_UPDATE_PERIOD / 1000.0f);
      break;
  }

  // calcolo dell'output finale del PID
  pid.output = pid.proportional + pid.integral - pid.derivative;

  // azzeramento delle variabili per il prossimo ciclo del PID
  pid.sum1 = 0L;
  pid.sum2 = 0UL;
  pid.sum3 = 0UL;
  pid.data_count = 0;

  // 6. limitazione dell'output del PID tra PID_MIN_OUTPUT e PID_MAX_OUTPUT
  return constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT);
}

// azzera gli accumulatori del PID
void pid_reset_accumulators(struct pid &pid) {
  pid.sum1 = 0L;
  pid.sum2 = 0L;
  pid.sum3 = 0L;
  pid.data_count = 0;
}
