/**
 * @file main.cpp
 * @author Giacomo Simonetto
 * @brief Firmware per incubatrice neonatale
 *
 * Firmware per il controllo di un'incubatrice neonatale, con gestione di
 * temperatura, umidità e pannello frontale con display lcd e switch.
 */


// ----------------------------------------------------------------------------
// -------------------------------- dipendenze --------------------------------
// ----------------------------------------------------------------------------

#include "config.h"    // parametri di configurazione e variabili globali
#include "error.h"     // gestione errori e messaggi di errore
#include "utils.h"     // funzioni di utilità generiche
#include "pid.h"       // gestione controllori PID
#include "actuators.h" // gestione accensinoe e spegnimento degli attuatori
#include "serial.h"    // gestione messaggi su serial monitor
#include "lcd.h"       // gestione messaggi su display lcd


// ----------------------------------------------------------------------------
// ----------------------- variabili globali temporanee -----------------------
// ----------------------------------------------------------------------------

int16_t sensor_read = 0;       // valore appena letto dal sensore SHT20
int16_t pot_read = 0;          // valore appena letto dai potenziometri
int16_t new_temp_setpoint = 0; // setpoint appena calcolato
int16_t new_rh_setpoint = 0;   // setpoint appena calcolato


// ----------------------------------------------------------------------------
// ----------------------- helper functions per il main -----------------------
// ----------------------------------------------------------------------------

/**
 * @brief Attivazione del controllo manuale degli attuatori.
 *
 * Imposta lo stato degli attuatori in modo che coincida con lo stato degli
 * switch del pannello frontale e disabilita il controllo automatico settando
 * opportunamente il flag status.manual_ctrl a MANUAL_CTRL.
 */
void set_manual_ctrl();

/**
 * @brief Attivazione del controllo automatico degli attuatori.
 *
 * Riattiva il controllo automatico degli attuatori resettando i timer del
 * ciclo PWM, resettando i vecchi dati accumulati dai PID e impostando il
 * flag status.manual_ctrl a AUTO_CTRL.
 */
void set_auto_ctrl();

/**
 * @brief Verifica se è necessario un refill dell'acqua.
 *
 * Questa funzione verifica se è necessario effettuare un refill dell'acqua
 * dell'incubatrice. Controlla se il periodo di tempo in cui l'umidificatore
 * è stato acceso supera il valore di REFILL_INTERVAL. Se è necessario un
 * refill, la funzione spegne l'umidificatore e accende il led di refill,
 * attivando il blocco automatico dell'umidificatore.
 *
 * Per far ripristinare il funzionamento dell'umidificatore, è necessario
 * effettuare un refill dell'acqua e successivamente riavviare l'incubatrice
 * neonatale dal tasto reset della scheda Arduino. In questo modo tutti i
 * contatori vengono azzerati.
 *
 * Per funzionare correttamente, questa funzione deve essere invocata ad ogni
 * iterazione del loop principale.
 *
 * @return true refill necessario, umidificatore spento e led di refill acceso
 * @return false refill non necessario, nessuna azione eseguita
 */
bool check_refill();


// ----------------------------------------------------------------------------
// ------------------------------ setup function ------------------------------
// ----------------------------------------------------------------------------

