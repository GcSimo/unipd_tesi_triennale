#include <iostream>
using namespace std;

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))


// ----------------------------------------------------------------------------

// istanze dei controllori PID (template)
#define TEMP 0 // controllore PID della temperatura
#define RH 1   // controllore PID dell'umidità

// tipi di controllori
#define NONE 0 // nessun controllo
#define HYST 1 // controllore ad isteresi ON/OFF
#define PID 2  // controllore PID

// tipi di windup per i controllori PID
#define NO_WINDUP 0 // nessun anti-windup
#define CLAMPING 1  // anti-windup con clamping
#define BACK_CALC 2 // anti-windup con back calculation

#define SHT20_READ_PERIOD 1000    // tempo di campionamento dell'SHT20

// parametri per la gestione del segnale pwm
#define PWM_PERIOD 15000 // periodo del pwm per controllo attuatori (in ms) (max 30 sec)
#define PWM_MIN_TIME_ON 500  // intervallo minimo di accensione attuatori (in ms)
#define PWM_MIN_TIME_OFF 500 // intervallo minimo di spegnimento attuatori (in ms)

// parametri generali per i controllori pid
#define PID_MIN_OUTPUT 0   // limite minimo dell'output del PID
#define PID_MAX_OUTPUT 100 // limite massimo dell'output del PID
#define PID_DATA_PERIOD SHT20_READ_PERIOD // periodo di acquisizione dei dati
#define PID_UPDATE_PERIOD PWM_PERIOD      // periodo di aggiornamento dell'output

#define TEMP_PID_KP 50.0f    // guadagno proporzionale del PID per temperatura
#define TEMP_PID_KI 0.6f // guadagno integrale del PID per temperatura
#define TEMP_PID_KD 500.0f   // guadagno derivativo del PID per temperatura
#define TEMP_PID_KW 0.0f    // guadagno anti-windup del PID per temperatura
#define TEMP_PID_WINDUP CLAMPING // tipo di anti-windup del PID per temperatura
#define TEMP_PID_P_SAMPLES 2 // campioni per calcolo della componente proporzionale
#define TEMP_PID_D_SAMPLES 20 // campioni per calcolo della componente derivativa
#define TEMP_PID_I_SAMPLES (PID_UPDATE_PERIOD / PID_DATA_PERIOD) // campioni per calcolo della componente integrale

// parametri per il controllo dell'umidità
#define RH_CTRL NONE     // tipo di controllore per l'umidità
#define RH_HYS_THLD 200  // soglia di isteresi per umidità (in cent. di %)
#define RH_PID_BIAS 0.0f   // bias del PID per umidità
#define RH_PID_KP 0.0f   // guadagno proporzionale del PID per umidità
#define RH_PID_KI 0.0f   // guadagno integrale del PID per umidità
#define RH_PID_KD 0.0f   // guadagno derivativo del PID per umidità
#define RH_PID_KW 0.0f   // guadagno anti-windup del PID per umidità
#define RH_MIN_INTEGRAL 0.0f // limite minimo della componente integrale
#define RH_MAX_INTEGRAL 100.0f  // limite massimo della componente integrale
#define RH_PID_WINDUP CLAMPING // tipo di anti-windup del PID per umidità
#define RH_PID_P_SAMPLES 2 // campioni per calcolo della componente proporzionale
#define RH_PID_D_SAMPLES 10 // campioni per calcolo della componente derivativa
#define RH_PID_I_SAMPLES (PID_UPDATE_PERIOD / PID_DATA_PERIOD) // campioni per calcolo della componente integrale


