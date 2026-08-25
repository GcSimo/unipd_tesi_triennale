/**
 * @file lcd.h
 * @author Giacomo Simonetto
 * @brief Gestione del display lcd dell'incubatrice neonatale
 *
 * Dichiarazione delle funzioni per la gestione del display lcd del pannello
 * frontale dell'incubatrice neonatale.
 */

#ifndef LCD_H
#define LCD_H

#include "config.h"

/**
 * @brief Stampa il messaggio di avvio sul display lcd.
 */
void lcd_boot_message();

/**
 * @brief Stampa lo stato dell'incubatrice su display lcd.
 *
 * Questa funzione stampa sul display lcd:
 * - la temperatura misurata dal sensore SHT20
 * - l'umidità misurata dal sensore SHT20
 * - il setpoint di temperatura impostato
 * - il setpoint di umidità impostato
 * - stato del controllo degli attuatori (manuale o automatico)
 */
void lcd_print_status();

/**
 * @brief Accensione manuale del riscaldatore.
 */
void lcd_man_heat_on();

/**
 * @brief Spegnimento manuale del riscaldatore.
 */
void lcd_man_heat_off();

/**
 * @brief Accensione manuale dell'umidificatore.
 */
void lcd_man_rh_on();

/**
 * @brief Spegnimento manuale dell'umidificatore.
 */
void lcd_man_rh_off();

/**
 * @brief Accensione manuale dell'illuminazione.
 */
void lcd_man_light_on();

/**
 * @brief Spegnimento manuale dell'illuminazione.
 */
void lcd_man_light_off();

/**
 * @brief Visualizzazione del nuovo setpoint di temperatura.
 */
void lcd_new_temp_setpoint();

/**
 * @brief Visualizzazione del nuovo setpoint di umidità.
 */
void lcd_new_rh_setpoint();

/**
 * @brief Stampa il messaggio di refill sul display lcd.
 */
void lcd_refill_message();

/**
 * @brief Stampa il messaggio di errore associato al codice di errore
 * restituito dal sensore SHT20.
 *
 * @param errorCode codice di errore restituito dal sensore SHT20
 */
void lcd_sht20_error(int errorCode);

#endif // LCD_H
