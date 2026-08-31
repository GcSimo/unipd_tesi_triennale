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
 * @brief Controllo del display lcd.
 *
 * Di seguito sono riportate le tecniche utilizzate per la gestione del
 * display lcd adottate per evitare flickering e aggiornamenti troppo
 * frequenti delle informazioni stampate sul display.
 *
 *
 * ------------ gestione in pagine e temporizzazione delle pagine -------------
 *
 * Le informazioni stampate sul display sono suddivise in pagine, ciascuna con
 * il proprio indice. In questo modo è possibile sapere cosa è stato stampato
 * sul display. Le pagine sono le seguenti:
 *
 * 1. LCD_DATA_PAGE:    mostra temperatura, umidità relativi setpoint, stato
 *                      del controllo attuatori ed eventuali codici di errore
 *
 * 2. LCD_TEMP_SP_PAGE: mostra l'aggiornamento del setpoint di temperatura
 *
 * 3. LCD_RH_SP_PAGE:   mostra l'aggiornamento del setpoint di umidità
 *
 * 4. LCD_MESSAGE_PAGE: mostra messaggi di accensione, spegnimento e refill
 *                      degli attuatori e messaggi di errore del sensore SHT20
 *
 * Ad ogni aggiornamento del display viene aggiornato il timer lcd_update,
 * con l'istante dell'ultima stampa e l'indice di pagina lcd_page in base
 * al contenuto stampato sul display.
 *
 * La pagina relativa alla visualizzazione dei dati (LCD_DATA_PAGE) viene
 * aggiornata solo dopo LCD_UPDATE_PERIOD millisecondi dall'ultima stampa.
 * In questo modo l'utente ha il tempo di leggere le informazioni stampate
 * precedentemente sul display lcd, prima che vengano sovrascritte dalle
 * nuove informazioni.
 *
 * Tutte le restanti pagine non hanno temporizzazione, per cui le nuove
 * informazioni vengono subito stampate sul display lcd sovrascrivendo quelle
 * precedentemente stampate. Questo comportamento è stato scelto perché le
 * informazioni stampate su queste pagine coincidono con eventi specifici
 * che richiedono un aggiornamento immediato del display, come ad esempio
 * l'accensione o lo spegnimento di un attuatore o la visualizzazione di un
 * messaggio di errore del sensore SHT20.
 *
 * NOTA: il messaggio di avvio non ha temporizzazione, viene mostrato solo
 *       durante la fase di avvio e inizializzazione dell'incubatrice.
 *
 *
 * ------------- gestione dei flickering per rapido aggiornamento -------------
 *
 * Per evitare flickering dovuti a frequenti e continui aggiornamenti di tutte
 * le informazioni stampate sul display, è stato scelto di tenere traccia di
 * ciò che è stato precedentemente stampato e aggiornare selettivamente solo
 * i dati modificati.
 *
 * Di fronte ad un cambio di pagina, il display viene completamente aggiornato
 * con le nuove informazioni. Viceversa, se la nuova pagina da stampare è la
 * stessa attualmente stampata, si aggiorneranno solo i dati modificati.
 *
 * Per memorizzare i valori dei dati precedentemente stampati si utilizzano
 * delle apposite variabili che sono mantenute aggiornate all'ultimo valore
 * stampato sul display. Di seguito sono elencati i dati variabili di ogni
 * pagina e le relative variabili che ne tengono traccia:
 *
 * - LCD_DATA_PAGE:
 *    - temperatura ed umidità misurate con relativi setpoint tracciati dalle
 *      variabili old_temp, old_rh, old_temp_setpoint e old_rh_setpoint
 *    - codice di errore tracciato dalla variabile old_error_code
 *
 * - LCD_TEMP_SETPOINT_PAGE e LCD_RH_SETPOINT_PAGE:
 *   ad ogni chiamata delle funzioni di aggiornamento del setpoint corrisponde
 *   ad un aggiornamento del setpoint, quindi andrà sempre aggiornato e non
 *   è necessario memorizzare il valore precedentemente stampato
 *
 * - LCD_MESSAGE_PAGE:
 *   non contengono dati, per cui andrà sempre aggiornata l'intera pagina
 *
 *
 * ------------ gestione dei messaggi di errore del sensore SHT20 -------------
 *
 * I messaggi di errore del sensore SHT20 sono salvati sotto forma di struct
 * lcd_message formate di 4 stringhe di 21 caratteri ciascuna, che permettono
 * di salvare il testo suddiviso nelle 4 righe del display lcd.
 *
 * Siccome i codici di errore del sensore SHT20 vanno da 0x80 a 0x8A, è stato
 * scelto di rappresentare le struct con le descrizioni in un array tale per
 * cui la struct all'indice "i" corrisponde al codice di errore "0x80 +i".
 * In questo modo risulta immediato trovare il messaggio di errore da stampare
 * sul display lcd in base al codice di errore restituito dal sensore SHT20.
 */