void setup() {
  // inizializzazione seriale e I2C
  Serial.begin(115200);

  // inizializzazione protocollo Wire per I2C
  Wire.begin();

  // inizializzazione display lcd e stampa messaggio di avvio
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // messaggio di avvio su serial monitor e display lcd
  serial_boot_message();
  lcd_boot_message();

  // inizializzazione pin
  pinMode(ALARM_LED, OUTPUT);
  pinMode(HEAT_LED, OUTPUT);
  pinMode(RH_LED, OUTPUT);
  pinMode(REFILL_LED, OUTPUT);

  pinMode(HEAT_RELAY, OUTPUT);
  pinMode(RH_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LIGHT_RELAY, OUTPUT);

  pinMode(HEAT_SW, INPUT);
  pinMode(RH_SW, INPUT);
  pinMode(LIGHT_SW, INPUT);

  // inizializzazione stato iniziale dei pin con led, relè
  digitalWrite(ALARM_LED, LED_OFF);
  digitalWrite(HEAT_LED, LED_OFF);
  digitalWrite(RH_LED, LED_OFF);
  digitalWrite(REFILL_LED, LED_OFF);

  digitalWrite(HEAT_RELAY, RELAY_OFF);
  digitalWrite(RH_RELAY, RELAY_OFF);
  digitalWrite(FAN_RELAY, RELAY_OFF);
  digitalWrite(LIGHT_RELAY, RELAY_OFF);

  // inizializzazione stato iniziale delle variabili globali
  memset(&status, 0, sizeof(status));
  memset(&timers, 0, sizeof(timers));
  memset(&temp_pid, 0, sizeof(temp_pid));
  memset(&rh_pid, 0, sizeof(rh_pid));

  // inizializzazione setpoint di temperatura e umidità
  status.temp_setpoint = new_temp_setpoint = TEMP_DEF_SP;
  status.rh_setpoint = new_rh_setpoint = RH_DEF_SP;

  // inizializzazione controllori PID
  #if TEMP_CTRL == 2
    pid_init(temp_pid, TEMP_PID_KP, TEMP_PID_KI, TEMP_PID_KD, TEMP_PID_KW, TEMP_PID_WINDUP);
  #endif
  #if RH_CTRL == 2
    pid_init(rh_pid, RH_PID_KP, RH_PID_KI, RH_PID_KD, RH_PID_KW, RH_PID_WINDUP);
  #endif

  // inizializzazione sensore SHT20 e controllo errori del sensore
  while (!sensor.begin()) {
    lcd_sht20_error(sensor.get_error()); // stampa messaggio su display lcd
    serial_sht20_error(sensor.get_error()); // stampa messaggio su serial monitor
    digitalWrite(ALARM_LED, LED_ON); // accensione led di allarme
    delay(1000); // attesa di 1 secondo prima di riprovare
  }
  digitalWrite(ALARM_LED, LED_OFF); // spegnimento led in assenza di errori

  // ritardo prima di iniziare il loop principale
  delay(2000);

  // inizializzazione valori potenziometri e switch
  status.temp_pot_value = analogRead(TEMP_POT);
  status.rh_pot_value = analogRead(RH_POT);
  status.heat_sw = digitalRead(HEAT_SW);
  status.rh_sw = digitalRead(RH_SW);
  status.light_sw = digitalRead(LIGHT_SW);

  // attivazione luce in base al valore dello switch
  if (status.light_sw)
    light_turn_on(); // accensione illuminazione

  // inizializzazione timers del sensore e del ciclo pwm
  timers.start_pwm = millis();
  sensor.init_timer();
}


// ----------------------------------------------------------------------------
// ------------------------------ loop function -------------------------------
// ----------------------------------------------------------------------------

