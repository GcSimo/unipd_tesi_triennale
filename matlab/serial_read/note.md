# note sui test effettuati

## setup
- sensore a 20cm dalle pareti laterali, 17cm dalla parete dove passano i cavi e 7.5cm dal fondo della camera
- codice serial_read_2.m


## set prove 1
### data_20260827_161338.csv
- kp = 10, ki = 0, kd = 0
- raggiunge sui 30.5°C circa e poi inizia una lenta salita

### data_20260827_163923.csv
- kp = 20, ki = 0, kd = 0
- esce dalla saturazione sui 31°C
- raggiunge sui 33°C circa e poi inizia una lenta salita

### data_20260827_170304.csv
- kp = 50, ki = 0, kd = 0
- esce dalla saturazione sui 34°C
- raggiunge sui 34.70 °C e poi inizia una lenta salita

### data_20260827_171303.csv
- continua il test precedente con kp aumentato
- kp = 100, ki = 0, kd = 0
- esce dalla saturazione sui 35°C
- raggiunge sui 35.4°C e poi sembra rimanere stabile

### data_20260827_171817.csv
- continua il test precedente con l'aggiunta di ki
- kp = 100, ki = 10, kd = 0, back calculation con kw = 0
- cresce fino ai 36.70°C per kw = 0 (back calculation come integral anti-windup)

### data_20260827_172830.csv
- kp = 100, ki = 5, kd = 0, integral clamping
- ha leggera sovraelongazione fino a 36.15, scende fino a 35.9°C, torna stabile sui 36°C circa
- viene aperto lo sportellone delle mani a 700 secondi circa
- viene richiuso a 1100+ circa
- buona la stabilità a portello aperto, ma ha oscillazioni troppo veloci a portello chiuso

### data_20260827_175414.csv
- kp = 75, ki = 10, kd = 0, integral clamping
- arriva a regime, ma con oscillazioni

### data_20260827_180306.csv
- kp = 75, ki = 5, kd = 2, integral clamping, 8 buffer error size
- continua ad avere oscillazioni, il derivativo non funziona (è sempre 0)

### data_20260827_182254.csv
- kp = 50, ki = 2, kd = 2, integral clamping, 8 buffer error size
- continua ad avere oscillazioni più lente

### data_20260827_183121.csv
- kp = 50, ki = 0.5, kd = 20, integral clamping, 8 buffer error size
- chiusura culla ritardata al secondo 55


## set prove 2
### data_20260831_110724.csv
- test evoluzione forzata con riscaldatore al massimo

### data_20260831_122037.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 6.40A
- tempo da 35°C a 37°C = 178 secondi
- 6.40A * 12V * 178s / 2°C = 6835,2 J/°C

### data_20260831_130256.csv
- raggiunta equilibrio iniziale per avvio test

### data_20260831_132059.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 6.75A
- tempo da 35°C a 37°C = 173 secondi
- 6.75A * 12V * 173s / 2°C = 7006,5 J/°C

### data_20260831_134425.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 6.67A
- tempo da 35°C a 37°C = 175 secondi
- 6.67A * 12V * 175s / 2°C = 7003,5 J/°C

### data_20260831_144643.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 6.10A
- tempo da 35°C a 37°C = 215 secondi
- 6.10A * 12V * 215s / 2°C = 7869 J/°C


## set prove 3
### data_20260901_135714.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 5.90A
- tempo da 35°C a 37°C = 301 secondi
- 5.90A * 12V * 301s / 2°C = 10655 J/°C

### data_20260901_142034.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 6.34A
- tempo da 35°C a 37°C = 213 secondi
- 6.34A * 12V * 213s / 2°C = 8102 J/°C

### data_20260901_143416.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 6.17A
- tempo da 35°C a 37°C = 158 secondi
- 6.17A * 12V * 158s / 2°C = 5849 J/°C

