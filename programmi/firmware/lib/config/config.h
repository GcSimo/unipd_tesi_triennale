/**
 * @file config.h
 * @author Giacomo Simonetto
 * @brief Configurazione di parametri e variabili globali di stato.
 *
 * Dichiarazione dei parametri di configurazione e delle variabili globali
 * di stato dell'incubatrice neonatale.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "sht20.h"

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
#define FAN_RELAY 11   // relè ventola di omogeneizzazione
#define LIGHT_RELAY 12 // relè illuminazione

#define TEMP_POT A0 // potenziometro temperatura
#define RH_POT A1   // potenziometro umidità


// ----------------------------------------------------------------------------
// -------------------- configurazione flag del programma ---------------------
// ----------------------------------------------------------------------------

// stati relay, switch e led
#define RELAY_OFF HIGH // relè spento (logica invertita)
#define RELAY_ON LOW   // relè acceso (logica invertita)
#define LED_OFF LOW    // led spento
#define LED_ON HIGH    // led acceso

// modalità di controllo della temperatura e dell'umidità
#define AUTO_CTRL 0   // controllo automatico tramite PID o isteresi
#define MANUAL_CTRL 1 // controllo manuale tramite switch

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
#define LIMITING 2  // anti-windup con limiting
#define BACK_CALC 3 // anti-windup con back calculation

// ----------------------------------------------------------------------------
// ------------ configurazione parametri per gestione dei setpoint ------------
// ----------------------------------------------------------------------------

// parametri per lettura potenziometro e scelta del setpoint di temperatura
#define TEMP_POT_THLD 5   // soglia per soppressione del rumore
#define TEMP_POT_MIN 10   // valore minimo del potenziometro (misurato 3-4)
#define TEMP_POT_MAX 1015 // valore massimo del potenziometro (misurato 1019-1020)
#define TEMP_DEF_SP 3600  // setpoint iniziale temperatura (in cent. di °C)
#define TEMP_MIN_SP 2500  // valore minimo del setpoint (in cent. di °C)
#define TEMP_MAX_SP 4000  // valore massimo del setpoint (in cent. di °C)
#define TEMP_STEP_SP 10   // passo di variazione del setpoint(in cent. di °C)

// parametri per lettura del potenziometro e scelta del setpoint di umidità
#define RH_POT_THLD 5     // soglia per soppressione del rumore
#define RH_POT_MIN 10     // valore minimo del potenziometro (misurato 3-4)
#define RH_POT_MAX 1015   // valore massimo del potenziometro (misurato 1019-1020)
#define RH_DEF_SP 2000    // setpoint iniziale umidità (in cent. di %)
#define RH_MIN_SP 2000    // valore minimo del setpoint (in cent. di %)
#define RH_MAX_SP 9000    // valore massimo del setpoint (in cent. di %)
#define RH_STEP_SP 100    // passo di variazione del setpoint (in cent. di %)


// ----------------------------------------------------------------------------
// ------------ configurazione parametri per gestione degli errori ------------
// ----------------------------------------------------------------------------

// parametri per gestione errori sul range di temperatura valido
#define TEMP_ERR_THLD 0  // soglia di errore per temperatura (in cent. di °C)
#define TEMP_ERR_HYST 20 // isteresi per errore di temperatura (in cent. di °C)

// parametri per gestione errori sul range di umidità valido
#define RH_ERR_THLD 0   // soglia di errore per umidità (in cent. di %)
#define RH_ERR_HYST 100 // isteresi per errore di umidità (in cent. di %)


// ----------------------------------------------------------------------------
// ----------- configurazione parametri per la gestione dei timers ------------
// ----------------------------------------------------------------------------

// parametri per la gestione dei timer
#define SW_READ_PERIOD 50         // tempo di campionamento degli switch
#define TEMP_POT_READ_PERIOD 100  // tempo di campionamento del temp_pot
#define RH_POT_READ_PERIOD 100    // tempo di campionamento dell'rh_pot
#define SP_UPDATE_DELAY 1300      // ritardo di aggiornamento del setpoint
#define SHT20_READ_PERIOD 1000    // tempo di campionamento dell'SHT20
#define LCD_UPDATE_PERIOD 1000    // tempo di aggiornamento dell'lcd
#define ERR_TIMER 1100            // tempo di durata degli errori temporizzati
#define REFILL_INTERVAL 2500000UL // durata dell'acqua prima del refill
// 250ml capacità (da 750ml a 500ml) -> 250g di acqua
// 0.1g/sec dalla tesi magistrale di Vanni
// 250g / 0.1g/sec = 2500 sec = 41.6 min = 2500000 ms


// ----------------------------------------------------------------------------
// -------- configurazione parametri per la gestione del pwm e dei pid --------
// ----------------------------------------------------------------------------

// parametri per la gestione del segnale pwm
#define PWM_PERIOD 15000     // periodo del pwm per controllo attuatori (in ms) (max 30 sec)
#define PWM_MIN_TIME_ON 500  // intervallo minimo di accensione attuatori (in ms)
#define PWM_MIN_TIME_OFF 500 // intervallo minimo di spegnimento attuatori (in ms)

// parametri generali per i controllori pid
#define PID_MIN_OUTPUT 0   // limite minimo dell'output del PID
#define PID_MAX_OUTPUT 100 // limite massimo dell'output del PID
#define PID_DATA_PERIOD SHT20_READ_PERIOD // periodo di acquisizione dei dati
#define PID_UPDATE_PERIOD PWM_PERIOD      // periodo di aggiornamento dell'output


// ----------------------------------------------------------------------------
// ------- configurazione parametri per i controllori ad isteresi e pid -------
// ----------------------------------------------------------------------------

// parametri per il controllo della temperatura
#define TEMP_CTRL PID      // tipo di controllore per la temperatura
#define TEMP_HYS_THLD 10   // soglia di isteresi per temperatura (in cent. di °C)
#define TEMP_PID_BIAS 0.0f // bias di feedforward
#define TEMP_PID_KP 70.0f  // guadagno proporzionale
#define TEMP_PID_KI 0.8f   // guadagno integrale
#define TEMP_PID_KD 0.0f   // guadagno derivativo
#define TEMP_PID_KW 0.0f   // guadagno anti-windup
#define TEMP_MIN_INTEGRAL -30.0f // limite minimo della componente integrale
#define TEMP_MAX_INTEGRAL 30.0f  // limite massimo della componente integrale
#define TEMP_PID_WINDUP CLAMPING // tipo di anti-windup
#define TEMP_PID_RESET 0      // reset delle componenti del PID ad ogni riavvio del PID
#define TEMP_PID_P_SAMPLES 2  // campioni per calcolo della componente proporzionale
#define TEMP_PID_D_SAMPLES 20 // campioni per calcolo della componente derivativa
#define TEMP_PID_I_SAMPLES (PID_UPDATE_PERIOD / PID_DATA_PERIOD) // campioni per calcolo della componente integrale

// parametri per il controllo dell'umidità
#define RH_CTRL NONE     // tipo di controllore per l'umidità
#define RH_HYS_THLD 200  // soglia di isteresi per umidità (in cent. di %)
#define RH_PID_BIAS 0.0f // bias del PID per umidità
#define RH_PID_KP 0.0f   // guadagno proporzionale del PID per umidità
#define RH_PID_KI 0.0f   // guadagno integrale del PID per umidità
#define RH_PID_KD 0.0f   // guadagno derivativo del PID per umidità
#define RH_PID_KW 0.0f   // guadagno anti-windup del PID per umidità
#define RH_MIN_INTEGRAL 0.0f   // limite minimo della componente integrale
#define RH_MAX_INTEGRAL 100.0f // limite massimo della componente integrale
#define RH_PID_WINDUP CLAMPING // tipo di anti-windup del PID per umidità
#define RH_PID_RESET 0      // reset delle componenti del PID ad ogni riavvio del PID
#define RH_PID_P_SAMPLES 2  // campioni per calcolo della componente proporzionale
#define RH_PID_D_SAMPLES 20 // campioni per calcolo della componente derivativa
#define RH_PID_I_SAMPLES (PID_UPDATE_PERIOD / PID_DATA_PERIOD) // campioni per calcolo della componente integrale


// ----------------------------------------------------------------------------
// ------------------ sensori e periferiche dell'incubatrice ------------------
// ----------------------------------------------------------------------------

// sensore SHT20 per temperatura e umidità
extern sht20 sensor;

// display lcd 20x4 con interfaccia I2C
extern LiquidCrystal_I2C lcd;


// ----------------------------------------------------------------------------
// -------- definizione struct per variabili di stato dell'incubatrice --------
// ----------------------------------------------------------------------------

// variabili globali di stato
struct status {
  // misurazioni e setpoint di temperatura e umidità
  // memorizzati come interi a 4 cifre (es. 36.25°C = 3625, 20.00% = 2000)
  int16_t temp_sht20;      // temperatura rilevata dal sensore SHT20
  int16_t rh_sht20;        // umidità rilevata dal sensore SHT20
  int16_t temp_setpoint;  // setpoint temperatura
  int16_t rh_setpoint;    // setpoint umidità

  // valori dei potenziometri
  int16_t temp_pot_value; // valore potenziometro temperatura
  int16_t rh_pot_value;   // valore potenziometro umidità

  // duty cycle per il controllo tramite pwm della temperatura e dell'umidità
  uint16_t temp_pwm_value; // duty cycle per il pwm della temperatura (in ms)
  uint16_t rh_pwm_value;   // duty cycle per il pwm dell'umidità (in ms)

  // codice di errore
  uint16_t error_code;

  // stato dei relè
  bool heat_relay;  // stato relay e led riscaldatore
  bool rh_relay;    // stato relay e led umidificatore
  bool fan_relay;   // stato relay ventola di omogeneizzazione
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
  // timer per letture periferiche (switch, potenziometri, sensore SHT20)
  uint32_t heat_sw;    // lettura switch riscaldatore
  uint32_t rh_sw;      // lettura switch umidificatore
  uint32_t light_sw;   // lettura switch illuminazione
  uint32_t temp_pot;   // lettura potenziometro temperatura
  uint32_t rh_pot;     // lettura potenziometro umidità

  // timers per aggiornamento setpoint e display
  uint32_t temp_setpoint; // aggiornamento ritardato del setpoint temperatura
  uint32_t rh_setpoint;   // aggiornamento ritardato del setpoint umidità
  uint32_t lcd_update;    // aggiornamento display lcd

  // timer di gestione del refill dell'acqua
  uint32_t last_rh_on;     // timestamp ultima accensione umidificatore
  uint32_t refill_counter; // contatore accensione umidificatore

  // timer per segnale pwm
  uint32_t start_pwm; // timestamp inizio ciclo pwm
};

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

// ----------------------------------------------------------------------------
// ------------------- variabili di stato dell'incubatrice --------------------
// ----------------------------------------------------------------------------

// variabili globali di stato dell'incubatrice neonatale
extern struct status status; // variabili di stato dell'incubatrice
extern struct timers timers; // variabili per i timer dell'incubatrice
extern struct pid<TEMP> temp_pid;  // variabili per il PID della temperatura
extern struct pid<RH> rh_pid;    // variabili per il PID dell'umidità

#endif // CONFIG_H
