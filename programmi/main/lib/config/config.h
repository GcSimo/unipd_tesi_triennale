/**
 * @file config.h
 * @author Giacomo Simonetto
 * @brief File di configurazione per pin e variabili globali di stato
 * dell'incubatrice neonatale.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <SHT2x.h>
#include <LiquidCrystal_I2C.h>

// ----------------------------------------------------------------------------
// ------------------- configurazione pin dell'arduino mega -------------------
// ----------------------------------------------------------------------------

#define ALARM_LED 2  // led rosso di allarme
#define HEAT_LED 3   // led verde di stato riscaldatore
#define RH_LED 4     // led verde di stato umidificatore
#define REFILL_LED 5 // led rosso di refill acqua

#define HEAT_SW 6  // interruttore riscaldatore
#define RH_SW 7    // interruttore umidificatore
#define LIGHT_SW 8 // interruttore illuminazione

#define HEAT_RELAY 9  // relè riscaldatore
#define RH_RELAY 10    // relè umidificatore
#define FAN_RELAY 11   // relè ventola ausiliaria
#define LIGHT_RELAY 12 // relè illuminazione

#define TEMP_POT A0 // potenziometro temperatura
#define RH_POT A1   // potenziometro umidità

// ----------------------------------------------------------------------------
// -------------------- configurazione parametri programma --------------------
// ----------------------------------------------------------------------------

// parametri per potenziometro e setpoint di temperatura
#define TEMP_POT_TIMER 100 // intervallo tra due letture consecutive
#define TEMP_POT_THLD 5    // soglia per soppressione del rumore
#define TEMP_POT_MIN 5     // valore minimo del potenziometro
#define TEMP_POT_MAX 1015  // valore massimo del potenziometro
#define TEMP_MIN 25.0      // valore minimo del setpoint
#define TEMP_MAX 40.0      // valore massimo del setpoint
#define TEMP_STEP 0.1      // passo di variazione del setpoint

// parametri potenziometro e setpoint di umidità
#define RH_POT_TIMER 100 // intervallo tra due letture consecutive
#define RH_POT_THLD 5    // soglia per soppressione del rumore
#define RH_POT_MIN 5     // valore minimo del potenziometro
#define RH_POT_MAX 1015  // valore massimo del potenziometro
#define RH_MIN 20.0      // valore minimo del setpoint
#define RH_MAX 90.0      // valore massimo del setpoint
#define RH_STEP 1.0      // passo di variazione del setpoint


// parametri per il controllo della temperatura
#define TEMP_DEF_SETPOINT 36.0    // setpoint iniziale temperatura
#define TEMP_CONTROLLER 1         // 0 = HYSTERESIS or 1 = PID
#define TEMP_HYSTERESIS_THLD 0.25 // soglia di isteresi per temperatura
#define TEMP_PID_KP 10.0 // guadagno proporzionale del PID per temperatura
#define TEMP_PID_KI 0.0  // guadagno integrale del PID per temperatura
#define TEMP_PID_KD 0.0  // guadagno derivativo del PID per temperatura

// parametri per il controllo dell'umidità
#define RH_DEF_SETPOINT 20.0   // setpoint iniziale umidità
#define RH_CONTROLLER 1        // 0 = HYSTERESIS or 1 = PID
#define RH_HYSTERESIS_THLD 2.0 // soglia di isteresi per umidità
#define RH_PID_KP 10.0 // guadagno proporzionale del PID per umidità
#define RH_PID_KI 0.0  // guadagno integrale del PID per umidità
#define RH_PID_KD 0.0  // guadagno derivativo del PID per umidità

// parametri generali per controllore
#define CTRL_PWM_PERIOD 8000     // periodo del pwm per controllo attuatori (in ms)
#define CTRL_MIN_OUTPUT 0        // valore minimo di uscita del PID
#define CTRL_MAX_OUTPUT CTRL_PWM_PERIOD // valore massimo di uscita del PID
#define PID_UPDATE_INTERVAL 1000 // intervallo di aggiornamento del PID (in ms)
#define PID_PREVIOUS_ERRORS 5    // numero di errori passati da memorizzare

// parametri per la gestione dei timer
#define SHT20_READ_INTERVAL 1000 // intervallo tra due letture dell'SHT20
#define LCD_UPDATE_INTERVAL 1000 // intervallo tra due aggiornamenti dell'lcd
#define ALARM_INTERVAL 1000      // intervallo di accensione dell'allarme
#define REFILL_INTERVAL 3600000  // durata dell'acqua prima del refill, per ora
                                 // è impostata genericamente a 1 ora
                                 // - 0,1017 g/s di acqua nebulizzata -

// stati del sensore SHT20
#define SHT20_IDLE 0               // sensore in pausa
#define SHT20_READY_FOR_TEMP_REQ 1 // pronto per richiedere la temperatura
#define SHT20_WAIT_TEMP 2          // in attesa di ricevere la temperatura
#define SHT20_READY_FOR_RH_REQ 3   // pronto per richiedere l'umidità
#define SHT20_WAIT_RH 4            // in attesa di ricevere l'umidità
#define SHT20_NEW_DATA 5           // nuovi dati disponibili dal sensore


// ----------------------------------------------------------------------------
// ------------------ sensori e periferiche dell'incubatrice ------------------
// ----------------------------------------------------------------------------

// sensore SHT20 per temperatura e umidità
extern SHT2x SHT20;

// display lcd 20x4 con interfaccia I2C
extern LiquidCrystal_I2C lcd;


// ----------------------------------------------------------------------------
// ------------------- variabili di stato dell'incubatrice --------------------
// ----------------------------------------------------------------------------

// variabili globali di stato
struct status {
  // temperatura e umidità
  float temp_sht20; // temperatura rilevata dal sensore SHT20
  float rh_sht20;   // umidità rilevata dal sensore SHT20
  float temp_setpoint;    // setpoint temperatura
  float rh_setpoint;      // setpoint umidità

  // timer di gestione del refill dell'acqua
  unsigned long last_rh_on;     // timestamp ultima accensione umidificatore
  unsigned long refill_counter; // contatore accensione umidificatore

  // valori dei potenziometri
  int temp_pot_value;     // valore del potenziometro temperatura
  int rh_pot_value;       // valore del potenziometro umidità

  // valori PWM per il controllo PID della temperatura e dell'umidità
  unsigned int temp_pwm_value; // valore PWM per il PID della temperatura
  unsigned int rh_pwm_value;   // valore PWM per il PID dell'umidità

  // stato del sensore SHT20
  unsigned char sht20_state;

  // codice di errore
  unsigned char error_code;

  // stato dei relè
  bool heat_relay;  // stato relay e led riscaldatore
  bool rh_relay;    // stato relay e led umidificatore
  bool fan_relay;   // stato relay ventola ausiliaria
  bool light_relay; // stato relay illuminazione

  // stato degli switch
  bool heat_sw;    // stato interruttore riscaldatore
  bool rh_sw;      // stato interruttore umidificatore
  bool light_sw;   // stato interruttore illuminazione

  // stato dei led (esclusi led del riscaldatore e dell'umidificatore)
  bool alarm_led;  // stato led di allarme
  bool refill_led; // stato led di refill acqua

  // variabile per disabilitare il controllo automatico della temperatura
  // e dell'umidità quando si agisce manualmente sugli switch
  bool manual_ctrl;
};

// variabili per gestione dei timer
struct timers {
  unsigned long temp_pot;   // timer per letture potenziometro temperatura
  unsigned long rh_pot;     // timer per letture potenziometro umidità
  unsigned long sht20_read; // timer per richieste di lettura sensore SHT20
  unsigned long lcd_update; // timer aggiornamento display
  unsigned long alarm;      // timer per spegnimento led di allarme
};

// parametri dei controllori pid
struct pid {
  float integral; // termine integrale
  float deriv_s1; // prima sommatoria del termine derivativo
  float deriv_s2; // seconda sommatoria del termine derivativo
  float prev_err[PID_PREVIOUS_ERRORS]; // buffer circolare per errori passati
  //unsigned long last_update;           // timestamp ultimo aggiornamento PID
  int prev_err_index;                  // indice buffer circolare
};

// coefficienti del termine derivativo dei PID (vedi calcolo della derivata)
constexpr float pid_deriv_c1 = 12 / PID_PREVIOUS_ERRORS / (PID_PREVIOUS_ERRORS * PID_PREVIOUS_ERRORS - 1) / PID_UPDATE_INTERVAL * 1000;
constexpr float pid_deriv_c2 = 6 / PID_PREVIOUS_ERRORS / (PID_PREVIOUS_ERRORS + 1) / PID_UPDATE_INTERVAL * 1000;

// struct per memorizzare i messaggi da stampare sul display lcd
struct lcd_message {
  char line1[21]; // prima riga del messaggio
  char line2[21]; // seconda riga del messaggio
  char line3[21]; // terza riga del messaggio
  char line4[21]; // quarta riga del messaggio
};

// ----------------------------------------------------------------------------
// ------------------- variabili di stato dell'incubatrice --------------------
// ----------------------------------------------------------------------------

// variabili globali di stato dell'incubatrice neonatale
extern struct status status; // variabili di stato dell'incubatrice
extern struct timers timers; // variabili timer
extern struct pid temp_pid;  // variabili per il PID della temperatura
extern struct pid rh_pid;    // variabili per il PID dell'umidità

// ----------------------------------------------------------------------------
// ----------------------------- helper functions -----------------------------
// ----------------------------------------------------------------------------

/**
 * @brief Converte un numero decimale in una stringa.
 *
 * Formatta correttamente i valori di temperatura ed umidità
 * per la stampa sul display lcd con due cifre intere e due decimali.
 *
 * @param value il numero decimale da convertire
 * @return puntatore al float_buffer contenente la stringa formattata
 */