void loop() {
  // --------------------------------------------------------------------------
  // ------ gestione switch riscaldatore, umidificatore e illuminazione -------
  // --------------------------------------------------------------------------

  // -------------------- cambio stato switch riscaldatore --------------------
  if (millis() - timers.heat_sw >= SW_READ_PERIOD && status.heat_sw != digitalRead(HEAT_SW)) {
    timers.heat_sw = millis(); // aggiornamento timer
    status.heat_sw = !status.heat_sw; // aggiornamento stato switch

    // switch riscaldatore attivo
    if (status.heat_sw) {
      heat_turn_on(); // accensione riscaldatore e led associato
      lcd_man_heat_on(); // stampa messaggio di accensione sul display
      serial_man_heat_on(); // stampa messaggio di accensione su serial monitor
    }

    // switch riscaldatore spento
    else {
      heat_turn_off(); // spegnimento riscaldatore e led associato
      lcd_man_heat_off(); // stampa messaggio di spegnimento sul display
      serial_man_heat_off(); // stampa messaggio di spegnimento su serial monitor
    }

    // attivazione controllo manuale
    set_manual_ctrl();
  }

  // ------------------- cambio stato switch umidificatore --------------------
  if (millis() - timers.rh_sw >= SW_READ_PERIOD && status.rh_sw != digitalRead(RH_SW)) {
    timers.rh_sw = millis(); // aggiornamento timer
    status.rh_sw = !status.rh_sw; // aggiornamento stato switch

    // avviso di refill acqua
    if (status.refill_led && status.rh_sw) {
      lcd_refill_message(); // stampa messaggio refill sul display
    }

    // switch umidificatore attivo
    else if (!status.refill_led && status.rh_sw) {
      rh_turn_on(); // accensione umidificatore e led associato
      lcd_man_rh_on(); // stampa messaggio di accensione sul display
      serial_man_rh_on(); // stampa messaggio di accensione su serial monitor
    }

    // switch umidificatore spento
    else if (!status.refill_led && !status.rh_sw) {
      rh_turn_off(); // spegnimento umidificatore e led associato
      lcd_man_rh_off(); // stampa messaggio di spegnimento sul display
      serial_man_rh_off(); // stampa messaggio di spegnimento su serial monitor
    }

    // attivazione controllo manuale
    set_manual_ctrl();
  }

  // ------------------- cambio stato switch illuminazione --------------------
  if (millis() - timers.light_sw >= SW_READ_PERIOD && status.light_sw != digitalRead(LIGHT_SW)) {
    timers.light_sw = millis(); // aggiornamento timer
    status.light_sw = !status.light_sw; // aggiornamento stato switch

    // switch illuminazione attivo
    if (status.light_sw) {
      light_turn_on(); // accensione illuminazione
      lcd_man_light_on(); // stampa messaggio di accensione sul display
      serial_man_light_on(); // stampa messaggio di accensione su serial monitor
    }
    // switch illuminazione spento
    else {
      light_turn_off(); // spegnimento illuminazione
      lcd_man_light_off(); // stampa messaggio di spegnimento sul display
      serial_man_light_off(); // stampa messaggio di spegnimento su serial monitor
    }
  }


  // --------------------------------------------------------------------------
  // ------------ aggiornamento setpoint di temperatura e umidità -------------
  // --------------------------------------------------------------------------

  /**
   * @brief Aggiornamento dei setpoint di temperatura e umidità.
   *
   * Il processo di aggiornamento del setpoint avviene nelle seguenti fasi:
   *
   * 1. lettura del potenziometro ogni XX_POT_TIMER millisecondi
   * 2. controllo di soglia per eliminare disturbi e rumore elettrico
   * 3. calcolo del nuovo setpoint di temperatura o umidità e verifica che
   *    rientri nei limiti previsti XX_MIN_SP e XX_MAX_SP, con conseguente
   *    stampa del nuovo valore sul display lcd
   * 4. avvio del timer di ultima modifica del setpoint se il nuovo valore
   *    è diverso dal precedente e attivazione del controllo automatico
   * 5. aggiornamento del setpoint se il nuovo valore è diverso dal precedente
   *    e se è trascorso il tempo di ritardo previsto SETPOINT_UPDATE_DELAY
   *    e stampa del nuovo setpoint su serial monitor
   *
   * Nel caso di potenziometri ad alta impedenza, è possibile che la lettura
   * analogica del valore del potenziometro venga influenzata dalla carica
   * residua del condensatore dell'ADC, provocando valori errati. Questo
   * fenomeno è denominato crosstalk dell'ADC. Per evitarlo, viene effettuata
   * una ulteriore lettura preliminare in modo che il condensatore abbia tempo
   * di caricarsi correttamente prima della lettura effettiva.
   */

  // controllo timer per lettura potenziometro temperatura
  if (millis() - timers.temp_pot >= TEMP_POT_READ_PERIOD) {
    timers.temp_pot += TEMP_POT_READ_PERIOD;  // aggiornamento timer
    analogRead(TEMP_POT); // lettura per evitare crosstalk dell'ADC
    pot_read = analogRead(TEMP_POT); // 1. lettura potenziometro temperatura

    // 2. controllo threshold
    if (abs(status.temp_pot_value - pot_read) >= TEMP_POT_THLD) {
      status.temp_pot_value = pot_read; // aggiornamento valore potenziometro temperatura

      // 3. calcolo nuovo setpoint di temperatura
      new_temp_setpoint = map(pot_read, TEMP_POT_MIN, TEMP_POT_MAX, TEMP_MIN_SP / TEMP_STEP_SP, TEMP_MAX_SP / TEMP_STEP_SP) * TEMP_STEP_SP;
      if (new_temp_setpoint < TEMP_MIN_SP) new_temp_setpoint = TEMP_MIN_SP; // controllo limite inferiore
      if (new_temp_setpoint > TEMP_MAX_SP) new_temp_setpoint = TEMP_MAX_SP; // controllo limite superiore

      // stampa su display lcd
      lcd_new_temp_setpoint(new_temp_setpoint);

      // 4. controllo variazioni del setpoint di temperatura
      if (status.temp_setpoint != new_temp_setpoint) {
        timers.temp_setpoint = millis(); // aggiornamento timer
        set_auto_ctrl(); // attivazione controllo automatico
      }
    }
  }

  // 5. aggiornamento effettivo del setpoint di temperatura
  if (status.temp_setpoint != new_temp_setpoint && millis() - timers.temp_setpoint >= SP_UPDATE_DELAY) {
    status.temp_setpoint = new_temp_setpoint; // aggiornamento setpoint precedente
    serial_new_temp_setpoint(); // stampa su serial monitor

  }

  // controllo timer per lettura potenziometro umidità
  if (millis() - timers.rh_pot >= RH_POT_READ_PERIOD) {
    analogRead(RH_POT); // lettura per evitare crosstalk dell'ADC
    timers.rh_pot += RH_POT_READ_PERIOD; // aggiornamento timer
    pot_read = analogRead(RH_POT); // 1. lettura potenziometro umidità

    // 2. controllo threshold
    if (abs(status.rh_pot_value - pot_read) >= RH_POT_THLD) {
      status.rh_pot_value = pot_read; // aggiornamento valore potenziometro umidità

      // 3. calcolo nuovo setpoint di umidità
      new_rh_setpoint = map(pot_read, RH_POT_MIN, RH_POT_MAX, RH_MIN_SP / RH_STEP_SP, RH_MAX_SP / RH_STEP_SP) * RH_STEP_SP;
      if (new_rh_setpoint < RH_MIN_SP) new_rh_setpoint = RH_MIN_SP; // controllo limite inferiore
      if (new_rh_setpoint > RH_MAX_SP) new_rh_setpoint = RH_MAX_SP; // controllo limite superiore

      // stampa su display lcd
      lcd_new_rh_setpoint(new_rh_setpoint);

      // 4. controllo variazioni del setpoint di umidità
      if (status.rh_setpoint != new_rh_setpoint) {
        timers.rh_setpoint = millis(); // aggiornamento timer
        set_auto_ctrl(); // attivazione controllo automatico
      }
    }
  }

  // 5. aggiornamento effettivo del setpoint di umidità
  if (status.rh_setpoint != new_rh_setpoint && millis() - timers.rh_setpoint >= SP_UPDATE_DELAY) {
    status.rh_setpoint = new_rh_setpoint; // aggiornamento setpoint precedente
    serial_new_rh_setpoint(); // stampa su serial monitor
  }


  // --------------------------------------------------------------------------
  // --------------------- lettura dati dal sensore SHT20 ---------------------
  // --------------------------------------------------------------------------

  /**
   * @brief Gestione del sensore SHT20.
   *
   * Come illustrato nella classe sht20, per leggere le misurazioni del
   * sensore SHT20 è necessario invocare ripetutamente la funzione update()
   * fino a quando non restituisce true. Tale funzione si occupa di gestire
   * in autonomia le fasi di richiesta, attesa e lettura dei dati dal sensore.
   *
   * Se la funzione update() restituisce false, significa che le misurazioni
   * non sono ancora disponibili o che si è verificato un errore. Va quindi
   * verificata la presenza di eventuali errori con la funzione get_error().
   *
   * Quando la funzione update() restituisce true, significa che le nuove
   * misurazioni sono disponibili e vanno lette entrambe tramite le funzioni
   * get_temperature() e get_humidity(), altrimenti non è possibile effettuare
   * la richiesta di nuove misurazioni.
   *
   * All'arrivo di nuove misurazioni si effettuano le seguenti operazioni:
   * 1. lettura della temperatura e dell'umidità (in float) e conversione
   *    dei valori in interi a 4 cifre
   * 2. verifica di eventuali overflow dei valori e salvataggio dei valori
   *    nella struct status
   * 3. gestione degli attuatori se in modalità automatica
   * 4. stampa di log sullo stato dell'incubatrice su serial monitor
   *
   * La gestione degli attuatori dipende dal tipo di controllo selezionato:
   * - per i controllori ad isteresi ON/OFF, si verifica se la differenza tra
   *   il setpoint e la misurazione supera la soglia di isteresi, modificando
   *   di conseguenza lo stato degli attuatori
   * - per i controllori PID, si aggiungono i nuovi dati al controllore con
   *   la funzione pid_add_data().
   */

  // aggiorna il sensore e verifica la presenza di nuovi dati
  if (!sensor.update()) {
    // verifica presenza di nuovi errori nel sensore SHT20
    if (sensor.get_error() && !err_check(ERR_SHT20)) {
      serial_sht20_error(sensor.get_error()); // stampa errore su serial monitor
      lcd_sht20_error(sensor.get_error()); // stampa errore su display lcd
      err_set(ERR_SHT20); // impostazione bit di errore per il sensore SHT20
    }
  } else {
    // 1. lettura della temperatura dal sensore e conversione in intero a 4 cifre
    sensor_read = (int16_t)(sensor.get_temperature() * 100 + 0.5);

    // 2. verifica overflow della temperatura e gestione errori
    if (sensor_read < 0) {        // verifica temperatura negativa
      err_set(ERR_TEMP_OVERFLOW); //  - impostazione bit di errore
      status.temp_sht20 = 0;      //  - assegnazione valore minimo
    }
    else if (sensor_read > 9999) { // verifica temperatura troppo alta
      err_set(ERR_TEMP_OVERFLOW);   //  - impostazione bit di errore
      status.temp_sht20 = 9999;     //  - assegnazione valore massimo
    }
    else {                             // temperatura valida
      err_clear(ERR_TEMP_OVERFLOW);       //  - rimozione bit di errore
      status.temp_sht20 = sensor_read; //  - salvataggio valore
    }

    // 1. lettura dell'umidità dal sensore e conversione in intero a 4 cifre
    sensor_read = (int16_t)(sensor.get_humidity() * 100 + 0.5);

    // 2. verifica overflow dell'umidità e gestione errori
    if (sensor_read < 0) {      // verifica umidità negativa
      err_set(ERR_RH_OVERFLOW); //  - impostazione bit di errore
      status.rh_sht20 = 0;      //  - assegnazione valore minimo
    }
    else if (sensor_read > 9999) { // verifica umidità troppo alta
      err_set(ERR_RH_OVERFLOW);     //  - impostazione bit di errore
      status.rh_sht20 = 9999;       //  - assegnazione valore massimo
    }
    else {                           // umidità valida
      err_clear(ERR_RH_OVERFLOW);       //  - rimozione bit di errore
      status.rh_sht20 = sensor_read; //  - salvataggio valore
    }

    // 3. controllo degli attuatori se in modalità automatica
    if (!status.manual_ctrl) {
      // controllo ad isteresi ON/OFF della temperatura
      #if TEMP_CTRL == 1 // HYSTERESIS
        if (!status.heat_relay && status.temp_setpoint - status.temp_sht20 > TEMP_HYS_THLD) {
          heat_turn_on();
          serial_auto_heat_on();
        }
        else if (status.heat_relay && status.temp_sht20 - status.temp_setpoint > TEMP_HYS_THLD) {
          heat_turn_off();
          serial_auto_heat_off();
        }
      #endif

      // controllo tramite PID della temperatura
      #if TEMP_CTRL == 2 // PID
        pid_add_data(temp_pid, status.temp_setpoint - status.temp_sht20, status.temp_sht20);
      #endif

      // controllo ad isteresi ON/OFF dell'umidità
      #if RH_CTRL == 1 // HYSTERESIS
        if (!status.rh_relay && !status.refill_led && status.rh_setpoint - rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) > RH_HYS_THLD) {
          rh_turn_on();
          serial_auto_rh_on();
        }
        else if (status.rh_relay && rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint) - status.rh_setpoint > RH_HYS_THLD) {
          rh_turn_off();
          serial_auto_rh_off();
        }
      #endif

      // controllo tramite PID dell'umidità
      #if RH_CTRL == 2 // PID
        pid_add_data(rh_pid, status.rh_setpoint - rh_at_temp_setpoint(status.rh_sht20, status.temp_sht20, status.temp_setpoint), status.rh_sht20);
      #endif
    }

    // 4. stampa di log sullo stato dell'incubatrice su serial monitor
    serial_datalog();
  }


  // --------------------------------------------------------------------------
  // ------------------- controllo PWM per attuatori binari -------------------
  // --------------------------------------------------------------------------

  /**
   * @brief Implementazione del controllo pwm per gli attuatori.
   *
   * Siccome gli attuatori hanno un output binario (acceso/spento), mentre
   * un controllore PID produce un output discreto (0-100), si implementa
   * un sistema di conversione dell'output discreto del PID in un segnale
   * pwm binario per gli attuatori.
   *
   * Le variabili "temp_pwm_value" e "rh_pwm_value" contengono la larghezza
   * del duty cycle in millisecondi per il controllo rispettivamente del
   * riscaldatore e dell'umidificatore. È stato scelto di memorizzare il
   * valore del duty cycle in millisecondi per evitare di dover eseguire
   * continue conversioni da un valore discreto (0-100) a un intervallo
   * di tempo in millisecondi.
   *
   * All'inizio di ogni ciclo, si effettua l'aggiornamento dei valori del duty
   * cycle calcolando l'output del PID ed effettuando le opportune conversioni
   * per ottenere il valore in millisecondi. Si applicano inoltre dei vincoli
   * per evitare rapide accensioni e spegnimenti degli attuatori, impostando
   * un periodo minimo di accensione e spegnimento oltre il quale il duty
   * cycle viene forzato a 0 o al periodo massimo del ciclo pwm.
   *
   * Successivamente, si controlla lo stato degli attuatori confrontando il
   * tempo trascorso dall'inizio del ciclo pwm con il valore del duty cycle.
   * Se il tempo trascorso è inferiore al duty cycle, l'attuatore viene acceso,
   * altrimenti viene spento.
   *
   * Il controllo pwm viene disabilitato quando è attivo il controllo manuale.
   * Alla riattivazione del controllo automatico, il timer del ciclo PWM viene
   * reimpostato a "millis() - PWM_PERIOD" per garantire che il ciclo pwm
   * inizi immediatamente.
   */

  // implementazione PWM se è attivo almeno un controllore PID
  #if TEMP_CTRL == 2 || RH_CTRL == 2
    // controllo pwm in modalità automatica
    if (!status.manual_ctrl) {
      // inizio del ciclo di controllo PWM ogni CTRL_PWM_PERIOD millisecondi
      if (millis() - timers.start_pwm >= PWM_PERIOD) {
        timers.start_pwm += PWM_PERIOD; // aggiornamento timer

        // aggiornamento dei valori del duty cycle
        #if TEMP_CTRL == 2 // PID per la temperatura
          // conversione output del PID della temperatura in duty cycle
          status.temp_pwm_value = (pid_update_output(temp_pid) - PID_MIN_OUTPUT) * PWM_PERIOD / (PID_MAX_OUTPUT - PID_MIN_OUTPUT) + 0.5f;

          // applicazione dei vincoli sul duty cycle
          if (status.temp_pwm_value < PWM_MIN_TIME_ON)
            status.temp_pwm_value = 0;
          else if (status.temp_pwm_value > PWM_PERIOD - PWM_MIN_TIME_OFF)
            status.temp_pwm_value = PWM_PERIOD;
        #endif
        #if RH_CTRL == 2 // PID per l'umidità
          // conversione output del PID dell'umidità in duty cycle
          status.rh_pwm_value = (pid_update_output(rh_pid) - PID_MIN_OUTPUT) * PWM_PERIOD / (PID_MAX_OUTPUT - PID_MIN_OUTPUT) + 0.5f;

          // applicazione dei vincoli sul duty cycle
          if (status.rh_pwm_value < PWM_MIN_TIME_ON)
            status.rh_pwm_value = 0;
          else if (status.rh_pwm_value > PWM_PERIOD - PWM_MIN_TIME_OFF)
            status.rh_pwm_value = PWM_PERIOD;
        #endif
      }

      // controllo del duty cycle per il riscaldatore controllato in PID
      #if TEMP_CTRL == 2
        if (!status.heat_relay && millis() - timers.start_pwm < status.temp_pwm_value) {
          heat_turn_on();
          serial_auto_heat_on();
        }
        else if (status.heat_relay && millis() - timers.start_pwm >= status.temp_pwm_value) {
          heat_turn_off();
          serial_auto_heat_off();
        }
      #endif

      // controllo del duty cycle per l'umidificatore controllato in PID
      #if RH_CTRL == 2
        if (!status.rh_relay && !status.refill_led && millis() - timers.start_pwm < status.rh_pwm_value) {
          rh_turn_on();
          serial_auto_rh_on();
        }
        else if (status.rh_relay && millis() - timers.start_pwm >= status.rh_pwm_value) {
          rh_turn_off();
          serial_auto_rh_off();
        }
      #endif
    }
  #endif


  // --------------------------------------------------------------------------
  // -------------------- gestione errori e led di allarme --------------------
  // --------------------------------------------------------------------------

  check_temp_range(); // controllo temperatura fuori dal range di sicurezza
  check_rh_range();   // controllo umidità fuori dal range di sicurezza
  update_temporized_errors(); // aggiornamento stato errori temporizzati
  update_alarm_led(); // aggiornamento stato led di allarme


  // --------------------------------------------------------------------------
  // ----------------- gestione refill acqua e led di refill ------------------
  // --------------------------------------------------------------------------

  /**
   * @brief Verifica necessità di refill dell'acqua.
   *
   * Per capire se è necesasrio un refill dell'acqua, si verifica se la somma
   * dei seguenti valori supera REFILL_INTERVAL:
   *
   * 1. durata delle precedenti accensioni dell'umidificatore data dalla
   *   variabile timers.refill_counter
   *
   * 2. tempo trascorso dall'ultima accensione, data dalla differenza
   *   (millis() - timers.last_rh_on) moltiplicata per lo stato attuale
   *   dell'umidificatore (status.rh_relay), così da considerare solo
   *   il tempo trascorso se l'umidificatore è acceso
   */

  // verifica se è necessario un refill dell'acqua
  if (!status.refill_led && timers.refill_counter + (millis() - timers.last_rh_on) * status.rh_relay >= REFILL_INTERVAL) {
     rh_turn_off();                    // spegnimento umidificatore
     lcd_refill_message();             // stampa messaggio di refill sul display lcd
     serial_refill_message();          // stampa messaggio di refill su serial monitor
     digitalWrite(REFILL_LED, LED_ON); // accensione led di refill
     status.refill_led = true;         // aggiornamento stato led di refill
  }


  // --------------------------------------------------------------------------
  // ------------ aggiornamento display lcd con stato incubatrice -------------
  // --------------------------------------------------------------------------

  // aggiorna lo stato dell'incubatrice sul display lcd
  lcd_print_status();
}

