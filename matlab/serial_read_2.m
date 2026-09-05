function serial_read_2(showPlot, portName, baudRate, filename)
	% --- 1. impostazione parametri di default se non forniti dall'utente ---
	if nargin < 1 || isempty(showPlot)
		showPlot = 3; % 0 = nessuno, 1 = temp, 2 = rh, 3 = entrambi
	end
	if nargin < 2 || isempty(portName)
		%portName = "/dev/cu.usbserial-110";
		portName = "/dev/cu.usbmodem1101";
	end
	if nargin < 3 || isempty(baudRate)
		baudRate = 115200;
	end
	if nargin < 4 || isempty(filename)
		filename = sprintf('serial_read/data_%s.csv', datestr(now, 'yyyymmdd_HHMMSS'));
	end

	% --- 2. configurazione porta seriale ---
	try
		arduinoObj = serialport(portName, baudRate); % crea l'oggetto seriale
		configureTerminator(arduinoObj, "CR/LF"); % imposta terminatore di riga
		flush(arduinoObj); % libera il buffer da eventuali dati vecchi
	catch ME
		error('Errore nell''apertura della porta seriale %s: %s', portName, ME.message);
	end

	% --- 3. configurazione file CSV ---
	% estrae il percorso della cartella dal nome del file
	[folderPath, ~, ~] = fileparts(filename);

	% crea un'eventuale cartella richiesta se non è già presente
	if ~isempty(folderPath) && ~exist(folderPath, 'dir')
		mkdir(folderPath);
	end

	% apre il file in modalità scrittura ('w' write, 'a' append)
	fileID = fopen(filename, 'w');
	if fileID == -1
		error('Impossibile aprire il file %s per la scrittura.', filename);
	end

	% scrive l'intestazione del CSV (la prima riga)
	fprintf(fileID, ['Time_Seconds,Temperature,Humidity,Setpoint_Temperature,Setpoint_Humidity,', ...
						'Ctrl,Error_Code,T_PWM,RH_PWM,', ...
						'T_PID_p,T_PID_i,T_PID_d,T_PID_out,', ...
						'RH_PID_p,RH_PID_i,RH_PID_d,RH_PID_out,', ...
						'T_output,RH_output,T_min_thld,T_max_thld,RH_min_thld,RH_max_thld\n']);

	% --- 4. sistema di sicurezza per chiusura file e porta seriale ---
	% onCleanup garantisce la chiusura del file CSV e della porta seriale non
	% appena lo script termina (sia con successo, sia per errore, sia per Ctrl+C)
	cleanupObj = onCleanup(@() cleanUpRoutine(arduinoObj, fileID));

	% --- 5. setup grafico in tempo reale ---
	if showPlot > 0
		% crea una finestra grafica per stampare il grafico in tempo reale
		fig = figure('Name', 'Monitoraggio Real-Time Arduino', 'Color', 'w', 'Position', [100, 100, 1000, 700]);

		% flag di controllo e Pulsante di stop ---
		fig.UserData = true; % Variabile nascosta che mantiene vivo il ciclo
		uicontrol('Parent', fig, 'Style', 'pushbutton', 'String', 'Ferma acquisizione', ...
				'Units', 'pixels', 'Position', [15 15 100 35], ...
				'Callback', @(src, event) set(fig, 'UserData', false));
				%'BackgroundColor', [0.8 0.2 0.2], 'ForegroundColor', 'w', ...
				%'FontWeight', 'bold', 'FontSize', 10, ...

		% crea 2 righe se showPlot è 3, altrimenti 1 riga
		if showPlot == 3
			t_layout = tiledlayout(2, 1, 'TileSpacing', 'compact', 'Padding', 'compact');
		else
			t_layout = tiledlayout(1, 1, 'TileSpacing', 'compact', 'Padding', 'compact');
		end

		maxPts = 3600;

		c_P = [0.8500 0.3250 0.0980];
		c_I = [0.9290 0.6940 0.1250];
		c_D = [0.4940 0.1840 0.5560];
		c_Out = [0.4660 0.6740 0.1880];

		% -- Riquadro 1: Temperatura --
		if showPlot == 1 || showPlot == 3
			ax1 = nexttile(t_layout);
			title('Controllo Temperatura');
			grid on; hold on;

			yyaxis left;
			ylabel('Temperatura (°C)');
			line_temp  = animatedline('Color', 'r', 'LineWidth', 1.5, 'DisplayName', 'Misurazione', 'MaximumNumPoints', maxPts);
			line_set_t = animatedline('Color', 'r', 'LineStyle', '--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint', 'MaximumNumPoints', maxPts);
			line_t_min_thld = animatedline('Color', 'r', 'LineStyle', ':', 'LineWidth', 1, 'DisplayName', 'T_{min_thld}', 'MaximumNumPoints', maxPts);
			line_t_max_thld = animatedline('Color', 'r', 'LineStyle', ':', 'LineWidth', 1, 'DisplayName', 'T_{max_thld}', 'MaximumNumPoints', maxPts);

			yyaxis right;
			ylabel('Segnali (PWM/PID)');
			line_t_pwm = animatedline('Color', 'k', 'LineWidth', 1.5, 'DisplayName', 'PWM_{T}', 'MaximumNumPoints', maxPts);
			line_t_output = animatedline('Color', 'k', 'LineWidth', 1.5, 'DisplayName', 'Output_{T}', 'MaximumNumPoints', maxPts);
			line_t_p   = animatedline('Color', c_P, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'P_{PID}', 'MaximumNumPoints', maxPts);
			line_t_i   = animatedline('Color', c_I, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'I_{PID}', 'MaximumNumPoints', maxPts);
			line_t_d   = animatedline('Color', c_D, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'D_{PID}', 'MaximumNumPoints', maxPts);
			line_t_out = animatedline('Color', c_Out, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Out_{PID}', 'MaximumNumPoints', maxPts);
			legend('Location', 'westoutside');

			ax1.XLimMode = 'auto';
		end

		% -- Riquadro 2: Umidità --
		if showPlot == 2 || showPlot == 3
			ax2 = nexttile(t_layout);
			title('Controllo Umidità');
			grid on; hold on;

			yyaxis left;
			ylabel('Umidità (%)');
			line_rh     = animatedline('Color', 'b', 'LineWidth', 1.5, 'DisplayName', 'Misurazione', 'MaximumNumPoints', maxPts);
			line_set_rh = animatedline('Color', 'b', 'LineStyle', '--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint', 'MaximumNumPoints', maxPts);
			line_rh_min_thld = animatedline('Color', 'b', 'LineStyle', ':', 'LineWidth', 1, 'DisplayName', 'RH_{min_thld}', 'MaximumNumPoints', maxPts);
			line_rh_max_thld = animatedline('Color', 'b', 'LineStyle', ':', 'LineWidth', 1, 'DisplayName', 'RH_{max_thld}', 'MaximumNumPoints', maxPts);
			yyaxis right;
			ylabel('Segnali (PWM/PID)');
			line_rh_pwm = animatedline('Color', 'k', 'LineWidth', 1.5, 'DisplayName', 'PWM_{RH}', 'MaximumNumPoints', maxPts);
			line_rh_output = animatedline('Color', 'k', 'LineWidth', 1.5, 'DisplayName', 'Output_{RH}', 'MaximumNumPoints', maxPts);
			line_rh_p   = animatedline('Color', c_P, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'P_{PID}', 'MaximumNumPoints', maxPts);
			line_rh_i   = animatedline('Color', c_I, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'I_{PID}', 'MaximumNumPoints', maxPts);
			line_rh_d   = animatedline('Color', c_D, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'D_{PID}', 'MaximumNumPoints', maxPts);
			line_rh_out = animatedline('Color', c_Out, 'LineStyle', '-.', 'LineWidth', 1.2, 'DisplayName', 'Out_{PID}', 'MaximumNumPoints', maxPts);
			legend('Location', 'westoutside');


			ax2.XLimMode = 'auto';
		end

		% sincronizza l'asse X solo se entrambi i grafici sono presenti
		if showPlot == 3
			linkaxes([ax1, ax2], 'x');
		end

		% etichetta asse X
		xlabel(t_layout, 'Tempo trascorso (secondi)');
	end

	% --- 5. Ciclo di lettura ---

	% Inizializzazione della variabile di tempo zero.
	% Rimane 'empty' fino al primo messaggio valido ricevuto.
	t0 = [];

	try
		while true

			% controllo uscita sicura
			% esce dal ciclo se la finestra è stata chiusa (X) o se hai premuto il tasto di arresto
			if showPlot > 0
				if ~isvalid(fig) || isequal(fig.UserData, false)
					disp('Acquisizione interrotta dal grafico. Salvataggio in corso...');
					break;
				end
			end

			% Legge una riga in formato stringa
			lineStr = readline(arduinoObj);

			% Controlla se la riga è il log di sistema
			if startsWith(lineStr, "LOG:")

				% Calcolo del tempo relativo in secondi
				if isempty(t0)
					t0 = tic; % Salva il momento di inizio al primo log ricevuto
					t_elapsed = 0.0;
				else
					t_elapsed = toc(t0); % Calcola i secondi trascorsi da t0
				end

				% --- Estrazione Dati tramite Regex ---
				% Se il valore non c'è (es. PID disabilitato via #if), extractNum restituisce NaN
				temp    = extractNum(lineStr, 'T:\s*([-+]?\d*\.?\d+)');
				rh      = extractNum(lineStr, 'RH:\s*([-+]?\d*\.?\d+)');
				set_t   = extractNum(lineStr, 'Set_T:\s*([-+]?\d*\.?\d+)');
				set_rh  = extractNum(lineStr, 'Set_RH:\s*([-+]?\d*\.?\d+)');

				ctrl    = extractStr(lineStr, 'Ctrl:\s*([A-Z]+)');
				err     = extractNum(lineStr, 'Error_Code:\s*(\d+)');

				t_pwm   = extractNum(lineStr, 'T_PWM_norm:\s*([-+]?\d*\.?\d+)');
				rh_pwm  = extractNum(lineStr, 'RH_PWM_norm:\s*([-+]?\d*\.?\d+)');

				t_p     = extractNum(lineStr, 'T_PID_p:\s*([-+]?\d*\.?\d+)');
				t_i     = extractNum(lineStr, 'T_PID_i:\s*([-+]?\d*\.?\d+)');
				t_d     = extractNum(lineStr, 'T_PID_d:\s*([-+]?\d*\.?\d+)');
				t_out   = extractNum(lineStr, 'T_PID_out:\s*([-+]?\d*\.?\d+)');

				rh_p    = extractNum(lineStr, 'RH_PID_p:\s*([-+]?\d*\.?\d+)');
				rh_i    = extractNum(lineStr, 'RH_PID_i:\s*([-+]?\d*\.?\d+)');
				rh_d    = extractNum(lineStr, 'RH_PID_d:\s*([-+]?\d*\.?\d+)');
				rh_out  = extractNum(lineStr, 'RH_PID_out:\s*([-+]?\d*\.?\d+)');

				t_output  = extractNum(lineStr, 'T_output:\s*(\d+)');
				rh_output = extractNum(lineStr, 'RH_output:\s*(\d+)');

				t_min_thld = extractNum(lineStr, 'T_min_thld:\s*([-+]?\d*\.?\d+)');
				t_max_thld = extractNum(lineStr, 'T_max_thld:\s*([-+]?\d*\.?\d+)');
				rh_min_thld = extractNum(lineStr, 'RH_min_thld:\s*([-+]?\d*\.?\d+)');
				rh_max_thld = extractNum(lineStr, 'RH_max_thld:\s*([-+]?\d*\.?\d+)');

				% --- Scrittura sul CSV ---
				fprintf(fileID, '%.2f,%.2f,%.2f,%.2f,%.2f,%s,%d,%.0f,%.0f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.0f,%.0f,%.2f,%.2f,%.2f,%.2f\n', ...
					t_elapsed, temp, rh, set_t, set_rh, ctrl, err, t_pwm, rh_pwm, ...
					t_p, t_i, t_d, t_out, rh_p, rh_i, rh_d, rh_out, ...
					t_output, rh_output, t_min_thld, t_max_thld, rh_min_thld, rh_max_thld);

				% Aggiornamento Grafico
				if showPlot > 0 && isvalid(fig)
					% Dati temperatura
					if showPlot == 1 || showPlot == 3
						addpoints(line_temp, t_elapsed, temp);
						addpoints(line_set_t, t_elapsed, set_t);
						addpoints(line_t_pwm, t_elapsed, t_pwm);
						addpoints(line_t_p, t_elapsed, t_p);
						addpoints(line_t_i, t_elapsed, t_i);
						addpoints(line_t_d, t_elapsed, t_d);
						addpoints(line_t_out, t_elapsed, t_out);
						addpoints(line_t_output, t_elapsed, t_output);
						addpoints(line_t_min_thld, t_elapsed, t_min_thld);
						addpoints(line_t_max_thld, t_elapsed, t_max_thld);
					end

					% Dati umidità
					if showPlot == 2 || showPlot == 3
						addpoints(line_rh, t_elapsed, rh);
						addpoints(line_set_rh, t_elapsed, set_rh);
						addpoints(line_rh_pwm, t_elapsed, rh_pwm);
						addpoints(line_rh_p, t_elapsed, rh_p);
						addpoints(line_rh_i, t_elapsed, rh_i);
						addpoints(line_rh_d, t_elapsed, rh_d);
						addpoints(line_rh_out, t_elapsed, rh_out);
						addpoints(line_rh_output, t_elapsed, rh_output);
						addpoints(line_rh_min_thld, t_elapsed, rh_min_thld);
						addpoints(line_rh_max_thld, t_elapsed, rh_max_thld);
					end

					% limitrate forza MATLAB a non aggiornare l'UI più di 20 volte
					% al secondo, prevenendo lag e crash in caso di alta frequenza di dati
					drawnow limitrate;
				end
			end
		end

	% Gestione di interruzioni catturate (errori di sistema o Ctrl+C)
	catch ME
		% se l'errore è un'interruzione da tastiera (Ctrl+C),
		% mostra un messaggio di chiusura pulita. Altrimenti, rilancia l'errore.
		if strcmp(ME.identifier, 'MATLAB:dispatcher:Interrupt')
			disp('Interruzione da tastiera (Ctrl+C) completata in modo pulito.');
		else
			% Se è un VERO errore di sistema, lo mostra a schermo
			rethrow(ME);
		end
	end
end

% --- Funzioni Ausiliarie Locali ---

% Cerca un numero corrispondente al pattern. Restituisce NaN se non lo trova.
function val = extractNum(testo, pattern)
	m = regexp(testo, pattern, 'tokens', 'once');
	if isempty(m)
		val = NaN;
	else
		val = str2double(m{1});
	end
end

% Cerca una parola corrispondente al pattern. Restituisce "N/A" se non lo trova.
function val = extractStr(testo, pattern)
	m = regexp(testo, pattern, 'tokens', 'once');
	if isempty(m)
		val = "N/A";
	else
		val = string(m{1});
	end
end

% Funzione chiamata automaticamente in caso di errore o quando premi Ctrl+C
function cleanUpRoutine(arduinoObj, fileID)
	fprintf('\nChiusura connessione seriale e salvataggio file CSV in corso...\n');
	clear arduinoObj;
	if fileID ~= -1
		fclose(fileID);
	end
end
