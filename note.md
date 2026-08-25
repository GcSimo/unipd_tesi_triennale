# Tesi

## formule utili
### dipendenza tra temperatura e umidità relativa
umidità relativa dello stato 2 (RH2) in funzione della temperatura dello stato 1 (T1), della temperatura dello stato 2 (T2) e dell'umidità relativa dello stato 1 (RH1):
```
RH2 = RH1 * exp(4283.78 * (T1-T2) / (243.12 + T1) / (243.12 + T2))
```

---

## dati per collegamento
### deviatori SPDT
- non serve la resistenza di pull-up o pull-down
- pin centrale collegato ad un digital input pin del microcontrollore
- pin laterali collegati a VCC e GND

### potenziometro (valori da confermare)
- resistenza: 10k ohm
- pin centrale collegato ad un analog input pin del microcontrollore
- pin laterali collegati a VCC e GND

### LED rosso (valori da confermare)
- anodo collegato a pin digitale del microcontrollore, catodo collegato a GND tramite resistenza di bias
- forward voltage: 1.8V
- forward current: 15mA
- bias resistor: 470 ohm
```
I = (Vcc - Vf) / Rb = (5.0V - 1.8V) / 470 ohm = 6.8 mA
```

### LED verde (valori da confermare)
- anodo collegato a pin digitale del microcontrollore, catodo collegato a GND tramite resistenza di bias
- forward voltage: 2.0V
- forward current: 15mA
- bias resistor: 220 ohm
```
I = (Vcc - Vf) / Rb = (5.0V - 2.0V) / 220 ohm = 13.6 mA
```

### Step up - mp1584
- maximum current: 3A

---

## Arduino Mega 2560
### limiti di corrente
- 20mA corrente massima per I/O pin
- 50mA corrente massima per 3.3V pin
- 500mA limite di Vcc per alimentazione via USB (limitato dal fusibile MF-MSMF050-2)
- 800mA limite di Vcc per alimentazione via jack barrel (limitato dal voltage regulator LD1117)

---

## Monitor
### interfaccia I2C
- indirizzo I2C a 7 bit, 0x27, i 3 LSbit sono selezionabili tramite i pin A0, A1, A2 (se sono aperti -> 0)
- il ponticello LED abilita la retroilluminazione del display, il potenziometro regola il contrasto del display

### libreria LiquidCrystal_I2C - [https://github.com/johnrickman/LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)

```Arduino
// dependencies
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// create a new instance of an LCD display (address 0x27, 20 characters and 4 lines)
LiquidCrystal_I2C lcd(0x27,20,4);

// initialize the LCD display
void init();

// set cursor to specific position
void setCursor(column, row);

// set cursor to home position (0,0)
void home();

// print text on the display
void print("Hello, world!");

// clear the display
void clear();

// turn on the backlight
void backlight();

// turn off the backlight
void noBacklight();

// turn on the display
void display();

// turn off the display
void noDisplay();

// write a custom character saved at a specific index to the display
size_t write(index);

// create a custom character at a specific index defined as a charmap made of an array of 8 bytes (5x8 pixels)
void createChar(index, charmap);
```

---

## Sensore SHT20
### comandi I2C
- ``1000 000``: indirizzo I2C a 7 bit del sensore
- ``1110 0011``: comando per lettura temperatura (hold master)
- ``1110 0101``: comando per lettura umidità (hold master)
- ``1111 0011``: comando per lettura temperatura (no hold master)
- ``1111 0101``: comando per lettura umidità (no hold master)
- ``1110 0110``: scrittura sullo user register
- ``1110 0111``: lettura dello user register
- ``1111 1110``: soft reset del sensore

### modalità di lavoro
- Hold master: il sensore tiene la linea SCL bassa durante la misurazione, impedendo altre comunicazioni I2C sullo stesso bus.
- No hold master: il sensore lascia la linea SCL libera durante la misurazione, permettendo altre comunicazioni I2C sullo stesso bus. Serve aspettare 18.5 micros tra il comando NACK/ACK e l'invio del comando di stop. Il controllore effettua polling per verificare quando il dato è pronto, ovvero manda richieste di lettura fino a quando il sensore non risponde con un ACK.

### parsing della risposta
La risposta è composta da tre byte (ricevuti in big endian order):
- byte 1: byte più significativo
- byte 2: 6bit meno significativo + 2bit di status
- byte 3: checksum CRC8

Conversione valori binari in valori fisici:
```
RH = -6 + 125 * (valore binario / 2^16)
T = -46.85 + 175.72 * (valore binario / 2^16)
```

### user register e configurazioni
Lo user register di 1 byte è composto nel seguente modo (trasmesso in big endian order):
- bit 7,0: risoluzione di misura (RH, T)
- bit 6: status di batteria (0 = batteria ok, 1 = batteria scarica)
- bit 5,4,3: riservato (mantenere gli stessi valori di default)
- bit 2: on-chip heater (0 = off, 1 = on)
- bit 1: disabilita OTP reload (0 = abilitato, 1 = disabilitato)

Di seguito la tabella con le risoluzioni e i tempi di conversione con i corrispettivi bit 7,0 dello user register:
| bit 7,0  | RH     | T      | RH typ wait | RH max wait | T typ wait | T max wait |
| -------- | ------ | ------ | ----------- | ----------- | ---------- | ---------- |
| 00 (def) | 12 bit | 14 bit | 22 ms       | 29 ms       | 66 ms      | 85 ms      |
| 01       | 8 bit  | 12 bit | 3 ms        | 4 ms        | 17 ms      | 22 ms      |
| 10       | 10 bit | 13 bit | 7 ms        | 9 ms        | 33 ms      | 43 ms      |
| 11       | 11 bit | 11 bit | 12 ms       | 15 ms       | 9 ms       | 11 ms      |

### libreria SHT2x di  Rob Tillaart - [https://github.com/RobTillaart/SHT2x](https://github.com/RobTillaart/SHT2x)

```Arduino
// dependencies
#include "Wire.h"
#include "SHT2x.h"

// create a new instance of the SHT2x sensor
SHT2x sht;

// initialize the SHT2x sensor
bool begin();

// read the temperature and humidity values from the SHT2x sensor (blocking function)
bool read();

// convert the temperature [or humidity] value previously read in Celsius [or percentage] from the SHT2x sensor
float getTemperature();
float getHumidity();

// return the temperature [or humidity] value previously read in raw format from the SHT2x sensor
uint16_t getRawTemperature();
uint16_t getRawHumidity();

// request a new temperature [or humidity] measurement from the SHT2x sensor (non-blocking function)
bool requestTemperature();
bool requestHumidity();

// check if the temperature [or humidity] measurement is ready from the SHT2x sensor (non-blocking function)
bool reqTempReady();
bool reqHumReady()
bool requestReady();

// read the temperature [or humidity] value from the SHT2x sensor (non-blocking function)
bool readTemperature();
bool readHumidity();

// read the status byte of the SHT2x sensor
uint8_t getStatus();

// check if the SHT2x sensor is connected
bool isConnected();

// perform a soft reset of the SHT2x sensor
bool reset();

// get the error code of the SHT2x sensor in case an error occurs
int getError();

// check battery status of the SHT2x sensor (always followed by a getError() call)
bool batteryOK();

// manage sensor resolution of the SHT2x sensor (res = {0,1,2,3})
bool setResolution(res);
uint8_t getResolution();
```