### data_20260901_144539.csv
- test evoluzione forzata con riscaldatore al massimo
- consumo medio del riscaldatore 5.98A
- tempo da 35°C a 37°C = 166 secondi
- 5.98A * 12V * 166s / 2°C = 5956 J/°C

### data_20260901_145820.csv
- test evoluzione forzata con riscaldatore al massimo (ed evoluzione libera)
- consumo medio del riscaldatore 6.00A
- tempo da 35°C a 37°C = 153 secondi
- 6.00A * 12V * 153s / 2°C = 5508 J/°C

### data_20260901_155236.csv
- test evoluzione forzata con riscaldatore al massimo con continui saliscendi tra 34°C e 38°C
- consumo medio del riscaldatore ---
- tempo da 35°C a 37°C:
  - 90 secondi
  - 65 secondi
  - 61 secondi
  - 59 secondi
  - 59 secondi

- si svolgono le seguenti operazioni:
  - Potenza_erogata * Delta_tempo -> Energia_erogata
  - Energia_erogata / Delta_Temperatura -> Energia/°C
  - Energia/°C / Periodo_PWM -> Potenza_PWM/°C
  - Potenza_PWM/°C / Potenza_media_erogata * 100-> %_duty_cycle/°C
  - Potenza_erogata * Delta_tempo / Delta_Temperatura / Periodo_PWM / Potenza_media_erogata *100 = Delta_tempo / Delta_Temperatura / Periodo_PWM * 100 = %_duty_cycle/°C

- assumiamo tempo medio di 60 secondi quando l'incubatrice si trova a regime termico tra 34°C e 38°C
- 60 sec / 2°C / 8 sec * 100 = 375 %/°C


## set prove 4
### data_20260901_163102.csv
- test pid con kp = 375, ki = 0, kd = 0
- valore proporzionale troppo grande (mancava un /100 per come sono gestite le temperature)

### data_20260901_164038.csv
- test pid con kp = 3.75, ki = 0, kd = 0
- rimane sempre troppo aggressivo, forse perché c'è un ritardo tra l'attuatore e il sensore dovuto all'inerzia termica del sistema

### data_20260901_165006.csv
- test pid con kp = 1, ki = 0, kd = 0
- partenza da 36°C
- accensione manuale da 300 a 320 secondi
- nota: pwm a 40-45% mantiene i 35.55°C con setpoint 36°C
- spegnimento manuale e apertura da 630 a 700 secondi
- risale con sovraelongazione a 36.02°C, scende a 35.35°C e si stabiizza a 35.6°C con pwm a 40°C

### data_20260901_171003.csv
- test pid con kp = 0.7, ki = 0, kd = 0
- partenza da 32°C, raggiunge in sovraelongazione i 35.7°C, si stabilizza a 35.4°C con pwm a 40%

### data_20260901_171707.csv
- test pid con kp = 0.5, ki = 0, kd = 0
- partenza da 32°C, raggiunge in sovraelongazione i 35.5°C, si stabilizza a 35.15°C con pwm a 42%

### data_20260901_172544.csv
- test pid con kp = 0.3, ki = 0, kd = 0
- partenza da 31°C, raggiunge in sovraelongazione i 35°C, si stabilizza a 34.7°C con pwm a 38%

### data_20260901_173825.csv
- test pid con kp = 0.5, ki = 0.01, kd = 0
- partenza da vecchio regime (34.7°C)
- raggiunge in sovraelongazione i 36.4°C, scende a 35.8°C, sale a 36.1°C, si stabilizza a 36.02°C con pwm a 45%
- spegnimento manuale e apertura da 550 a 700 secondi
- raggiunge in sovraelongazione i 36.4°C, scende a 35.7°C, sale a 36.1°C, fa un po' di oscillazioni

### data_20260901_180225.csv
- test pid con kp = 0.5, ki = 0.005, kd = 10 - contributo derivativo invertito
- buono, nota: il contributo derivativo mostrato agisce al contrario (viene sottratto)
- apertura sportello da 550
- ritorna a regime senza sovraelongazioni
- chiusura sportello a 900
- a 1300 lieve abbassamento della temperatura esterna percepita da me

