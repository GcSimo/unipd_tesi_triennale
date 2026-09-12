# note presentazione tesi

## slide 2 - contesto iniziale
- le incubatrici neonatali servono per garantire un ambiente stabile, controllato ed idoneo per i neonati prematuri o con peso corporeo ridotto
- i neonati prematuri o con peso corporeo ridotto non hanno un sistema di termoregolazione in grado di mantenere la temperatura corporea stabile
- se la temperatura corporea scende al di sotto dei 36.5°C, il neonato entra in ipotermia che può provocare complicazioni gravi come infezioni sistemiche, complicanze polmonari, ipossia di organi e tessuti, fino alla morte
- la temperatura critica (temperatura al di sotto della quale il corpo non è più in grado di mantenere la temperatura corporea stabile) per un adulto è intorno agli 0°C, mentre per un neonato prematuro è intorno ai 20-23°C
- data l'elevata temperatura critica, l'ipotermia neonatale è un problema globale anche nei paesi tropicali
- il prototipo di incubatrice neonatale sviluppato da Bincoletto Vanni si propone di essere un primo passo verso una soluzione economica e conforme alle normative da impiegare nei paesi in via di sviluppo

- l'incubatrice è composta da pareti in polistirene espanso tenute insieme da una struttura in legno, dispone di un riscaldatore, un umidificatore, un sensore di temperatura ed umidità ed un pannello frontale per permettere all'utente di interfacciarsi con il sistema

## slide 3 - Fasi di sviluppo ed opportunità
- nella prima fase è stato progettato il circuito elettronico su fritzing, sono state fatte le opportune saldature ed è stato implementato su breadboard
- nella seconda fase è stato sviluppato il firmware per gestire le varie periferiche, i sistemi di controllo e le interazioni con l'utente
- nella terza fase sono stati testati i sistemi di controllo implementati, dopo aver tarato gli opportuni parametri

- per fare ciò è stato necessario mettere in campo competenze di settori diversi (elettronica, informatica, termodinamica, teoria dei controlli) per portare a termine tutte le fasi dello sviluppo
- sono stati affrontati numerosi problemi tipici delle implementazioni pratiche, non sempre affrontati a lezione
- è stato approfondito il funzionamento dei sistemi di controllo, in particolare l'azione coordinata dei tre componenti del controllore PID in funzione di varie configurazioni di sistema

## slide 4 - Progettazione e realizzazione dell'hardware
- si può apprezzare a sinistra il circuito elettronico realizzato nel cassetto inferiore con le connessioni tra le diverse periferiche e la scheda di controllo, mentre a destra si possono vedere le saldature effettuate tra i componenti del pannello frontale
- si nota in basso a sinistra il morsetto per il collegamento di tutti i cavi di massa dei vari componenti, approccio utilizzato per evitare ground lifting che inizialmente avevano dato problemi di funzionamento al sistema (una delle non idealità del mondo reale e che non viene trattata a lezione)

## slide 5 - sviluppo del firmware
- il firmware ha numerosi compiti da svolgere in maniera concorrente, alcuni hanno vincoli stringenti sul timing di esecuzione, come il controllo del pwm o la lettura del sensore
- si descrivono i vari compiti del firmware:
  - monitoraggio degli switch e dei potenziometri e acquisizione dei dati dal sensore SHT20;
  - stampa dei messaggi su display lcd e sul monitor seriale;
  - calcolo dell’output dei controllori (isteresi o PID) e controllo degli attuatori tramite un’eventuale segnale PWM.
- si necessita inoltre di produrre un codice robusto, altamente efficiente e facilmente manutenibile ed aggiornabile
- il codice è stato suddiviso in moduli, il più possibile indipendenti tra loro per migliorare l'organizzazione del codice e permettere un facile aggiornamento delle singole parti, inoltre l'ampio uso di commenti permette una facile comprensione
- per sincronizzare i vari compiti è stata utilizzata la funzione millis() e non è presente alcuna forma di codice bloccante come delay() o cicli che restano bloccati ad aspettare l'esito di determinate condizioni

## slide 6 - controllore ad isteresi
- il controllore ad isteresi permette un controllo estremamente semplice da implementare ed offre una buona risposta
- soddisfa le richieste della normativa IEC 60601-2-19 (overshoot < 2°C, tempo di assestamento < 15 minuti e variazione della temperatura < 0.5°C in 1 ora di regime stazionario)
- non offre, però, un controllo stabile ed efficace della temperatura

## slide 7 - controllore PID
- molto più complesso da implementare, in quanto richiede alcuni accorgimenti pratici come
  - la gestione del windup dell’integrale
  - il filtraggio del rumore della derivata tramite regressione lineare
  - la conversione dell'output continuo in un segnale PWM binario
- richiede la necessità di eseguire numerosi test per individuare i parametri ottimali del controllore
- offre un tempo di assestamento maggiore rispetto al controllore ad isteresi, ma garantisce un controllo molto più stabile ed efficace
della temperatura

## slide 8 - conclusioni
- il firmware ha un'ottimo ritardo medio di risposta, garantendo un'ottima stabilità nella sincronizzazione dei vari compiti con un tempo di ritardo medio pari a  110 µsec (durata di una singola iterazione della funzione loop()), inoltre occupa solo il 10% circa di memoria RAM e di memoria flash
- essendo leggero e molto efficiente, lascia a disposizione ampio margine per integrare altre funzionalità, anche con un carico computazionale elevato
- durante il processo di taratura dei parametri del controllore PID è stato possibile osservare ed approfondire il comportamento delle varie componenti del PID in funzione di diverse configurazioni di sistema,

## slide 9 - sviluppi futuri
- l'integrazione di un sistema per l'umidità permetterebbe di completare il sistema di controllo dell'ambiente interno dell'incubatrice, si prevede la necessità di usare controllori multivariabile, dato il legame tra temperatura ed umidità
- la sostituzione dei relè con dei mosfet logic level permetterebbe di evitare continue commutazioni che a lungo andare potrebbero portare alla rottura dei relè, inoltre permettono di ridurre il periodo di pwm ottenendo una risposta più rapida e precisa
- la taratura dei parametri ha richiesto numerose prove e la modellazione del sistema tramite una semplice funzione con singolo polo si è rivelata troppo approssimativa, per favorire successive operazioni di progettazione e taratura di nuovi sistemi di controllo, potrebbe essere utile sviluppare un modello utilizzabile in simulazioni software
- dopo aver compreso il limite della componente integrale, si ritiene che per migliorare ancora le prestazioni del controllo di temperatura, si potrebbe implementare un controllore ibrido che agisce con diversi approcci di controllo in funzione dell'azione da svolgere (preheating, mantenimento della temperatura) in modo da tarare i parametri in maniera ottimale per soddisfare le esigenze di ogni fase
