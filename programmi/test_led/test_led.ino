/**
 * @brief Test dei led del pannello frontale incubatrice neonatale
 * 
 * Il programma accende in sequenza un led alla volta per 1 secondo.
 */

// definizione dei pin dei LED
#define ALARM_LED 2
#define HEAT_LED 3
#define RH_LED 4
#define REFILL_LED 5

void setup() {
  // inizializzazione dei pin dei LED come output
  pinMode(ALARM_LED, OUTPUT);
  pinMode(HEAT_LED, OUTPUT);
  pinMode(RH_LED, OUTPUT);
  pinMode(REFILL_LED, OUTPUT);

  // assegnazione stato iniziale dei LED
  digitalWrite(ALARM_LED, LOW);
  digitalWrite(HEAT_LED, LOW);
  digitalWrite(RH_LED, LOW);
  digitalWrite(REFILL_LED, LOW);
}

void loop() {
  // accensione del led di allarme per 1 secondo
  digitalWrite(ALARM_LED, HIGH);
  delay(1000);
  digitalWrite(ALARM_LED, LOW);

  // accensione del led di riscaldamento per 1 secondo
  digitalWrite(HEAT_LED, HIGH);
  delay(1000);
  digitalWrite(HEAT_LED, LOW);

  // accensione del led di umidità relativa per 1 secondo
  digitalWrite(RH_LED, HIGH);
  delay(1000);
  digitalWrite(RH_LED, LOW);

  // accensione del led di riempimento per 1 secondo
  digitalWrite(REFILL_LED, HIGH);
  delay(1000);
  digitalWrite(REFILL_LED, LOW);
}
