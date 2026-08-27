/**
 * @file config.cpp
 * @author Giacomo Simonetto
 * @brief File di configurazione con dichiarazioni delle variabili globali
 * di stato dell'incubatrice neonatale.
 */

#include "config.h"

// sensore SHT20 per temperatura e umidità
SHT2x SHT20;

// display lcd 20x4 con interfaccia I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);

// variabili globali di stato dell'incubatrice neonatale
struct status status; // variabili di stato dell'incubatrice
struct timers timers; // variabili timer
struct pid temp_pid; // variabili PID per il controllo della temperatura
struct pid rh_pid; // variabili PID per il controllo dell'umidità

// buffer per conversione da float a stringa
char float_buffer[6];

// converte il numero decimale in stringa formattata correttamente per
// la stampa con due cifre intere e due decimali
//char *float_to_string(float value) {
//  sprintf(float_buffer, "%2d.%02d", (int)value, (int)(value * 100) % 100);
//  return float_buffer;
//}

// calcola l'umidità relativa al raggiungimento del setpoint di temperatura
float rh_at_temp_setpoint(float rh1, float t1, float t2) {
  return rh1 * exp(4283.78 * (t1 - t2) / (243.12 + t1) / (243.12 + t2));
}

// map ottimizzata per int a 16bit (evitando operazioni con long a 32bit)
//int map_16bit(int x, int in_min, int in_max, int out_min, int out_max) {
//  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//}
