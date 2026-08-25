/**
 * @brief Test sensore SHT20 incubatrice neonatale
 * 
 * Legge i valori di temperatura e umidità dal sensore SHT20 e li stampa sul monitor seriale ad intervalli di 1 secondo
 */

#include <Wire.h>
#include "SHT2x.h"

// Crea l'oggetto per il sensore
SHT2x sht;

void setup() {
  // inizializzazione la comunicazione seriale
  Serial.begin(9600);
  delay(1000);
  Serial.println("Inizializzazione SHT20...");

  // inizializza il sensore SHT20
  Wire.begin();
  sht.begin(); 
  delay(100);
  
  // controlla connessione del sensore per verificare la connessione
  if (!sht.isConnected()) {
	Serial.println("Errore: sensore SHT20 non connesso!");
	while (1); // blocca l'esecuzione se il sensore non è connesso
  }

  // stampa di log
  Serial.println("Sensore pronto!");
  Serial.println("---------------------------------");

  // stampa le informazioni sul sensore
  Serial.print("versione firmware: ");
  Serial.println(sht.getFirmwareVersion());
  Serial.print("risoluzione: ");
  Serial.println(sht.getResolution(), HEX);
  Serial.print("status byte: ");
  Serial.println(sht.getStatus(), HEX);
  Serial.print("battery status: ");
  Serial.println(sht.batteryOK() ? "OK" : "LOW");
  Serial.print("errors: ");
  Serial.println(sht.getError(), HEX);
  Serial.println("---------------------------------");
}

void loop() {
	// richiede al sensore di leggere i valori di temperatura e umidità
	sht.read();

	// stampa i valori convertiti sul monitor seriale
	Serial.print("temp: ");
	Serial.print(sht.getTemperature());
	Serial.print(" °C");

	Serial.print(" | rh: ");
	Serial.print(sht.getHumidity());
	Serial.println(" %");

	// stampa i valori grezzi sul monitor seriale
	Serial.print("raw temp: ");
	Serial.print(sht.getRawTemperature());
	Serial.print(" | raw rh: ");
	Serial.println(sht.getRawHumidity());

	// attende 1 secondo prima della prossima lettura
	delay(1000);
}