// parametri del controllore PID
template <uint8_t C> struct pid {
  // componenti del controllore PID
  float proportional; // componente proporzionale
  float integral;     // componente integrale
  float derivative;   // componente derivativa
  float output;       // output puro del PID non limitato

  // errori e misurazioni passate per calcolo PID
  int16_t errors[(C == TEMP) ? (max(TEMP_PID_P_SAMPLES, TEMP_PID_I_SAMPLES)) : (max(RH_PID_P_SAMPLES, RH_PID_I_SAMPLES))];
  int16_t measures[(C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES];
  uint8_t error_idx;
  uint8_t measure_idx;

  // flag per primi dati forniti (per inizializzare i valori nei buffer)
  bool initialized;
};

struct pid<TEMP> temp_pid;  // variabili per il PID della temperatura

// ----------------------------------------------------------------------------

template <uint8_t C> uint8_t error_idx_offset(const pid<C> &pid, int8_t offset) {
  constexpr uint8_t size = (C == TEMP) ? max(TEMP_PID_P_SAMPLES, TEMP_PID_I_SAMPLES) : max(RH_PID_P_SAMPLES, RH_PID_I_SAMPLES);
  return (pid.error_idx + offset % size + size) % size;
}
template <uint8_t C> uint8_t measure_idx_offset(const pid<C> &pid, int8_t offset) {
  constexpr uint8_t size = (C == TEMP) ? TEMP_PID_D_SAMPLES : RH_PID_D_SAMPLES;
  return (pid.measure_idx + offset % size + size) % size;
}
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
template <uint8_t C> float pid_update_output(pid<C> &pid) {
  // verifica che il PID sia stato inizializzato, altrimenti restituisce 0.0f
  if (!pid.initialized)
    return 0.0f;

  // recupero dei parametri del PID
  // NOTA: le costanti sono divise per 100, siccome le misure del sensore sono
  // state moltiplicate per 100 per memorizzarle in interi con 2 cifre decimali
  constexpr float kp = ((C == TEMP) ? TEMP_PID_KP : RH_PID_KP) / 100.0f;
  constexpr float ki = ((C == TEMP) ? TEMP_PID_KI : RH_PID_KI) / 100.0f;
  constexpr float kd = ((C == TEMP) ? TEMP_PID_KD : RH_PID_KD) / 100.0f;
  constexpr float kw = ((C == TEMP) ? TEMP_PID_KW : RH_PID_KW) / 100.0f;
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
  pid.output = pid.proportional + pid.integral + integral_contrib - pid.derivative;

  // gestione del windup della componente integrale
  if (windup == 0) { // no anti-windup
    pid.integral += integral_contrib;
  } else if (windup == 1) { // clamping
    if (!(pid.output > PID_MAX_OUTPUT && mean_error > 0.0f) && !(pid.output < PID_MIN_OUTPUT && mean_error < 0.0f))
      pid.integral += integral_contrib;
  } else if (windup == 2) { // back calculation
    pid.integral += integral_contrib + kw * (constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT) - pid.output) * (PID_UPDATE_PERIOD / 1000.0f);
  }

  // -------------- 5. calco dell'output e opportune limitazioni --------------
  // ricalcolo dell'output finale del PID
  pid.output = pid.proportional + pid.integral - pid.derivative;

  // limitazione dell'output del PID tra PID_MIN_OUTPUT e PID_MAX_OUTPUT
  return constrain(pid.output, PID_MIN_OUTPUT, PID_MAX_OUTPUT);
}

// ----------------------------------------------------------------------------
bool is_close(float a, float b, float tol = 1e-3) {
    return std::abs(a - b) < tol;
}