// ----------------------------------------------------------------------------
// ------------------- variabili e flag per la paginazione --------------------
// ----------------------------------------------------------------------------

uint8_t lcd_page = 0;

#define LCD_DATA_PAGE 1
#define LCD_TEMP_SETPOINT_PAGE 2
#define LCD_RH_SETPOINT_PAGE 3
#define LCD_MESSAGE_PAGE 4


// ----------------------------------------------------------------------------
// ------------------- variabili per memorizzare dati stampati ----------------
// ----------------------------------------------------------------------------

int16_t old_temp = 0U;
int16_t old_rh = 0U;
int16_t old_temp_setpoint = 0U;
int16_t old_rh_setpoint = 0U;
uint16_t old_error_code = 0U;


// ----------------------------------------------------------------------------
// ------------------- messaggi di errore del sensore SHT20 -------------------
// ----------------------------------------------------------------------------

// struct per memorizzare i messaggi da stampare sul display lcd
struct lcd_message {
  char line1[21]; // prima riga del messaggio
  char line2[21]; // seconda riga del messaggio
  char line3[21]; // terza riga del messaggio
  char line4[21]; // quarta riga del messaggio
};

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


// ----------------------------------------------------------------------------
// ------------------- funzioni per la gestione del display -------------------
// ----------------------------------------------------------------------------

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
   *  2 | C t r l   s t a t u s :   M A N U A L   | per controllo manuale
   *  2 | C t r l   s t a t u s :   A U T O       | per controllo automatico
   *  3 | N o   e r r o r s   d e t e c t e d     | in assenza di errori
   *  3 | E r r o r   c o d e :   x x x x         | in presenza di errori
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
    lcd.setCursor(0, 0);
    lcd.print(F("Nuovo setpoint temp:"));
    lcd.setCursor(11, 1);
    lcd.print((char)223);
    lcd.print('C');
    lcd.setCursor(0, 3);
    lcd.print(F("precedente: "));
    lcd.print(data_to_string(status.temp_setpoint));
    lcd.print((char)223);
    lcd.print('C');
  }

  // stampa nuovo setpoint temperatura
  if (lcd_page != LCD_TEMP_SETPOINT_PAGE || new_temp_setpoint != old_temp_setpoint) {
    lcd.setCursor(6, 1);
    lcd.print(data_to_string(new_temp_setpoint));
    old_temp_setpoint = new_temp_setpoint;
  }

  lcd_page = LCD_TEMP_SETPOINT_PAGE; // aggiornamento pagina
  timers.lcd_update = millis(); // aggiornamento timer
}

// visualizzazione del nuovo setpoint di umidità
void lcd_new_rh_setpoint(float new_rh_setpoint) {
  // pulizia e stampa intestazioni per cambio pagina
  if (lcd_page != LCD_RH_SETPOINT_PAGE) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Nuovo setpoint RH:"));
    lcd.setCursor(11, 1);
    lcd.print('%');
    lcd.setCursor(0, 3);
    lcd.print(F("precedente: "));
    lcd.print(data_to_string(status.rh_setpoint));
    lcd.print('%');
  }

  // aggiornamento setpoint di umidità
  if (lcd_page != LCD_RH_SETPOINT_PAGE || new_rh_setpoint != old_rh_setpoint) {
    lcd.setCursor(6, 1);
    lcd.print(data_to_string(new_rh_setpoint));
    old_rh_setpoint = new_rh_setpoint;
  }

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
  if (errorCode < 0x81 || errorCode > 0x8B)
    errorCode = 0x8B; // errore sconosciuto

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