// ----------------------------------------------------------------------------
// ----------------------- helper functions per il main -----------------------
// ----------------------------------------------------------------------------

// attivazione del controllo manuale degli attuatori
void set_manual_ctrl() {
  if (status.manual_ctrl)
    return; // controllo manuale già attivo, nessuna azione eseguita

  // modifica del flag per il controllo manuale
  status.manual_ctrl = MANUAL_CTRL;

  // imposta il riscaldatore in base allo stato dello switch
  if (status.heat_sw && !status.heat_relay) {
    heat_turn_on(); // accensione riscaldatore e led associato
    serial_man_heat_on(); // stampa messaggio di accensione su serial monitor
  } else if (!status.heat_sw && status.heat_relay) {
    heat_turn_off(); // spegnimento riscaldatore e led associato
    serial_man_heat_off(); // stampa messaggio di spegnimento su serial monitor
  }

  // imposta l'umidificatore in base allo stato dello switch
  if (status.rh_sw && !status.rh_relay && !status.refill_led) {
    rh_turn_on(); // accensione umidificatore e led associato
    serial_man_rh_on(); // stampa messaggio di accensione su serial monitor
  } else if (!status.rh_sw && status.rh_relay) {
    rh_turn_off(); // spegnimento umidificatore e led associato
    serial_man_rh_off(); // stampa messaggio di spegnimento su serial monitor
  }
}

