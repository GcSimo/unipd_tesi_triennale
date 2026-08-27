/**
 * @file controller.cpp
 * @author Giacomo Simonetto
 * @brief Controllo degli attuatori dell'incubatrice neonatale
 *
 * Implementazione delle funzioni dei controllori ad isteresi ON/OFF e PID
 * per la gestione degli attuatori dell'incubatrice neonatale.
 */

#include "controller.h"

// coefficienti del termine derivativo dei PID (vedi calcolo della derivata)
constexpr float pid_deriv_c1 = 12.0f / PID_PREVIOUS_ERRORS / (PID_PREVIOUS_ERRORS * PID_PREVIOUS_ERRORS - 1.0f);
constexpr float pid_deriv_c2 = 6.0f / PID_PREVIOUS_ERRORS / (PID_PREVIOUS_ERRORS + 1.0f);

// inizializza i parametri per i controllori PID
void ctrl_begin() {
  // inizializzazione dei controllori PID
  #if TEMP_CONTROLLER == 1 // PID
    temp_pid.kp = TEMP_PID_KP;
    temp_pid.ki = TEMP_PID_KI * (PID_UPDATE_INTERVAL / 1000.0f);
    temp_pid.kd = TEMP_PID_KD / (PID_UPDATE_INTERVAL / 1000.0f);
    temp_pid.kw = TEMP_PID_KW * (PID_UPDATE_INTERVAL / 1000.0f);
    temp_pid.proportional = 0.0;
    temp_pid.integral = 0.0;
    temp_pid.derivative = 0.0;
    temp_pid.output = 0.0;
    //temp_pid.last_update = 0;
    for (int i = 0; i < PID_PREVIOUS_ERRORS; i++) {
      temp_pid.prev_err[i] = 0.0;
    }
    temp_pid.prev_err_idx = 0;
  #endif

  #if RH_CONTROLLER == 1 // PID
    rh_pid.kp = RH_PID_KP;
    rh_pid.ki = RH_PID_KI * (PID_UPDATE_INTERVAL / 1000.0f);
    rh_pid.kd = RH_PID_KD / (PID_UPDATE_INTERVAL / 1000.0f);
    rh_pid.kw = RH_PID_KW * (PID_UPDATE_INTERVAL / 1000.0f);
    rh_pid.proportional = 0.0;
    rh_pid.integral = 0.0;
    rh_pid.derivative = 0.0;
    rh_pid.output = 0.0;
    //rh_pid.last_update = 0;
    for (int i = 0; i < PID_PREVIOUS_ERRORS; i++) {
      rh_pid.prev_err[i] = 0.0;
    }
    rh_pid.prev_err_idx = 0;
  #endif
}

