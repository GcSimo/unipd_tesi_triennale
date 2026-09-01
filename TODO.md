# TODO LIST - incubatrice neonatale

## firmware:
- [x] aggiungere gestione degli errori (temperatura, umidità fuori dai range)
- [x] aggiungere pid integral windup
- [x] verificare indipendenza parametri PID temperatura e umidità
- [x] ritardare impostazione setpoint temperatura e umidità
- [x] integrare derivative kick
- [x] creare classe per gestire sht20
- [ ] aggiungere conversione °C in potenza
- [ ] verificare massima durata di un loop
- [ ] verifica glitch iniziale dei relè e invertire digitalWrite con pinMode per i relè nella funzione di setup()
- [ ] verificare derivata al variare del setpoint (derivative kick test)
- [ ] verificare temporizzazione display, led allarme
- [ ] tarare parametri PID
- [ ] tarare timer di refill
- [ ] gestione output nel passaggio da automatico a manuale -> non necessaria\
- [ ] implementare il pid come oggetto -> non necessaria

## csv:
- [x] creare script matlab per interpretare comandi seriali

## documento tesi:
- [ ] scattare foto del prototipo da inserire nel documento
- [ ] scaricare template per presentazione powerpoint
- [ ] modificare titolo della tesi per eliminare controllo di umidità

## hardware:
- [x] correggere connessioni relay e switch
- [ ] bloccaggio del coperchio in plexiglass
- [ ] misurare distanze per supporto dell'arduino e della millefori
- [ ] verificare dimensione della millefori e componenti già presenti
- [ ] aggiungere switch di reset per il refill
- [ ] fare elenco del materiale da acquistare

## confronto con prof:
- [x] metodo dei minimi quadrati per calcolo derivata dell'errore
- [x] miglioramenti al controllore pid ([https://onlinelibrary.wiley.com/doi/10.1002/9781394442102.ch1](https://onlinelibrary.wiley.com/doi/10.1002/9781394442102.ch1), [http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-reset-windup/](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-reset-windup/))
- [ ] ordine dei materiali e gestione del relé
- [ ] scadenze per consegna tesi e discussione
- [ ] modificare titolo della tesi per eliminare controllo di umidità


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
