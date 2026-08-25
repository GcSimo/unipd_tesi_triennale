/**
 * @brief Test switch del pannello frontale incubatrice neonatale
 *
 * Legge il valore dei tre switch e stampa sul monitor seriale eventuali cambiamenti di stato.
 *
 */

// definizione dei pin degli switch
#define HEAT_SWITCH 6
#define RH_SWITCH 7
#define LIGHT_SWITCH 8

// definizione delle variabili per memorizzare lo stato precedente degli switch
bool heat_switch_state = LOW;
bool rh_switch_state = LOW;
bool light_switch_state = LOW;

void setup() {
  // inizializzazione dei pin degli switch come input
  pinMode(HEAT_SWITCH, INPUT);
  pinMode(RH_SWITCH, INPUT);
  pinMode(LIGHT_SWITCH, INPUT);

  // inizializza la comunicazione seriale
  Serial.begin(9600);
  Serial.println("--- avvio test switch ---");

  // lettura iniziale degli switch
  heat_switch_state = digitalRead(HEAT_SWITCH);
  rh_switch_state = digitalRead(RH_SWITCH);
  light_switch_state = digitalRead(LIGHT_SWITCH);

  // stampa dello stato iniziale degli switch
  Serial.print("heat: ");
  Serial.print(heat_switch_state);
  Serial.print(" | rh: ");
  Serial.print(rh_switch_state);
  Serial.print(" | light: ");
  Serial.println(light_switch_state);
}

void loop() {
  // legge lo stato dello switch di riscaldamento
  if (digitalRead(HEAT_SWITCH) != heat_switch_state) {
	heat_switch_state = !heat_switch_state;
	Serial.print("heat changed: ");
	Serial.println(heat_switch_state);
  }

  // legge lo stato dello switch di umidità relativa
  if (digitalRead(RH_SWITCH) != rh_switch_state) {
	rh_switch_state = !rh_switch_state;
	Serial.print("rh changed: ");
	Serial.println(rh_switch_state);
  }

  // legge lo stato dello switch della luce
  if (digitalRead(LIGHT_SWITCH) != light_switch_state) {
	light_switch_state = !light_switch_state;
	Serial.print("light changed: ");
	Serial.println(light_switch_state);
  }

  // attende 0.2 secondi prima della prossima lettura
  delay(200);
}
