#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// crea un'istanza dell'oggetto per il display LCD con indirizzo I2C 0x27 e dimensioni 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  // inizializza il display
  lcd.init();

  // accende la retroilluminazione del display
  lcd.backlight();

  // imposta il cursore e stampa i messaggi sul display
  lcd.setCursor(2, 0);
  lcd.print("Test display I2C");
  lcd.setCursor(2, 1);
  lcd.print("Arduino LCD 2004");
  lcd.setCursor(0, 2);
  lcd.print("Incubatrice neonatale");
  lcd.setCursor(3, 3);
  lcd.print("DEI - UNIPD");
}

void loop() {
  // intentionally left blank
}
