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
 * @brief Inizializza i parametri del PID.
 *
 * Salva i parametri del PID all'interno della struct, inizializzando le
 * varie variabili interne del PID.
 *
 * Il parametro anti_windup è opzionale, se non specificato viene impostato
 * a 1 (clamping).
 *
 * @param pid reference alla struct del PID da inizializzare
 * @param kp const reference al guadagno proporzionale
 * @param ki const reference al guadagno integra
 * @param kd const reference al guadagno derivativo
 * @param kw const reference al guadagno della back calculation (anti-windup)
 * @param anti_windup const reference all'opzione di anti-windup da utilizzare
 */
template <uint8_t C>
void pid_init(pid<C> &pid) {
  // inizializzazione delle componenti del PID
  pid.proportional = 0.0f;
  pid.integral = 0.0f;
  pid.derivative = 0.0f;
  pid.output = 0.0f;

  // inizializzazione buffer e indici per errori e misurazioni passate
  memset(pid.errors, 0, sizeof(pid.errors));
  memset(pid.measures, 0, sizeof(pid.measures));
  pid.error_idx = 0;
  pid.measure_idx = 0;
}

/**
 * @brief Salva una nuova misurazione negli accumulatori del PID.
 *
 * Salva l'errore e la misurazione in apposite variabili interne del PID
 * che verranno utilizzate per calcolare l'output del PID alla successiva
 * invocazione della funzione pid_update_output().
 *
 * Si assume che i dati forniti alla funzione vengano acquisiti dai sensori
 * ad intervalli di tempo costanti ogni PID_DATA_PERIOD millisecondi.
 * All'interno della funzione non si effettua nessun controllo temporale per
 * cui il rispetto di tale condizione è responsabilità del chiamante.
 *
 * Non ci sono vincoli sul tempo che intercorre da quando i dati vengono
 * acquisiti dal sensore e quando vengono passati alla funzione.
 *
 * @param pid reference alla struct del PID da aggiornare
 * @param error const reference all'errore tra setpoint e valore misurato
 * @param measure const reference al valore misurato dal sensore
 */