### data_20260901_181124.csv
- test pid con kp = 0.5, ki = 0.01, kd = 10 - raddrizzamento contributo derivativo
- sovraelongazione fino a 36.5°C, scende a 35.8°C e si stabilizza

### data_20260901_184203.csv
- test pid con kp = 0.5, ki = 0.01, kd = 20
- sovraelongazione fino a 36.5°C, scende a 35.8°C e continua con lente e leggere oscillazioni


## set prove 5

### data_20260902_131756.csv
- test pid con ...
- ...

### data_20260902_132207.csv
- test pid con ...
- ...

### data_20260902_133434.csv
- test pid con ...
- ...

### data_20260902_152137.csv
- test pid con kp = 50, ki = 0.5, kd = 1000
- cambio scalatura parametri * 100
- si ha una sovraelongazione a 36.2°C con lento ritorno al setpoint

### data_20260902_155002.csv
- test pid con kp = 50, ki = 0.5, kd = 500
- buono, leggero overshoot a 36.1°C per integrale troppo alto

### data_20260902_161512.csv
- test pid con kp = 50, ki = 0.5, kd = 500, 16 secondi di pwm e 16 campioni derivativi
- lento raggiungimento del setpoint per problemi integrativi, buono il pwm cycle

### data_20260902_163605.csv
- test pid con kp = 50, ki = 0.75, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- overshoot per integrale troppo alto e oscillazioni a regime
- buono il pwm cycle

### data_20260902_164735.csv
- test pid con kp = 75, ki = 0.75, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- overshoot per integrale troppo alto, sottoelongazione e poi stabilità
- buono il pwm cycle

### data_20260902_170301.csv
- test pid con kp = 50, ki = 0.5, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- no overshoot, ma integrativo troppo bassa

### data_20260902_172233.csv
- test pid con kp = 50, ki = 0.6, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- leggero overshoot

### data_20260902_173923.csv
- test pid con kp = 50, ki = 0.6, kd = 300, 15 secondi di pwm e 20 campioni derivativi
- arriva a regime, poi cala e ritorna a salire per integrale troppo lento

### data_20260902_175843.csv
- test pid con kp = 50, ki = 0.6, kd = 750, 15 secondi di pwm e 20 campioni derivativi
- arriva con overshoot

### data_20260902_181144.csv
- test pid con kp = 35, ki = 0.6, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- sovraelongazione con oscllazioni a regime

### data_20260902_181958.csv
- test pid con kp = 75, ki = 0.6, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- arriva a regime ma l'integrale è troppo basso per cui si ha sottoelongazione e poi lenta salita al setpoint

### data_20260902_183525.csv
- test pid con kp = 30, ki = 0.3, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- integrale troppo grande

### data_20260902_184328.csv
- test pid con kp = 40, ki = 0.2, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- integrale troppo basso

### data_20260902_184844.csv
- test pid con kp = 40, ki = 0.3, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- integrale troppo basso

### data_20260902_185452.csv
- test pid con kp = 40, ki = 0.45, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- integrale di poco troppo grande

### data_20260903_123734.csv
- test pid con kp = 40, ki = 0.4, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- primo test della giornata -> partenza da freddo
- con sovraelongazione

### data_20260903_125616.csv
- test pid con kp = 40, ki = 0.4, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- stessa configurazione, ma partenza dopo raffreddamento parziale
- nessuna sovraelongazione

### data_20260903_130221.csv
- test pid con kp = 40, ki = 0.4, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- stessa configurazione, ma con oblò aperti
- con leggera sovraelongazione

### data_20260903_140800.csv
- test pid con kp = 50, ki = 0.55, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- partenza da freddo
- con sovraelongazione

