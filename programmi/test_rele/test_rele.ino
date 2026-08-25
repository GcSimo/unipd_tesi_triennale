/**
 * @brief Test dei led del pannello frontale incubatrice neonatale
 *
 * Il programma accende in sequenza un led alla volta per 1 secondo.
 */

// definizione dei pin dei LED
#define HEAT_RELAY 9
#define RH_RELAY 10
#define FAN_RELAY 11
#define LIGHT_RELAY 12

void setup() {
  // inizializzazione dei pin dei LED come output
  pinMode(HEAT_RELAY, OUTPUT);
  pinMode(RH_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LIGHT_RELAY, OUTPUT);

  // assegnazione stato iniziale dei LED
  digitalWrite(HEAT_RELAY, HIGH);
  digitalWrite(RH_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(LIGHT_RELAY, HIGH);
}

void loop() {
  // accensione del led di allarme per 1 secondo
  digitalWrite(HEAT_RELAY, LOW);
  delay(1000);
  digitalWrite(HEAT_RELAY, HIGH);

  // accensione del led di riscaldamento per 1 secondo
  digitalWrite(RH_RELAY, LOW);
  delay(1000);
  digitalWrite(RH_RELAY, HIGH);

  // accensione del led di umidità relativa per 1 secondo
  digitalWrite(FAN_RELAY, LOW);
  delay(1000);
  digitalWrite(FAN_RELAY, HIGH);

  // accensione del led di riempimento per 1 secondo
  digitalWrite(LIGHT_RELAY, LOW);
  delay(1000);
  digitalWrite(LIGHT_RELAY, HIGH);
}