template <uint8_t C> void pid_add_data(pid<C> &pid, const int16_t &error, const int16_t &measure) {
  // inserimento dei nuovi dati
  pid.errors[pid.error_idx] = error;
  pid.measures[pid.measure_idx] = measure;

  // aggiornamento indici
  pid.error_idx = (pid.error_idx + 1) % ((C == TEMP) ? max(TEMP_PID_P_SAMPLES, TEMP_PID_I_SAMPLES) : max(RH_PID_P_SAMPLES, RH_PID_I_SAMPLES));
  pid.measure_idx = (pid.measure_idx + 1) % ((C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES);

  // gli indici puntano all'elemento più vecchio
  // l'elemento più recente è quello all'indice precedente
}

template <uint8_t C> uint8_t error_idx_offset(const pid<C> &pid, int8_t offset) {
  constexpr uint8_t size = (C == TEMP) ? max(TEMP_PID_P_SAMPLES, TEMP_PID_I_SAMPLES) : max(RH_PID_P_SAMPLES, RH_PID_I_SAMPLES);
  return (pid.error_idx + offset + size) % size;
}

template <uint8_t C> uint8_t measure_idx_offset(const pid<C> &pid, int8_t offset) {
  constexpr uint8_t size = (C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES;
  return (pid.measure_idx + offset + size) % size;
}

/**
 * @brief Aggiorna l'output del controllore PID.
 *
 * Calcola il nuovo output del PID in base ai dati salvati negli accumulatori
 * interni dalla funzione pid_add_data() nel tempo intercorso dall'ultimo
 * aggiornamento dell'output del PID.
 *
 * Si prevede che l'output del PID venga aggiornato ad intervalli di tempo
 * costanti ogni PID_UPDATE_PERIOD millisecondi. All'interno della funzione
 * non si effettua nessun controllo temporale per cui il rispetto di tale
 * condizione è responsabilità del chiamante.
 *
 * @param pid reference alla struct del PID da aggiornare
 * @return float output del PID limitato tra PID_MIN_OUTPUT e PID_MAX_OUTPUT
 */
template <uint8_t C> float pid_update_output(pid<C> &pid) {
  constexpr float kp = (C == TEMP) ? TEMP_PID_KP : RH_PID_KP;
  constexpr float ki = (C == TEMP) ? TEMP_PID_KI : RH_PID_KI;
  constexpr float kd = (C == TEMP) ? TEMP_PID_KD : RH_PID_KD;
  constexpr float kw = (C == TEMP) ? TEMP_PID_KW : RH_PID_KW;
  constexpr uint8_t p_samples = (C == TEMP) ? TEMP_PID_P_SAMPLES : RH_PID_P_SAMPLES;
  constexpr uint8_t i_samples = (C == TEMP) ? TEMP_PID_I_SAMPLES : RH_PID_I_SAMPLES;
  constexpr uint8_t d_samples = (C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES;
  constexpr float deriv_c1 = -6.0f / d_samples / (d_samples + 1.0f) / (PID_DATA_PERIOD / 1000.0f);
  constexpr float deriv_c2 = 12.0f / d_samples / (d_samples * d_samples - 1.0f) / (PID_DATA_PERIOD / 1000.0f);
  constexpr uint8_t windup = (C == TEMP) ? TEMP_PID_WINDUP : RH_PID_WINDUP;

  // calcolo della componente proporzionale
  float mean_error = 0.0f;
  for (uint8_t i = 0; i < p_samples; i++) {
    mean_error += pid.errors[error_idx_offset(pid, -(i+1))];
  }

  pid.proportional = kp * mean_error / p_samples;

  // calcolo del contributo della componente integrale
  float sum_integral = 0.0f;
  for (uint8_t i = 0; i < i_samples; i++) {
    sum_integral += pid.errors[error_idx_offset(pid, -(i+1))];
  }
  float integral_contrib = ki * sum_integral * (PID_DATA_PERIOD / 1000.0f);

  // calcolo della componente derivativa
  float sum1 = 0.0f;
  float sum2 = 0.0f;
  for (uint8_t i = 0; i < d_samples; i++) {
    sum1 += pid.measures[measure_idx_offset(pid, i)];
    sum2 += i * pid.measures[measure_idx_offset(pid, i)];
  }

  pid.derivative = kd * (deriv_c1 * sum1 + deriv_c2 * sum2);

  // calcolo dell'output temporaneo del PID
  pid.output = pid.proportional + pid.integral + integral_contrib - pid.derivative;

  // 5. anti-windup della componente integrale del PID
  if (windup == 0) { // no anti-windup
    pid.integral += integral_contrib;
  } else if (windup == 1) { // clamping
    if (!(pid.output > PID_MAX_OUTPUT && mean_error > 0.0f) && !(pid.output < PID_MIN_OUTPUT && mean_error < 0.0f))
      pid.integral += integral_contrib;
  } else if (windup == 2) { // back calculation
    pid.integral += integral_contrib + kw * (constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT) - pid.output) * (PID_UPDATE_PERIOD / 1000.0f);
  }

  // ricalcolo dell'output finale del PID
  pid.output = pid.proportional + pid.integral - pid.derivative;

  // limitazione dell'output del PID tra PID_MIN_OUTPUT e PID_MAX_OUTPUT
  return constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT);
}

/**
 * @brief Resetta gli accumulatori del PID.
 *
 * Resetta gli accumulatori interni del pid che contengono i dati delle
 * misurazioni ricevute dall'ultimo aggiornamento dell'output del PID.
 *
 * Deve essere invocata ogni volta che si riattiva il controllo automatico
 * del PID dopo un periodo di controllo manuale, per evitare che i vecchi
 * dati memorizzati influenzino il successivo calcolo dell'output del PID.
 *
 * @param pid reference alla struct del PID da resettare
 * @param error valore iniziale per i buffer degli errori
 * @param measure valore iniziale per i buffer delle misure
 */
template <uint8_t C> void pid_reset_buffers(pid<C> &pid, const int16_t &error, const int16_t &measure) {
  memset(pid.errors, error, sizeof(pid.errors));
  memset(pid.measures, measure, sizeof(pid.measures));
  pid.error_idx = 0;
  pid.measure_idx = 0;
}

#endif // PID_H
