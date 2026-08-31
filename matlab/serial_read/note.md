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
