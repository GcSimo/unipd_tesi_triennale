/**
 * @file lcd.cpp
 * @author Giacomo Simonetto
 * @brief Gestione del display lcd dell'incubatrice neonatale
 *
 * Implementazione delle funzioni per la gestione del display lcd del pannello
 * frontale dell'incubatrice neonatale.
 */

#include "lcd.h"
#include "utils.h"

/**
 * @brief Controllo della temporizzazione degli aggiornamenti del display lcd.
 *
 * Ad ogni aggiornamento del display viene aggiornato il timer lcd_update,
 * con l'istante dell'ultima stampa.
 *
 * Le pagine associate all'aggiornamento dei setpoint e dei messaggi non
 * hanno controlli di temporizzazione, per cui vengono stampate/aggiornate
 * immediatamente ad ogni chiamata della funzione.
 *
 * La pagina relativa alla visualizzazione dei dati (temperatura, umidità,
 * setpoint e stato controllo attuatori) viene aggiornata solo dopo
 * LCD_UPDATE_PERIOD millisecondi dall'ultima stampa, in modo da lasciare
 * il tempo all'utente di leggere le pagine precedentemente stampate.
 * Per questo motivo le informazioni potranno avere un ritardo massimo
 * di LCD_UPDATE_PERIOD millisecondi rispetto ai valori reali.
 */

/**
 * @brief Gestione del flickering del display lcd.
 *
 * Per evitare flickering del display dovuti a frequenti e continui
 * aggiornamenti di tutte le scritte sul display, è stato scelto di
 * tenere traccia delle informazioni precedentemente scritte e aggiornare
 * solo i dati modificati.
 *
 * Per tenere traccia di cosa è stato stampato sul display, è stata creata
 * la variabile lcd_page che memorizza quale pagina è attualmente visualizzata
 * sul display. Le pagine sono definite come segue:
 *
 * 1. LCD_DATA_PAGE:          temperatura e umidità misurate dal sensore,
 *                            setpoint impostato e stato controllo attuatori
 *
 * 2. LCD_TEMP_SETPOINT_PAGE: aggiornamento del setpoint di temperatura
 *
 * 3. LCD_RH_SETPOINT_PAGE:   aggiornamento del setpoint di umidità
 *
 * 4. LCD_MESSAGE_PAGE:       comunicazioni di accensione e spegnimento degli
 *                            attuatori e messaggi di errore del sensore SHT20
 *
 * In questo modo, se la pagina attualmente stampata è la stessa che deve
 * essere stampata, è sufficiente aggiornare solo i dati modificati. Di
 * seguito sono elencati i dati che cambiano per ogni pagina:
 *
 * - LCD_DATA_PAGE:
 *   i dati che cambiano sono la temperatura e l'umidità misurate dal sensore,
 *   per cui è stata creata la struct old_sensor, e lo stato del controllo
 *   degli attuatori per cui è stata creata la variabile manual_printed
 *
 * - LCD_TEMP_SETPOINT_PAGE e LCD_RH_SETPOINT_PAGE:
 *   ogni chiamata delle funzioni di aggiornamento del setpoint corrisponde
 *   ad un aggiornamento del setpoint, quindi andrà sempre aggiornato
 *
 * - LCD_MESSAGE_PAGE:
 *   non hanno dati da aggiornare, per cui andrà aggiornata l'intera pagina
 *
 * NOTA: il messaggio di avvio non ha temporizzazione, viene mostrato solo
 *       durante la fase di avvio e inizializzazione dell'incubatrice.
 */

// variabili e costanti per gestione della paginazione
char lcd_page = 0;

#define LCD_DATA_PAGE 1
#define LCD_TEMP_SETPOINT_PAGE 2
#define LCD_RH_SETPOINT_PAGE 3
#define LCD_MESSAGE_PAGE 4

// variabili per memorizzare i dati precedentemente stampati
float old_temp = 0.0f;
float old_rh = 0.0f;
float old_temp_setpoint = 0.0f;
float old_rh_setpoint = 0.0f;
unsigned char old_error_code = 0;

