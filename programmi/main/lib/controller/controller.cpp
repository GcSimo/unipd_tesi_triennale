/**
 * @file controller.cpp
 * @author Giacomo Simonetto
 * @brief Controllo degli attuatori dell'incubatrice neonatale
 *
 * Implementazione delle funzioni dei controllori ad isteresi ON/OFF e PID
 * per la gestione degli attuatori dell'incubatrice neonatale.
 */

#include "controller.h"
#include "utils.h"

// inizializza i parametri per i controllori PID
void ctrl_begin() {
  #if TEMP_CTRL == 2 // PID della temperatura
    temp_pid.kp = TEMP_PID_KP;
    temp_pid.ki = TEMP_PID_KI * (CTRL_PWM_PERIOD / 1000.0f);
    temp_pid.kd = TEMP_PID_KD / (CTRL_PWM_PERIOD / 1000.0f);
    temp_pid.kw = TEMP_PID_KW * (CTRL_PWM_PERIOD / 1000.0f);
    temp_pid.proportional = 0.0f;
    temp_pid.integral = 0.0f;
    temp_pid.derivative = 0.0f;
    temp_pid.output = 0.0f;
    temp_pid.sum1 = 0UL;
    temp_pid.sum2 = 0UL;
    temp_pid.sum3 = 0UL;
    temp_pid.data_count = 0;
    temp_pid.expected_data_count = CTRL_PWM_PERIOD / SHT20_READ_PERIOD;
    temp_pid.deriv_c1 = -6.0f / temp_pid.expected_data_count / (temp_pid.expected_data_count + 1);
    temp_pid.deriv_c2 = 12.0f / temp_pid.expected_data_count / (temp_pid.expected_data_count * temp_pid.expected_data_count - 1);
    temp_pid.anti_windup = TEMP_PID_WINDUP;
  #endif

  #if RH_CTRL == 2 // PID dell'umidità
    rh_pid.kp = RH_PID_KP;
    rh_pid.ki = RH_PID_KI * (CTRL_PWM_PERIOD / 1000.0f);
    rh_pid.kd = RH_PID_KD / (CTRL_PWM_PERIOD / 1000.0f);
    rh_pid.kw = RH_PID_KW * (CTRL_PWM_PERIOD / 1000.0f);
    rh_pid.proportional = 0.0f;
    rh_pid.integral = 0.0f;
    rh_pid.derivative = 0.0f;
    rh_pid.output = 0.0f;
    rh_pid.sum1 = 0UL;
    rh_pid.sum2 = 0UL;
    rh_pid.sum3 = 0UL;
    rh_pid.data_count = 0;
    rh_pid.expected_data_count = CTRL_PWM_PERIOD / SHT20_READ_PERIOD;
    rh_pid.deriv_c1 = -6.0f / rh_pid.expected_data_count / (rh_pid.expected_data_count + 1);
    rh_pid.deriv_c2 = 12.0f / rh_pid.expected_data_count / (rh_pid.expected_data_count * rh_pid.expected_data_count - 1);
    rh_pid.anti_windup = RH_PID_WINDUP;
  #endif
}

// elabora nuova misurazione di temperatura
void temp_ctrl_new_measure() {
  #if TEMP_CTRL == 1 // HYSTERESIS
    // aggiorna l'output del controllore ad isteresi ON/OFF della temperatura
    if (status.temp_setpoint - status.temp_sht20 > TEMP_HYS_THLD)
      status.temp_pwm_value = CTRL_MAX_OUTPUT;
    else if (status.temp_sht20 - status.temp_setpoint > TEMP_HYS_THLD)
      status.temp_pwm_value = CTRL_MIN_OUTPUT;

    #elif TEMP_CTRL == 2 // PID
      // aggiunge la nuova misurazione agli accumulatori del PID
      pid_add_data(temp_pid, status.temp_setpoint - status.temp_sht20, status.temp_sht20);
  #endif
}

// inizio ciclo PWM della temperatura
void temp_ctrl_new_pwm_cycle() {
  #if TEMP_CTRL == 2 // PID
    // aggiorna l'output del PID della temperatura per il prossimo ciclo del PWM
    status.temp_pwm_value = (pid_update_output(temp_pid) - CTRL_MIN_OUTPUT) * CTRL_PWM_PERIOD / (CTRL_MAX_OUTPUT - CTRL_MIN_OUTPUT);

    // controllo per evitare accensioni o spegnimenti troppo brevi del riscaldatore
    if (status.temp_pwm_value < CTRL_MIN_INTERVAL_ON)
      status.temp_pwm_value = 0;
    else if (status.temp_pwm_value > CTRL_PWM_PERIOD - CTRL_MIN_INTERVAL_OFF)
      status.temp_pwm_value = CTRL_PWM_PERIOD;
  #endif
}