### data_20260903_142528.csv
- test pid con kp = 50, ki = 0.55, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- stessa configurazione, ma partenza dopo raffreddamento parziale
- appena con sovraelongazione

### data_20260903_143757.csv
- test pid con kp = 50, ki = 0.55, kd = 500, 15 secondi di pwm e 20 campioni derivativi
- stessa configurazione, ma con oblò aperti
- leggera sovraelongazione

### data_20260903_152539.csv
- test isteresi temperatura - 1

### data_20260903_154139.csv
- test isteresi temperatura - 2
- buono per tesi, risposta allo scalino, con condizione iniziale di 31 °C

### data_20260903_163545.csv
- test pid con kp = 40, ki = 0.4, kd = 500, clamping, 15 secondi di pwm e 20 campioni derivativi
- buono per tesi, risposta allo scalino, con condizione iniziale di 31 °C

### data_20260903_164924.csv
- test pid con kp = 40, ki = 0.4, kd = 500, bias = 50, limiting [-30, +30], 15 secondi di pwm e 20 campioni derivativi
- troppo overshoot perché l'integrale arriva subito a saturazione +30
- a regime funziona solo l'azione di feedforward

### data_20260903_170156.csv
- test pid con kp = 40, ki = 0.4, kd = 500, bias = 50, clamping, 15 secondi di pwm e 20 campioni derivativi
- troppo overshoot perché l'integrale cresce fino a 17/18
- leggere oscillazioni prima di stabilizzarsi
- a regime funziona solo l'azione di feedforward

## set prove 6
### data_20260904_125808.csv
- test cambio setpoint temperatura

### data_20260904_131119.csv
- test evoluzione libera, alla fine temperatura esterna di 25.8°C
- polo e costante per setpoint a:
  - 25°C -> -0.0010 - 12.5703
  - 26°C -> -0.0011 - 11.4769
  - 27°C -> -0.0013 - 10.2818
  - 28°C -> -0.0014 - 8.8870
  - 29°C -> -0.0014 - 7.0738
  - 30°C -> -0.0011 - 4.4205

### data_20260904_135951.csv
- test evoluzione libera, alla fine temperatura esterna di 25.7 °C
- polo e costante per setpoint a:
  - 25°C -> -0.0012 - 14.7070
  - 26°C -> -0.0013 - 13.6616
  - 27°C -> -0.0015 - 12.5631
  - 28°C -> -0.0017 - 11.3731
  - 29°C -> -0.0019 - 10.0085
  - 30°C -> -0.0020 - 8.2765

### data_20260904_145036.csv
- test pid con kp = 45, ki = 0.4, kd = 500, no bias, clamping, 15 secondi di pwm e 20 campioni derivativi
- reset arduino e inizio test al secondo 53.-
- leggera sovraelongazione

### data_20260904_150913.csv
- test pid con kp = 50, ki = 0.4, kd = 500, no bias, clamping, 15 secondi di pwm e 20 campioni derivativi
- undershoot

### data_20260904_152754.csv
- test pid con kp = 60, ki = 0.4, kd = 500, no bias, clamping, 15 secondi di pwm e 20 campioni derivativi
- undershoot

### data_20260904_153124.csv
- test pid con kp = 60, ki = 0.4, kd = 100, no bias, clamping, 15 secondi di pwm e 20 campioni derivativi
- undershoot

### data_20260904_155040.csv
- test pid con kp = 70, ki = 0.4, kd = 0, no bias, clamping, 15 secondi di pwm e 20 campioni derivativi
- undershoot

### data_20260904_155746.csv
- test pid con kp = 70, ki = 0.7, kd = 0, no bias, clamping, 15 secondi di pwm e 20 campioni derivativi
- undershoot leggero

### data_20260904_160527.csv
- test pid con kp = 70, ki = 0.8, kd = 0, no bias, clamping, 15 secondi di pwm e 20 campioni derivativi
- undershoot leggero e sovraelongazione