// messaggi di errore del sensore SHT20 da stampare sul display lcd
const struct lcd_message sht20_error_lcd_message[11] PROGMEM = {
    {"    SHT20 Error:    ", "  Error during I2C  ", "   write command    ", "                    "},
    {"    SHT20 Error:    ", "  Error during I2C  ", "     read bytes     ", "                    "},
    {"    SHT20 Error:    ", "  Failed to switch  ", "  off the internal  ", "       heater       "},
    {"    SHT20 Error:    ", "Sensor not connected", "  or does not ack   ", "   on the I2C bus   "},
    {"    SHT20 Error:    ", "  CRC check failed  ", "for the temperature ", "      reading       "},
    {"    SHT20 Error:    ", "  CRC check failed  ", "  for the humidity  ", "      reading       "},
    {"    SHT20 Error:    ", "  CRC check failed  ", "   for the status   ", "      register      "},
    {"    SHT20 Error:    ", "  Heater is in its  ", "mandatory cool-down ", "       period       "},
    {"    SHT20 Error:    ", "  Failed to switch  ", "  on the internal   ", "       heater       "},
    {"    SHT20 Error:    ", " Invalid resolution ", " parameter provided ", "                    "},
    {"    SHT20 Error:    ", "   Unknown error    ", "      occurred      ", "                    "}
  };

// stampa messaggio di avvio sul display lcd
void lcd_boot_message() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F(" Neonatal incubator "));
  lcd.setCursor(0, 1);
  lcd.print(F("     prototype      "));
  lcd.setCursor(0, 3);
  lcd.print(F("Navlab - DEI - UNIPD"));
}


// stampa lo stato dell'incubatrice sul display lcd
void lcd_print_status() {
  // timer per visualizzazione messaggi sul display
  if (lcd_page != LCD_DATA_PAGE && millis() - timers.lcd_update < LCD_UPDATE_PERIOD)
  return;

  /*
   * Mappa dei dati stampati sul display lcd per facilitare
   * l'individuazione delle relative posizioni.
   *
   *    | 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 |
   *    | 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 |
   *  - | - - - - - - - - - - - - - - - - - - - - |
   *  0 | T :   x x . x x ° C   /   x x . x x ° C |
   *  1 | H :   x x . x x %     /   x x . x x %   |
   *  2 | S t a t u s :   A U T O   -   O K       |
   *  2 | S t a t u s :   M A N     -   x x x     | (xxx = error code)
   *  3 |     - -   w h i t e   l i n e   - -     |
   *  - | - - - - - - - - - - - - - - - - - - - - |
   */

  // pulizia e stampa intestazioni per cambio pagina
  if (lcd_page != LCD_DATA_PAGE) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("T:"));
    lcd.setCursor(8, 0);
    lcd.print((char)223);
    lcd.print(F("C / "));
    lcd.setCursor(18, 0);
    lcd.print((char)223);
    lcd.print('C');
    lcd.setCursor(0, 1);
    lcd.print(F("H:"));
    lcd.setCursor(8, 1);
    lcd.print(F("%  / "));
    lcd.setCursor(18, 1);
    lcd.print('%');
    lcd.setCursor(0, 2);
    lcd.print(F("Ctrl status: "));
    lcd.print(status.manual_ctrl ? F("MAN ") : F("AUTO"));
  }

  // aggiornamento temperatura
  if (lcd_page != LCD_DATA_PAGE || status.temp_sht20 != old_temp) {
    lcd.setCursor(3, 0);
    lcd.print(data_to_string(status.temp_sht20));
    old_temp = status.temp_sht20;
  }

  // aggiornamento setpoint di temperatura
  if (lcd_page != LCD_DATA_PAGE || status.temp_setpoint != old_temp_setpoint) {
    lcd.setCursor(13, 0);
    lcd.print(data_to_string(status.temp_setpoint));
    old_temp_setpoint = status.temp_setpoint;
  }

  // aggiornamento umidità
  if (lcd_page != LCD_DATA_PAGE || status.rh_sht20 != old_rh) {
    lcd.setCursor(3, 1);
    lcd.print(data_to_string(status.rh_sht20));
    old_rh = status.rh_sht20;
  }

  // aggiornamento setpoint di umidità
  if (lcd_page != LCD_DATA_PAGE || status.rh_setpoint != old_rh_setpoint) {
    lcd.setCursor(13, 1);
    lcd.print(data_to_string(status.rh_setpoint));
    old_rh_setpoint = status.rh_setpoint;
  }

  // aggiornamento errori
  if (lcd_page != LCD_DATA_PAGE || status.error_code != old_error_code) {
    // pulizia della riga dei messaggi di errore
    lcd.setCursor(0, 3);
    lcd.print(F("                    "));

    // stampa codice di errore se presente, altrimenti messaggio di assenza errori
    lcd.setCursor(0, 3);
    if (!status.error_code) {
      lcd.print(F("No errors detected  "));
    } else {
      lcd.print("Error code: ");
      lcd.print(status.error_code);
    }

    // aggiornamento variabile di stato
    old_error_code = status.error_code;
  }

  lcd_page = LCD_DATA_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// visualizzazione del nuovo setpoint di temperatura