// elabora nuova misurazione di umidità
void rh_ctrl_new_measure() {
  #if RH_CTRL == 1 // HYSTERESIS
    // aggiorna l'output del controllore ad isteresi ON/OFF dell'umidità
    if (status.rh_setpoint - rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) > RH_HYS_THLD)
    status.rh_pwm_value = CTRL_MAX_OUTPUT;
    else if (rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) - status.rh_setpoint > RH_HYS_THLD)
    status.rh_pwm_value = CTRL_MIN_OUTPUT;

  #elif RH_CTRL == 2 // PID
    // aggiunge la nuova misurazione agli accumulatori del PID
    pid_add_data(rh_pid, status.rh_setpoint - rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint), status.rh_sht20);
  #endif
}

// inizio ciclo PWM dell'umidità
void rh_ctrl_new_pwm_cycle() {
  #if RH_CTRL == 2 // PID
    // aggiorna l'output del PID dell'umidità per il prossimo ciclo del PWM
    status.rh_pwm_value = (pid_update_output(rh_pid) - CTRL_MIN_OUTPUT) * CTRL_PWM_PERIOD / (CTRL_MAX_OUTPUT - CTRL_MIN_OUTPUT);

    // controllo per evitare accensioni o spegnimenti troppo brevi dell'umidificatore
    if (status.rh_pwm_value < CTRL_MIN_INTERVAL_ON)
      status.rh_pwm_value = 0;
    else if (status.rh_pwm_value > CTRL_PWM_PERIOD - CTRL_MIN_INTERVAL_OFF)
      status.rh_pwm_value = CTRL_PWM_PERIOD;
  #endif
}


// ----------------------------------------------------------------------------
// ---- utility functions per evitare codice doppio nella gestione dei PID ----

/**
 * @brief Elabora la nuova misurazione.
 *
 * NOTE IMPLEMENTATIVE:
 *
 * Le grandezze richieste per aggiornare l'output sono:
 *  - sum1:
 *    somma degli errori tra setpoint e valore misurato, utilizzato per
 *    la componente proporzionale, integrale e derivativa del PID
 *
 *  - sum2:
 *    somma dei valori misurati, utilizzato per la componente derivativa
 *    del PID
 *
 *  - sum3:
 *    somma dei prodotti tra indice della misurazione e valore misurato,
 *    utilizzato per la componente derivativa del PID
 *
 *  - data_count:
 *    numero di misurazioni ricevute ed elaborate, utilizzato per il calcolo
 *    delle componenti proporzionale, integrale e derivativa del PID
 *
 */
void pid_add_data(struct pid &pid, const int16_t &error, const uint16_t &measure) {
  pid.sum1 += error;
  pid.sum2 += measure;
  pid.sum3 += pid.data_count * measure;
  pid.data_count++;
}

