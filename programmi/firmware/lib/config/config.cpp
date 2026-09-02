/**
 * @file config.cpp
 * @author Giacomo Simonetto
 * @brief Configurazione di parametri e variabili globali di stato.
 *
 * Dichiarazione dei parametri di configurazione e delle variabili globali
 * di stato dell'incubatrice neonatale.
 */

#include "config.h"

// variabili globali di stato dell'incubatrice neonatale
struct status status; // variabili di stato dell'incubatrice
struct timers timers; // variabili per i timer dell'incubatrice
struct pid<TEMP> temp_pid; // variabili per il PID della temperatura
struct pid<RH> rh_pid;     // variabili per il PID dell'umidità

// sensore SHT20 per temperatura e umidità
sht20 sensor(SHT20_READ_PERIOD);

// display lcd 20x4 con interfaccia I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);