void lcd_new_temp_setpoint(float new_temp_setpoint) {
  // pulizia e stampa intestazioni per cambio pagina
  if (lcd_page != LCD_TEMP_SETPOINT_PAGE) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Nuovo setpoint temp:"));
    lcd.setCursor(11, 2);
    lcd.print((char)223);
    lcd.print('C');
  }

  // stampa nuovo setpoint temperatura
  lcd.setCursor(6, 2);
  lcd.print(data_to_string(new_temp_setpoint));

  lcd_page = LCD_TEMP_SETPOINT_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// visualizzazione del nuovo setpoint di umidità
void lcd_new_rh_setpoint(float new_rh_setpoint) {
  // pulizia e stampa intestazioni per cambio pagina
  if (lcd_page != LCD_RH_SETPOINT_PAGE) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Nuovo setpoint RH:"));
    lcd.setCursor(11, 2);
    lcd.print('%');
  }

  // stampa nuovo setpoint umidità
  lcd.setCursor(6, 2);
  lcd.print(data_to_string(new_rh_setpoint));

  lcd_page = LCD_RH_SETPOINT_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// accensione manuale del riscaldatore
void lcd_man_heat_on() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Accensione manuale"));
  lcd.setCursor(0, 2);
  lcd.print(F("del riscaldatore!"));

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// spegnimento manuale del riscaldatore
void lcd_man_heat_off() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Spegnimento manuale"));
  lcd.setCursor(0, 2);
  lcd.print(F("del riscaldatore!"));

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// accensione manuale dell'umidificatore
void lcd_man_rh_on() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Accensione manuale"));
  lcd.setCursor(0, 2);
  lcd.print(F("dell'umidificatore!"));

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// spegnimento manuale dell'umidificatore
void lcd_man_rh_off() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Spegnimento manuale"));
  lcd.setCursor(0, 2);
  lcd.print(F("dell'umidificatore!"));

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// accensione manuale dell'illuminazione
void lcd_man_light_on() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Accensione manuale"));
  lcd.setCursor(0, 2);
  lcd.print(F("dell'illuminazione!"));

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// spegnimento manuale dell'illuminazione
void lcd_man_light_off() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Spegnimento manuale"));
  lcd.setCursor(0, 2);
  lcd.print(F("dell'illuminazione!"));

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// stampa il messaggio di refill sul display lcd
void lcd_refill_message() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Ricaricare l'acqua"));
  lcd.setCursor(0, 2);
  lcd.print(F("nell'umidificatore!"));

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// stampa il messaggio di errore del sensore SHT20
void lcd_sht20_error(int errorCode) {
  // recupero del messaggio dalla memoria flash
  struct lcd_message message;
  memcpy_P(&message, &sht20_error_lcd_message[errorCode - 0x81], sizeof(struct lcd_message));

  // stampa del messaggio sul display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message.line1);
  lcd.setCursor(0, 1);
  lcd.print(message.line2);
  lcd.setCursor(0, 2);
  lcd.print(message.line3);
  lcd.setCursor(0, 3);
  lcd.print(message.line4);

  lcd_page = LCD_MESSAGE_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}