/**
 * @brief Aggiorna l'output del controllore PID.
 *
 * NOTE IMPLEMENTATIVE:
 *
 * L'output "puro" è calcolato dalla successiva formula:
 *
 *   pid.output = pid.proportional + pid.integral + pid.derivative
 *
 * Di seguito si illustrano i procedimenti per il calcolo delle tre componenti
 * del PID sfruttando i dati preparati dalla funzione pid_add_data():
 *
 * proporzionale:
 *  - la componente proporzionale è calcolata come il prodotto tra la costante
 *    proporzionale kp e l'errore medio del ciclo di aggiornamento del PID
 *  - se ci dovessero essere problemi come mancanza di misurazioni, l'errore
 *    medio calcolato con i dati disponibili è comunque una buona stima di
 *    quello reale e il disturbo introdotto scompare totalmente al successivo
 *    aggiornamento dell'output del PID
 *
 *      mean_error = sum_error / data_count
 *      pid.proportional = kp * mean_error
 *
 * integrale:
 *  - la componente integrale è calcolata come il prodotto tra la costante
 *    integrale ki, l'errore accumulato nel ciclo di aggiornamento del PID
 *    e l'intervallo tra due misurazioni consecutive
 *  - se ci dovessero essere problemi come mancanza di misurazioni, i valori
 *    mancanti vengono sostituiti con l'errore medio calcolato per la parte
 *    proporzionale, essendo una buona stima e il disturbo introdotto verrà
 *    corretto grazie al feedback del controllore PID
 *
 *      pid.integral = ki * mean_error * expected_data_count * (SHT20_READ_PERIOD / 1000.0f)
 *
 * derivativo:
 *  - la componente derivativa è calcolata come il prodotto tra la costante
 *    derivativa kd e la derivata dell'errore calcolata tramite la formula
 *    del metodo dei minimi quadrati applicato alla regressione lineare sugli
 *    errori passati,
 *
 *      pid.derivative = kd * ( -deriv_c1 * sum2 + deriv_c2 * sum3);
 *
 *                       6         N-1              12          N-1
 *      dy/dt = - -------------- *  ∑  y_i + ---------------- *  ∑  (i * y_i)
 *                Ts * N (N + 1)   i=0       Ts * N (N^2 - 1)   i=0
 *              \______________/   \_____/   \______________/   \___________/
 *                  deriv_c1         sum2        deriv_c2           sum3
 *    con:
 *     - N = numero di errori passati
 *     - Ts = intervallo tra due misurazioni dell'errore consecutive
 *     - y_i = errore tra setpoint e valore misurato
 *     - i  = indice dell'errore passato (0 = più vecchio, N-1 = più recente)
 *
 *  - siccome i due coefficienti coeff1 e coeff2 sono costanti, vengono
 *    precalcolati al tempo di compilazione e memorizzati in due apposite
 *    costanti deriv_c1 e deriv_c2
 *  - se ci dovessero essere problemi come mancanza di misurazioni, vengono
 *    utilizzati solo i valori disponibili siccome è parecchio complesso
 *    interpolare e integrare i valori mancanti all'interno delle sommatorie,
 *    inoltre il disturbo introdotto scompare totalmente al successivo ciclo
 *    di aggiornamento dell'output ed la regressione rimane sempre una buona
 *    stima della derivata reale; l'unico svantaggio è che bisogna ricalcolare
 *    le costanti deriv_c1 e deriv_c2 in base ai dati disponibili
 *
 *
 * Si aggiungono le seguenti componenti software per la gestione del calcolo
 * dell'output del PID:
 *
 * verifica assenza di dati da elaborare:
 *  - in caso di problemi come mancanza totale di misurazioni, l'output del
 *    PID viene impostato al valore minimo, in modo da spegnere l'attuatore
 *    e non far peggiorare la situazione
 *
 * output constraints:
 *  - l'output del PID viene limitato tra CTRL_MIN_OUTPUT e CTRL_MAX_OUTPUT
 *
 * anti-windup:
 *  - si implementano tre opzioni per la gestione del windup della componente
 *    integrale del PID (no windup, clamping e back calculation) selezionabili
 *    tramite la macro PID_WINDUP:
 *  0. no anti-windup:
 *      la componente integrale viene sempre sommata all'output del PID
 *  1. clamping:
 *      se l'output del PID è superiore al limite massimo e l'errore è
 *      positivo, o viceversa se l'output del PID è inferiore al limite
 *      minimo e l'errore è negativo, il nuovo contributo della componente
 *      integrale viene scartato sia per il calcolo dell'output del PID che
 *      per l'accumulo dell'errore integrale
 *  2. back calculation:
 *      la componente integrale viene compensata da un termine proporzionale
 *      alla differenza tra l'output puro del PID e l'output limitato in modo
 *      da far riportare l'output del PID all'interno dei limiti
 *
 */
float pid_update_output(struct pid &pid) {
  // verifica assenza di dati da elaborare
  if (pid.data_count == 0) {
    pid.sum1 = 0.0f;
    pid.sum2 = 0.0f;
    pid.sum3 = 0.0f;
    return CTRL_MIN_OUTPUT;
  }

  // calcolo della componente proporzionale
  float mean_error = pid.sum1 / pid.data_count;
  pid.proportional = pid.kp * mean_error;

  // calcolo del contributo della componente integrale
  float integral_contrib = pid.ki * mean_error * pid.expected_data_count * (SHT20_READ_PERIOD / 1000.0f);

  // calcolo della componente derivativa
  if (pid.data_count == pid.expected_data_count) {
    pid.derivative = pid.kd * (pid.deriv_c1 * pid.sum2 + pid.deriv_c2 * pid.sum3);
  } else if (pid.data_count == 1) {
    pid.derivative = 0.0f;
  } else {
    float c1 = -6.0f / pid.data_count / (pid.data_count + 1.0f);
    float c2 = 12.0f / pid.data_count / (pid.data_count * pid.data_count - 1.0f);
    pid.derivative = pid.kd * (c1 * pid.sum2 + c2 * pid.sum3);
  }

  // calcolo dell'output temporaneo del PID
  pid.output = pid.proportional + pid.integral + integral_contrib + pid.derivative;

  // anti-windup della componente integrale del PID
  switch (pid.anti_windup) {
    case 0: // no anti-windup
      pid.integral += integral_contrib;
      break;

    case 1: // clamping
      if (!(pid.output > CTRL_MAX_OUTPUT && mean_error > 0.0f) && !(pid.output < CTRL_MIN_OUTPUT && mean_error < 0.0f))
        pid.integral += integral_contrib;
      break;

    case 2: // back calculation
      pid.integral += integral_contrib + pid.kw * (constrain(pid.output, CTRL_MIN_OUTPUT, CTRL_MAX_OUTPUT) - pid.output);
      break;
  }

  // calcolo dell'output finale del PID
  pid.output = pid.proportional + pid.integral + pid.derivative;

  // azzeramento delle variabili per il prossimo ciclo del PID
  pid.sum1 = 0.0f;
  pid.sum2 = 0.0f;
  pid.sum3 = 0.0f;
  pid.data_count = 0;

  // limitazione dell'output del PID tra CTRL_MIN_OUTPUT e CTRL_MAX_OUTPUT
  return constrain(pid.output, CTRL_MIN_OUTPUT, CTRL_MAX_OUTPUT);
}