char *float_to_string(float value);

// buffer per conversione da float a stringa
extern char float_buffer[6];

/**
 * @brief Calcola l'umidità relativa risultante al raggiungimento del setpoint
 * di temperatura.
 *
 * Il valore di umidità relativa è calcolato in funzione della temperatura,
 * per cui variando la temperatura cambia anche il valore di umidità relativa.
 * Per gestire correttamente l'accensione e lo spegnimento dell'umidificatore
 * è necessario confrontare il setpoint di umidità impostato con l'umidità
 * relativa che si avrebbe al raggiungimento del setpoint di temperatura.
 * Per calcolare tale valore si utilizza la seguente formula:
 *
 *   rh2 = rh1 * exp(4283.78 * (t1 - t2) / (243.12 + t1) / (243.12 + t2))
 *
 *   - t1: temperatura dello stato 1 in °C (attuale - misurata dallo SHT20)
 *   - t2: temperatura dello stato 2 in °C (desiderata - dal setpoint)
 *   - rh1: umidità relativa dello stato 1 (attuale - misurata dallo SHT20)
 *   - rh2: umidità relativa dello stato 2 (desiderata)
 *
 * fonte: https://sensirion.com/media/documents/A419127A/6836C0D2/Sensirion_AppNotes_Humidity_Sensors_at_a_Glance.pdf
 *
 * @param rh1 umidità attuale misurata dal sensore SHT20
 * @param t1 temperatura attuale misurata dal sensore SHT20
 * @param t2 setpoint di temperatura desiderato
 * @return float umidità relativa al raggiungimento del setpoint di temperatura
 */
float rh_at_temp_setpoint(float rh1, float t1, float t2);

/**
 * @brief Funzione map ottimizzata per int a 16bit.
 *
 * L'implementazione standard opera su long a 32bit e richiede più risorse
 * di calcolo per eseguire le operazioni di moltiplicazione e divisione.
 * Questa versione utilizza int a 16bit per ridurne il costo computazionale.
 *
 * @param x valore da mappare
 * @param in_min valore minimo dell'intervallo di input
 * @param in_max valore massimo dell'intervallo di input
 * @param out_min valore minimo dell'intervallo di output
 * @param out_max valore massimo dell'intervallo di output
 * @return int valore mappato nell'intervallo di output
 */
int map_16bit(int x, int in_min, int in_max, int out_min, int out_max);

#endif // CONFIG_H