// calcola l'output del controllore PIDs
float pid_compute(struct pid &pid, float error) {
  /**
   * @brief Nota sul calcolo della componente derivativa del PID
   *
   * Per ridurre l'impatto del rumore sul calcolo della derivata ed avere una
   * stima più stabile, si effettua una regressione lineare sui valori degli
   * errori passati utilizzando il metodo dei minimi quadrati.
   *
   * La formula generale è la seguente:
   *
   *                 12          N-1                    6         N-1
   *  dy/dt = ---------------- *  ∑  (i * y_i) - -------------- *  ∑  y_i
   *          Ts * N (N^2 - 1)   i=0             Ts * N (N + 1)   i=0
   *          \______________/   \___________/   \____________/   \_____/
   *              coeff. 1           sum. 1         coeff. 2       sum. 2
   *  con:
   *   - N = PID_PREVIOUS_ERRORS, numero di errori passati da considerare
   *   - Ts = PID_UPDATE_INTERVAL, intervallo di aggiornamento del PID
   *   - y_i = error, errore tra setpoint e valore misurato
   *   - i = indice dell'errore passato (0 = più vecchio, N-1 = più recente)
   *
   * Si osserva che:
   *  - i due coefficienti sono costanti, vengono calcolati al tempo di
   *    compilazione e memorizzati in pid_deriv_c1 e pid_deriv_c2
   *  - le due sommatorie vengono progressivamente aggiornate ad ogni ciclo
   *    in maniera efficiente come illustrato di seguito
   *
   * Aggiornamento delle due sommatorie ad ogni valutazione del PID:
   *  1. alla seconda sommatoria (sum. 2) si somma l'errore più recente
   *     (i = N-1) e si sottrae l'errore più vecchio (i = -1) che è uscito
   *     dalla finestra degli N errori passati puù recenti
   *  2. alla prima sommatoria (sum. 1) si somma l'errore più recente
   *     (i = N-1) pesato per N = N-1+1 e si sottrae il valore della
   *     prima sommatoria (sum. 1) per ridurre progressivamente il peso
   *     di tutti gli errori tra cui il più recente che avrà peso N-1 e
   *     il più vecchio che avrà peso 0
   *  3. si sostituisce l'errore più vecchio con quello nuovo nell'array
   *     circolare degli errori passati e si aggiorna l'indice affinché
   *     punti all'errore più vecchio del ciclo successivo
   *
   * NOTA: siccome le rappresentazioni float possono avere errori di
   * arrotondamento, è possibile che nella riduzione dei pesi della prima
   * sommatoria si verifichi un accumulo di errori che porti a valori
   * errati della derivata. Per questo motivo le sommatorie vengono
   * ricalcolate ad ogni ciclo del PID, siccome il numero di errori passati
   * è generalmente piccolo e il costo computazionale è trascurabile.
   */

  float output = 0.0;
  float sum1 = 0.0;
  float sum2 = 0.0;

  // aggiornamento buffer degli errori passati
  pid.prev_err[pid.prev_err_idx] = error; // buffer circolare degli errori passati
  pid.prev_err_idx = (pid.prev_err_idx + 1) % PID_PREVIOUS_ERRORS; // indice del buffer

  // calcolo delle sommatorie per il calcolo della derivata
  for (int i = 0; i < PID_PREVIOUS_ERRORS; i++) {
    int chron_idx = (pid.prev_err_idx + i) % PID_PREVIOUS_ERRORS;
    sum1 += i * pid.prev_err[chron_idx]; // prima sommatoria
    sum2 += pid.prev_err[chron_idx];     // seconda sommatoria
  }

  // calcolo delle componenti del PID
  pid.proportional = pid.kp * error;
  pid.integral += pid.ki * error;
  pid.derivative = pid.kd * (pid_deriv_c1 * sum1 - pid_deriv_c2 * sum2);

  // calcolo dell'output del PID
  pid.output = pid.proportional + pid.integral + pid.derivative;

  // verifica dei limiti dell'output
  if (pid.output > CTRL_MAX_OUTPUT)
    output = CTRL_MAX_OUTPUT;
  else if (pid.output < CTRL_MIN_OUTPUT)
    output = CTRL_MIN_OUTPUT;
  else
    output = pid.output;

  #if PID_ANTI_WINDUP == 0
    // anti-windup dell'integrale tramite clamping
    if (pid.output > output && error > 0.0f)
      pid.integral -= pid.ki * error; // riduzione dell'integrale
    else if (pid.output < output && error < 0.0f)
      pid.integral -= pid.ki * error; // riduzione dell'integrale
  #elif PID_ANTI_WINDUP == 1
    // anti-windup dell'integrale tramite back-calculation
    pid.integral += pid.kw * (output - pid.output);
  #endif

  // ritorno dell'output limitato del PID
  return map(output, CTRL_MIN_OUTPUT, CTRL_MAX_OUTPUT, 0, CTRL_PWM_PERIOD);
}

// aggiorna l'output del controllore di riscaldamento
void temp_ctrl_update() {
  #if TEMP_CONTROLLER == 0 // HYSTERESIS
    if (status.temp_setpoint - status.temp_sht20 > TEMP_HYSTERESIS_THLD)
    status.temp_pwm_value = CTRL_MAX_OUTPUT;
    else if (status.temp_sht20 - status.temp_setpoint > TEMP_HYSTERESIS_THLD)
    status.temp_pwm_value = CTRL_MIN_OUTPUT;

    #elif TEMP_CONTROLLER == 1 // PID
    // verifica intervallo di aggiornamento del PID
    //if (millis() - temp_pid.last_update < PID_UPDATE_INTERVAL)
    //  return;

    // calcolo dell'output del PID per la temperatura
    status.temp_pwm_value = pid_compute(temp_pid, status.temp_setpoint - status.temp_sht20);

    // aggiornamento timestamp dell'ultimo aggiornamento del PID
    //temp_pid.last_update = millis();
  #endif
}

// aggiorna l'output del controllore di umidità
void rh_ctrl_update() {
  #if RH_CONTROLLER == 0 // HYSTERESIS
    if (status.rh_setpoint - rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) > RH_HYSTERESIS_THLD)
    status.rh_pwm_value = CTRL_MAX_OUTPUT;
    else if (rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) - status.rh_setpoint > RH_HYSTERESIS_THLD)
    status.rh_pwm_value = CTRL_MIN_OUTPUT;

    #elif RH_CONTROLLER == 1 // PID
    // verifica intervallo di aggiornamento
    //if (millis() - rh_pid.last_update < PID_UPDATE_INTERVAL)
    //  return;

    // calcolo dell'output del PID per l'umidità relativa
    status.rh_pwm_value = pid_compute(rh_pid, status.rh_setpoint - rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint));

    // aggiornamento timestamp dell'ultimo aggiornamento del PID
    //rh_pid.last_update = millis();
  #endif
}
