# TODO LIST - incubatrice neonatale

## firmware:
- [x] aggiungere gestione degli errori (temperatura, umidità fuori dai range)
- [x] aggiungere pid integral windup
- [x] verificare indipendenza parametri PID temperatura e umidità
- [x] ritardare impostazione setpoint temperatura e umidità
- [x] integrare derivative kick
- [x] creare classe per gestire sht20
- [x] verifica glitch iniziale dei relè e invertire digitalWrite con pinMode per i relè nella funzione di setup()
- [x] tarare timer di refill
- [x] verificare massima durata di un loop - max ct: 248312 micros - mean ct: 166 micros (PID-PID) - 162 micros (PID-HYST) - 156 micros (MAN) - 153 (NONE-NONE)
- [x] verificare temporizzazione display, led allarme
- [x] verificare derivata al variare del setpoint (derivative kick test)
- [ ] tarare parametri PID
- [ ] aggiungere static_assert per verificare parametri di configurazione dell'incubatrice
- [ ] aggiungere conversione °C in potenza (feedforward per ragiungere il setpoint) -> complesso da implementare
- [ ] gestione output nel passaggio da automatico a manuale -> non necessaria
- [ ] implementare il pid come oggetto -> non necessaria (peggiorativa)

## csv:
- [x] creare script matlab per interpretare comandi seriali

## documento tesi:
- [x] scaricare template per presentazione powerpoint
- [x] scattare foto del prototipo da inserire nel documento:
  - [x] foto solo riscaldatore montato con ventola dietro
  - [x] foto solo ventola di omogeneizzazione
  - [x] foto solo scheda arduino
  - [x] foto solo stepdown in uso accanto ad uno nuovo
  - [x] foto interno del pannello frontale (lato pannello e lato incubatrice)
  - [x] misurare corrente assorbita dallo step down -> 1: 35 mA / 2: 66mA / 3: 96mA / 4: 117 mA / 5: 139 mA
- [ ] modificare titolo della tesi per eliminare controllo di umidità

## hardware:
- [x] correggere connessioni relay e switch
- [x] misurare distanze per supporto dell'arduino e della millefori
- [x] verificare dimensione della millefori e componenti già presenti
- [ ] bloccaggio del coperchio in plexiglass
- [ ] aggiungere switch di reset per il refill
- [ ] fare elenco del materiale da acquistare

## confronto con prof:
- [x] metodo dei minimi quadrati per calcolo derivata dell'errore
- [x] miglioramenti al controllore pid ([https://onlinelibrary.wiley.com/doi/10.1002/9781394442102.ch1](https://onlinelibrary.wiley.com/doi/10.1002/9781394442102.ch1), [http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-reset-windup/](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-reset-windup/))
- [x] ordine dei materiali e gestione del relé
- [x] scadenze per consegna tesi e discussione
- [x] modificare titolo della tesi per eliminare controllo di umidità


- discussione dal 7 al 12 settembre verso metà settimana
- scadenza per caricare è il giorno prima della discussione
- modifica titolo: ok
- presentare contesto, obiettivi e procedimenti pratici estremamente rapidamente in funzione degli obiettivi, risultati ed evoluzioni future
- va bene metodo dei minimi quadrati
- magari potrebbe essere che il pid viene aggiornato con intervalli più ampi rispetto al campionamento dei sensori
- testare quanti W vengono convertiti in quanti gradi a catena aperta
  - trovare lo stato di equilibrio (temperatura raggiunta) a determinate situazioni di duty cycle
  - costruire 3 punti per 3 valori di duty cycle e fare una curva di interpolazione
  - una volta costruita la curva si può appplicare la funzione inversa per trovare il duty cycle necessario per raggiungere una certa temperatura
  - in questo modo si può fare un controllo feedforward
  - si somma il contributo del pid al contributo del feedforward così il feedback agisce solo per correggere eventuali errori di modellazione
- lista materiale dopo la discussione
