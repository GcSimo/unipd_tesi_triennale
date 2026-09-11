function serial_read_old(portName, baudRate, filename)
	if nargin < 1 || isempty(portName)
		portName = "/dev/cu.usbserial-110"; % Modifica con la tua porta (es. "/dev/ttyUSB0" su Linux/Mac)
	end
	if nargin < 2 || isempty(baudRate)
		baudRate = 115200; % Assicurati che coincida con Serial.begin()
	end
	if nargin < 3 || isempty(filename)
		filename = sprintf('dati_arduino_%s.csv', datestr(now, 'yyyymmdd_HHMMSS')); % Nome di default del file CSV
	end

% --- 0. PULIZIA INIZIALE (SBLOCCO MEMORIA E PORTE) ---
clear all;  % Cancella tutte le variabili e chiude le porte seriali rimaste aperte
close all;  % Chiude tutte le finestre dei grafici precedenti
%clc;       % Pulisce la Command Window

% --- 1. CONFIGURAZIONE PORTA SERIALE ---
try
	% crea l'oggetto seriale
	arduinoObj = serialport(portName, baudRate);
	% imposta il terminatore di riga per comunicazioni seriali dall'Arduino
	configureTerminator(arduinoObj, "CR/LF");
	% pulisce eventuali dati vecchi rimasti nel buffer
	flush(arduinoObj);
catch ME
	error('Errore nell''apertura della porta seriale %s: %s', portName, ME.message);
end

% --- 2. CONFIGURAZIONE FILE CSV (SALVATAGGIO REAL-TIME) ---
% apre il file in modalità scrittura ('w' write, 'a' append)
fileID = fopen(filename, 'w');
if fileID == -1
	error('Impossibile aprire il file %s per la scrittura.', filename);
end

% SISTEMA DI SICUREZZA: onCleanup garantisce l'esecuzione di fclose(fileID)
% non appena lo script termina (sia con successo, sia per errore, sia per Ctrl+C)
cleanupObj = onCleanup(@() fclose(fileID));

% scrive l'intestazione del CSV (la prima riga)
fprintf(fileID, ['Timestamp,Temperature,Humidity,Setpoint_Temperature,Setpoint_Humidity,', ...
					'Status,Error_Code,T_PWM,RH_PWM,', ...
					'T_PID_p,T_PID_i,T_PID_d,T_PID_out,', ...
					'RH_PID_p,RH_PID_i,RH_PID_d,RH_PID_out\n']);

% --- 3. PARAMETRI DI ACQUISIZIONE ---
numCampioni = 3600; % numero di letture da effettuare (1h di dati a 1Hz)
dati = zeros(numCampioni, 1); % vettore preallocato per i dati
tempi = zeros(numCampioni, 1); % vettore preallocato per i tempi

% --- 4. SETUP DEL GRAFICO E PULSANTE DI STOP ---
% crea la figura per il grafico
fig = figure('Name', 'Acquisizione Arduino', 'NumberTitle', 'off');
hLine = plot(tempi, dati, 'b-', 'LineWidth', 1.5);
xlabel('Tempo (secondi)');
ylabel('Valore Sensore');
title('Andamento in tempo reale');
grid on;

% imposta il flag di esecuzione a true
setappdata(fig, 'runFlag', true);

% crea il pulsante in basso a sinistra tale che quando viene premuto,
% il flag di esecuzione viene impostato a false
btnStop = uicontrol('Style', 'pushbutton', 'String', 'Ferma Acquisizione', ...
					'Units', 'normalized', ...
					'Position', [0.75, 0.92, 0.23, 0.06], ...
					'Callback', @(~,~) setappdata(fig, 'runFlag', false));

% --- 5. CICLO DI LETTURA ---
disp('Acquisizione iniziata...');
tic; % avvia il cronometro per i timestamp

for i = 1:numCampioni
	% controllo di interruzione: termina il ciclo anticipatamente se il grafico
	% viene chiuso (~isgraphics) o se il pulsante viene premuto (runFlag = false)
	if ~isgraphics(fig) || ~getappdata(fig, 'runFlag')
		disp('Acquisizione interrotta dall''utente.');
		break;
	end

	% legge una riga di testo dalla seriale
	stringaDato = readline(arduinoObj);

	% converte la stringa in numero e lo salva
	dati(i) = str2double(stringaDato);
	tempi(i) = toc;

	% --> SCRITTURA IMMEDIATA NEL FILE CSV <--
	% %.3f indica 3 cifre decimali, %.2f indica 2 cifre decimali. \n va a capo.
	fprintf(fileID, '%.3f,%.2f\n', tempi(i), dati(i));

	% aggiorna i dati del grafico
	set(hLine, 'XData', tempi(1:i), 'YData', dati(1:i));

	% adatta l'asse X dinamicamente per mostrare l'andamento
	xlim([0, max(tempi(1:i) + 0.1)]);

	% forza MATLAB a disegnare subito il grafico
	drawnow;
end

% --- 6. CHIUSURA DEL FILE E PULIZIA ---
disp('Acquisizione completata.');

% se il grafico è ancora aperto, disabilita il pulsante di interruzione
if isgraphics(fig)
	set(btnStop, 'Enable', 'off', 'String', 'Completato');
end

% pulizia finale (opzionale ma raccomandata)
clear arduinoObj;