// attivazione del controllo automatico degli attuatori
void set_auto_ctrl() {
  if (!status.manual_ctrl)
    return; // controllo automatico già attivo, nessuna azione eseguita

  // modifica del flag per il controllo automatico
  status.manual_ctrl = AUTO_CTRL;

  // reset vecchi dati accumulati nel PID temperatura
  #if TEMP_CTRL == 2
  pid_reset_accumulators(temp_pid);
  #endif

  // reset vecchi dati accumulati nel PID umidità
  #if RH_CTRL == 2
  pid_reset_accumulators(rh_pid);
  #endif

  // reset timer pwm
  timers.start_pwm = millis() - PWM_PERIOD;
}

// verifica necessità di refill dell'acqua
bool check_refill() {

  /**
   * NOTE IMPLEMENTATIVE:
   *
   * Si verifica se la somma dei seguenti valori supera REFILL_INTERVAL:
   *
   * - durata delle precedenti accensioni dell'umidificatore data dalla
   *   variabile timers.refill_counter
   * - tempo trascorso dall'ultima accensione, data dalla differenza
   *   (millis() - timers.last_rh_on) moltiplicata per lo stato attuale
   *   dell'umidificatore (status.rh_relay), così da considerare solo
   *   il tempo trascorso se l'umidificatore è acceso
   */

  // verifica se è necessario un refill dell'acqua
  if (!status.refill_led && timers.refill_counter + (millis() - timers.last_rh_on) * status.rh_relay >= REFILL_INTERVAL) {
     rh_turn_off();                    // spegnimento umidificatore
     lcd_refill_message();
     serial_refill_message();
     digitalWrite(REFILL_LED, LED_ON); // accensione led di refill
     status.refill_led = true;         // aggiornamento stato led di refill
  }

  // restituisce lo stato del led di refill (true = refill, false = no refill)
  return status.refill_led;
}
