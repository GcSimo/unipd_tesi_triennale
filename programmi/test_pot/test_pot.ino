/**
 * @brief Test dei potenziometri del pannello frontale incubatrice neonatale
 * 
 * Il programma legge i valori dei potenziometri e li stampa sul monitor seriale ad intervalli di 0.2 secondo.
 */

// definizione dei pin dei potenziometri
#define TEMP_POT A0
#define RH_POT A1

void setup() {
  // inizializza la comunicazione seriale
  Serial.begin(9600);
  Serial.println("--- avvio test potenziometri ---");
}

void loop() {
  // legge i dati dai potenziometri
  int temp_value = analogRead(TEMP_POT);
  int rh_value = analogRead(RH_POT);

  // stampa i valori sul monitor seriale
  Serial.print("temp_value: ");
  Serial.print(temp_value);
  Serial.print(" | rh_value: ");
  Serial.println(rh_value);

  // attende 0.2 secondi prima della prossima lettura
  delay(200);
}
