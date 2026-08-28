/**
 * @file config.cpp
 * @author Giacomo Simonetto
 * @brief File di configurazione con dichiarazioni delle variabili globali
 * di stato dell'incubatrice neonatale.
 */

#include "config.h"

// variabili globali di stato dell'incubatrice neonatale
struct status status; // variabili di stato dell'incubatrice
struct timers timers; // variabili timer
struct pid temp_pid; // variabili PID per il controllo della temperatura
struct pid rh_pid; // variabili PID per il controllo dell'umidità

// sensore SHT20 per temperatura e umidità
sht20 sensor(SHT20_READ_PERIOD);

// display lcd 20x4 con interfaccia I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);