int main() {
    struct pid<TEMP> my_pid;
    cout << "==================================================" << endl;
    cout << "       TEST UNITARI PID INCUBATRICE (C++)         " << endl;
    cout << "==================================================" << endl;

    // ---------------------------------------------------------
    // TEST 1: Inizializzazione e Reset (Bumpless Transfer)
    // ---------------------------------------------------------
    cout << "\n[TEST 1] Inizializzazione e Reset Buffer..." << endl;
    pid_init<TEMP>(my_pid);

    // Inserisco il primo dato (Errore 500, Misura 2500)
    pid_add_data<TEMP>(my_pid, 500, 2500);
    pid_update_output<TEMP>(my_pid);

    // Calcoli attesi:
    // P = Kp (1.5) * errore medio (500) = 750
    // D = Derivata di un array tutto uguale a 2500 deve essere 0.0
    float expected_p = TEMP_PID_KP / 100.0f * 500.0f;
    float expected_d = 0.0f;

    if (is_close(my_pid.proportional, expected_p) && is_close(my_pid.derivative, expected_d)) {
        cout << " -> [PASS] Buffer inizializzati retroattivamente. No Derivative Kick!" << endl;
		cout << " -> P=" << my_pid.proportional << " (Atteso:" << expected_p << "), D=" << my_pid.derivative << " (Atteso:" << expected_d << ")" << endl;
    } else {
        cout << " -> [FAIL] Valori: P=" << my_pid.proportional << " (Atteso:" << expected_p << "), D=" << my_pid.derivative << " (Atteso:" << expected_d << ")" << endl;
    }


    // ---------------------------------------------------------
    // TEST 2: Verifica Componente Derivativa (Rampa Termica)
    // ---------------------------------------------------------
    cout << "\n[TEST 2] Pendenza Derivata con Rampa Lineare..." << endl;
    pid_init<TEMP>(my_pid); // reset totale

    // Creiamo una rampa di misura: cresce di +150 (es. 1.5°C) ad ogni campionamento (1 secondo)
    // Manteniamo l'errore a 0 per non sporcare i calcoli visivi
    float slope_per_sec = 150.0f;

    for (int i = 0; i < TEMP_PID_D_SAMPLES; i++) {
        // Al primo giro (i=0) la flag era false, quindi inizializza tutto a 1000.
        // Poi sostituisce i valori creando la rampa.
        pid_add_data<TEMP>(my_pid, 0, 1000 + i * slope_per_sec);
    }
    pid_update_output<TEMP>(my_pid);

    // Atteso: Kd (3.0) * pendenza misurata su dati filtrati.
    // Siccome è una rampa perfetta di +150/s calcolata ai minimi quadrati, il filtro
    // estrarrà esattamente 150.0. D = 3.0 * 150.0 = 450.0.
    expected_d = TEMP_PID_KD / 100.0f * slope_per_sec;

    cout << " -> Derivata calcolata = " << my_pid.derivative << ", Attesa = " << expected_d << endl;
    if (is_close(my_pid.derivative, expected_d)) {
        cout << " -> [PASS] Algoritmo ai minimi quadrati perfetto!" << endl;
    } else {
        cout << " -> [FAIL] Errore nel calcolo del filtro sulla pendenza." << endl;
    }


    // ---------------------------------------------------------
    // TEST 3: Verifica Accumulo Integrale (Valori Casuali)
    // ---------------------------------------------------------
    cout << "\n[TEST 3] Somma Componente Integrale..." << endl;
    pid_init<TEMP>(my_pid);

    // Inseriamo I_SAMPLES (che è 8) dati con errori casuali
    int16_t random_errors[] = {10, -5, 20, 0, 15, -10, 30, 20, -15, 5, -25, 10, 5, 0, 0};
    float manual_sum = 0;

    for (int i = 0; i < 15; i++) {
        pid_add_data<TEMP>(my_pid, random_errors[i], 2000);
        manual_sum += random_errors[i];
    }
    pid_update_output<TEMP>(my_pid);

    // Atteso: Ki (0.25) * Somma (80) * dt_secondi (1.0)
    float expected_i = TEMP_PID_KI / 100.0f * manual_sum * (PID_DATA_PERIOD / 1000.0f);

    cout << " -> Somma errori manuale = " << manual_sum << " - samples: " << TEMP_PID_I_SAMPLES << endl;
    cout << " -> Integrale calcolato  = " << my_pid.integral << ", Atteso = " << expected_i << endl;
    if (is_close(my_pid.integral, expected_i)) {
        cout << " -> [PASS] Il campionamento I_SAMPLES e l'accumulo coincidono!" << endl;
    } else {
        cout << " -> [FAIL] Discrepanza sull'accumulatore integrale." << endl;
    }

    // ---------------------------------------------------------
    // TEST 4: Verifica Reset Accumulo
    // ---------------------------------------------------------
    cout << "\n[TEST 4] Simulazione disattivazione e riattivazione PID..." << endl;

    // Al momento l'integrale vale "expected_i" (20.0). Spegniamo il PID.
    my_pid.initialized = false;

    // Riaccendiamo il PID con nuovi valori
    pid_add_data<TEMP>(my_pid, 100, 2500);
    pid_update_output<TEMP>(my_pid);

    // L'integrale calcolato dovrebbe essersi azzerato e contenere SOLO il nuovo step
    // Nuovo step = Ki (0.25) * (100 errore * 8 campioni di buffer pieno) * 1.0s = 200.0
    expected_i = TEMP_PID_KI / 100.0f * (100.0f * TEMP_PID_I_SAMPLES) * 1.0f;

    if (is_close(my_pid.integral, expected_i)) {
        cout << " -> [PASS] La memoria passata e' stata cancellata correttamente (Bumpless reset)!" << endl;
    } else {
        cout << " -> [FAIL] Il PID ricorda ancora i dati vecchi! (Integrale attuale: " << my_pid.integral << ")" << endl;
    }

    cout << "\n==================================================" << endl;
    cout << "              TUTTI I TEST COMPLETATI             " << endl;
    cout << "==================================================" << endl;

    return 0;
}
