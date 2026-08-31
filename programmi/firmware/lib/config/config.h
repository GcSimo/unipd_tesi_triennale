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
#define FAN_RELAY 11   // relè ventola ausiliaria
#define LIGHT_RELAY 12 // relè illuminazione

#define TEMP_POT A0 // potenziometro temperatura
#define RH_POT A1   // potenziometro umidità


// ----------------------------------------------------------------------------
// -------------------- configurazione parametri programma --------------------
// ----------------------------------------------------------------------------

// parametri per potenziometro e setpoint di temperatura
#define TEMP_POT_THLD 5   // soglia per soppressione del rumore
#define TEMP_POT_MIN 10   // valore minimo del potenziometro (misurato 3-4)
#define TEMP_POT_MAX 1015 // valore massimo del potenziometro (misurato 1019-1020)
#define TEMP_DEF_SP 3600  // setpoint iniziale temperatura
#define TEMP_MIN_SP 2500  // valore minimo del setpoint
#define TEMP_MAX_SP 4000  // valore massimo del setpoint
#define TEMP_STEP_SP 10   // passo di variazione del setpoint
#define TEMP_ERR_THLD 200 // soglia di errore per temperatura
#define TEMP_ERR_HYST 20 // isteresi per errore di temperatura

// parametri potenziometro e setpoint di umidità
#define RH_POT_THLD 5     // soglia per soppressione del rumore
#define RH_POT_MIN 10     // valore minimo del potenziometro (misurato 3-4)
#define RH_POT_MAX 1015   // valore massimo del potenziometro (misurato 1019-1020)
#define RH_DEF_SP 2000    // setpoint iniziale umidità
#define RH_MIN_SP 2000    // valore minimo del setpoint
#define RH_MAX_SP 9000    // valore massimo del setpoint
#define RH_STEP_SP 100    // passo di variazione del setpoint
#define RH_ERR_THLD 1000  // soglia di errore per umidità
#define RH_ERR_HYST 100 // isteresi per errore di umidità

// parametri per la gestione dei timer
#define SW_READ_PERIOD 50         // tempo di campionamento degli switch
#define TEMP_POT_READ_PERIOD 100  // tempo di campionamento del temp_pot
#define RH_POT_READ_PERIOD 100    // tempo di campionamento dell'rh_pot
#define SP_UPDATE_DELAY 1300      // ritardo di aggiornamento del setpoint
#define SHT20_READ_PERIOD 1000    // tempo di campionamento dell'SHT20
#define LCD_UPDATE_PERIOD 1000    // tempo di aggiornamento dell'lcd
#define ERR_TIMER 1100            // tempo di durata degli errori temporizzati
#define REFILL_INTERVAL 3600000UL // durata dell'acqua prima del refill
// per ora è impostato genericamente a 1 ora, poi servirà andare a tararla
// utilizzando il dato di 0,1017 g/s di acqua nebulizzata dalla tesi di Vanni

// parametri per il controllo della temperatura
#define TEMP_CTRL 2        // 0 = no control | 1 = hysteresis | 2 = PID
#define TEMP_HYS_THLD 25   // soglia di isteresi per temperatura
#define TEMP_PID_KP 50.0f  // guadagno proporzionale del PID per temperatura
#define TEMP_PID_KI 0.5f   // guadagno integrale del PID per temperatura
#define TEMP_PID_KD 20.0f  // guadagno derivativo del PID per temperatura
#define TEMP_PID_KW 0.0f   // guadagno anti-windup del PID per temperatura
#define TEMP_PID_WINDUP 1  // 0 = no windup | 1 = clamping | 2 = back calculation

// parametri per il controllo dell'umidità
#define RH_CTRL 2        // 0 = no control | 1 = hysteresis | 2 = PID
#define RH_HYS_THLD 200  // soglia di isteresi per umidità
#define RH_PID_KP 0.0f   // guadagno proporzionale del PID per umidità
#define RH_PID_KI 0.0f   // guadagno integrale del PID per umidità
#define RH_PID_KD 0.0f   // guadagno derivativo del PID per umidità
#define RH_PID_KW 0.0f   // guadagno anti-windup del PID per umidità
#define RH_PID_WINDUP 0  // 0 = no windup | 1 = clamping | 2 = back calculation

// parametri per la gestione del segnale pwm
#define PWM_PERIOD 8000   // periodo del pwm per controllo attuatori (in ms) (max 30000 ms)
#define PWM_MIN_TIME_ON 500  // intervallo minimo di accensione attuatori (in ms)
#define PWM_MIN_TIME_OFF 500 // intervallo minimo di spegnimento attuatori (in ms)

// parametri generali per i controllori pid
#define PID_MIN_OUTPUT 0   // limite minimo dell'output del PID
#define PID_MAX_OUTPUT 100 // limite massimo dell'output del PID
#define PID_DATA_PERIOD SHT20_READ_PERIOD // periodo di acquisizione dei dati
#define PID_UPDATE_PERIOD PWM_PERIOD      // periodo di aggiornamento dell'output

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
struct pid {
  // guadagni del controllore PID
  float kp;  // guadagno proporzionale
  float ki;  // guadagno integrale
  float kd;  // guadagno derivativo
  float kw;  // guadagno windup

  // componenti del controllore PID
  float proportional; // componente proporzionale
  float integral;     // componente integrale
  float derivative;   // componente derivativa
  float output;       // output puro del PID non limitato

  // variabili e accumulatori intermedi per calcolo PID
  float deriv_c1; // primo coefficiente per calcolo derivata
  float deriv_c2; // secondo coefficiente per calcolo derivata
  int32_t sum1;   // somma degli errori tra setpoint e valore misurato
  int32_t sum2;  // somma dei valori misurati
  int32_t sum3;  // somma dei prodotti tra indice e valore misurato

  // numero di misurazioni
  uint8_t data_count; // misurazioni ricevute ed elaborate
  uint8_t expected_data_count; // misurazioni attese

  // metodo di anti-windup da utilizzare
  uint8_t anti_windup; // 0 = no windup | 1 = clamping | 2 = back calculation
};

// ----------------------------------------------------------------------------
// ------------------- variabili di stato dell'incubatrice --------------------
// ----------------------------------------------------------------------------

// variabili globali di stato dell'incubatrice neonatale
extern struct status status; // variabili di stato dell'incubatrice
extern struct timers timers; // variabili per i timer dell'incubatrice
extern struct pid temp_pid;  // variabili per il PID della temperatura
extern struct pid rh_pid;    // variabili per il PID dell'umidità

#endif // CONFIG_H
