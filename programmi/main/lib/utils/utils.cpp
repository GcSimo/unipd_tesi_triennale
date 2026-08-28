/**
 * @file utils.cpp
 * @author Giacomo Simonetto
 * @brief Funzioni di utilità per l'incubatrice neonatale
 *
 * Implementazione delle funzioni di utilità per l'incubatrice neonatale.
 */

#include "utils.h"
#include "error.h"

// ----------------------------------------------------------------------------
// --------- gestione della conversione da intero a 4 cifre a stringa ---------
// ----------------------------------------------------------------------------

// stringa per overflow dei valori di temperatura e umidità
const char value_overflow[] PROGMEM = "xx.xx";

// buffer per conversione da float a stringa
char float_buffer[6];


// ----------------------------------------------------------------------------
// ------- calcolo dell'umidità relativa al raggiungimento del setpoint -------
// ----------------------------------------------------------------------------

// converte un dato (temperatura o umidità) in una stringa formattata
char *data_to_string(uint16_t value) {
  // controllo overflow per float negativi o superiori a 100
  if (value > 9999) {
    strcpy_P(float_buffer, value_overflow); // copia la stringa di overflow nel buffer
    err_set(ERR_PRINT_OVERFLOW); // impostazione bit di errore
  } else {
    // suddivisione in cifre e scrittura nel buffer di output
    float_buffer[5] = '\0'; // terminatore di fine stringa
    float_buffer[4] = '0' + value % 10; value /= 10; // centesimi
    float_buffer[3] = '0' + value % 10; value /= 10; // decimi
    float_buffer[2] = '.'; // punto decimale
    float_buffer[1] = '0' + value % 10; value /= 10; // unità
    float_buffer[0] = (value == 0) ? ' ' : ('0' + value); // decine o spazio se 0
  }

  // restituzione del puntatore al buffer con la stringa formattata
  return float_buffer;
}

// calcola l'umidità relativa al raggiungimento del setpoint di temperatura
uint16_t rh_at_temp_setpoint(uint16_t rh1, uint16_t t1, uint16_t t2) {
  return (uint16_t)round(rh1 * exp(4283.78 * (t1 - t2) / (243.12 + t1) / (243.12 + t2)));
}
