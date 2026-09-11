% -----------------------------------------------------------------------------
% ------------ csv_performance.m - calcola le prestazioni del PID -------------
% -----------------------------------------------------------------------------
%
% La seguente funzione legge i dati da un file CSV generato dalla funzione
% serial_read.m, estrae le informazioni principali, calcola le metriche di
% prestazione del PID e genera un grafico con i risultati ottenuti.
%
% La funzione accetta i seguenti parametri da riga di comando:
%  - filename:
%    nome del file CSV da leggere (obbligatorio)
%
%  - variabile:
%    quale variabile analizzare (opzionale, default = 'Temperature')
%     - 'Temperature' = analizza la colonna 'Temperature'
%     - 'Humidity' = analizza la colonna 'Humidity'
%
%  - start_time:
%    indice di inizio per l'analisi dei dati (opzionale, default = 1)
%
%  - end_time:
%    indice di fine per l'analisi dei dati (opzionale, default = ultimo)
%
%  - start_value:
%    valore di equilibrio iniziale (opzionale, default = primo valore)
%

function csv_performance(filename, variabile, start_time, end_time, start_value)

	%% ------ parsing dei parametri e assegnazione dei valori di default ------

	% nome del file CSV da leggere (obbligatorio)
	if nargin < 1 || isempty(filename)
		error('Uso corretto: csv_read <nome_file.csv>');
	end

	% tipo di variabile da analizzare (default = 'Temperature')
	if nargin < 2
		variabile = 'Temperature';
	end

	% indice di inizio per l'analisi dei dati (default = 1)
	if nargin < 3 || isempty(start_time)
		start_time = 1;
	end

	% indice di fine per l'analisi dei dati (default = ultimo dato)
	if nargin < 4 || isempty(end_time)
		end_time = [];
	end

	% valore di equilibrio iniziale (default = primo valore)
	if nargin < 5 || isempty(start_value)
		start_value = [];
	end

	%% ----------------------- configurazione file CSV ------------------------

	% converte il nome del file in stringa (nel caso fosse un char array)
	filename = char(filename);

	% aggiunge in automatico .csv se viene omesso dall'utente
	if ~isfile(filename) && ~endsWith(filename, '.csv', 'IgnoreCase', true)
		if isfile([filename, '.csv'])
			filename = [filename, '.csv'];
		end
	end

	% controlla l'esistenza del file
	if ~isfile(filename)
		error('Errore: Il file "%s" non è stato trovato nella cartella corrente.', filename);
	end

	% importa i dati dal CSV usando le intestazioni delle colonne
	opts = detectImportOptions(filename);
	opts.VariableNamingRule = 'preserve';

	% legge i dati presenti come tabella
	data = readtable(filename, opts);

	% verifica che la variabile esista nel file
	if ~ismember(variabile, data.Properties.VariableNames)
		error('La colonna "%s" non esiste nel file CSV.', variabile);
	end

	% estrae il vettore temporale e la variabile da analizzare
	t = data.Time_Seconds;
	y = data.(variabile);

	% applica i limiti di tempo se specificati
	if ~isempty(end_time)
		t = t(start_time:end_time);
		y = y(start_time:end_time);
	else
		t = t(start_time:end);
		y = y(start_time:end);
	end

	% effettua la mappatura della risposta per farla partire da 0
	if isempty(start_value)
		start_value = y(1);
	end
	y_shifted = y - start_value;

	% estrae automaticamente il setpoint associato alla variabile
	nome_setpoint = sprintf('Setpoint_%s', variabile);
	if ismember(nome_setpoint, data.Properties.VariableNames)
		setpoint = data.(nome_setpoint)(end);
	else
		setpoint = y(end);
		warning('Colonna %s non trovata. Uso l''ultimo valore letto come riferimento.', nome_setpoint);
	end

	%% ---------------------- calcolo delle metriche PID ----------------------

	% calcolo delle metriche con la funzione stepinfo
	metriche = stepinfo(y_shifted, t, setpoint-start_value, ...
		'SettlingTimeThreshold', 0.05, ...
		'RiseTimeLimits', [0.1, 0.9]);

	% stampa i risultati ottenuti nella console di matlab
	disp('-----------------------------------------');
	fprintf('Risultati test PID (%s) per file: %s\n', variabile, filename);
	disp('-----------------------------------------');
	disp(metriche);

	%% ----------------------- generazione del grafico ------------------------

	% crea una finestra grafica per stampare il grafico
	figure('Name', sprintf('Analisi PID: %s - %s', variabile, filename), 'Color', 'w', 'Units', 'centimeters', 'Position', [40, 7, 16, 10]);

	% impostazioni di base
	grid on; hold on;

	% Definizione dei colori (esadecimali)
	col_segnale  = '#ff0000';
	col_setpoint = [0.8500 0.3250 0.0980];
	col_settling = [0.9290 0.6940 0.1250];
	col_rise     = [0.4660 0.6740 0.1880];
	col_peak     = '#ed5c1e';

	% traccia la risposta del sistema
	plot(t, y, 'Color', col_segnale, 'LineStyle', '-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');

	% traccia il setpoint come linea tratteggiata nera
	yline(setpoint, 'Color', col_setpoint, 'LineStyle', '--', 'LineWidth', 1.5, 'DisplayName', 'Setpoint', 'LabelHorizontalAlignment', 'left');

	% --- settling time ---
	% traccia le bande di assestamento del 2% sopra e sotto il setpoint
	settling_tolerance = 0.05; % Banda di assestamento di default del 2%
	settling_upper_band = setpoint + (setpoint - start_value) * settling_tolerance;
	settling_lower_band = setpoint - (setpoint - start_value) * settling_tolerance;
	yline(settling_upper_band, 'Color', col_settling, 'LineStyle', ':', 'LineWidth', 1.8, 'DisplayName', 'Banda Assestamento (\pm5%)');
	yline(settling_lower_band, 'Color', col_settling, 'LineStyle', ':', 'LineWidth', 1.8, 'HandleVisibility', 'off');

	% mostra la fine del tempo di assestamento sul grafico
	t_sett = metriche.SettlingTime;
	if ~isnan(t_sett)
		[~, idx_sett] = min(abs(t - t_sett));
		plot(t_sett, y(idx_sett), 'Color', col_settling, 'Marker', 's', 'MarkerSize', 8, 'MarkerFaceColor', col_settling, 'HandleVisibility', 'off');
		xline(t_sett, 'Color', col_settling, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Settling Time', 'LabelVerticalAlignment', 'bottom');
	end

	% --- rise time ---
	% traccia le bande di salita del 10% sopra e sotto il setpoint
	rise_tolerance = 0.1; % Banda di salita di default del 10%
	rise_upper_band = start_value + (setpoint - start_value) * (1 - rise_tolerance);
	rise_lower_band = start_value + (setpoint - start_value) * rise_tolerance;
	yline(rise_upper_band, 'Color', col_rise, 'LineStyle', ':', 'LineWidth', 1.8, 'DisplayName', 'Banda di salita (10-90%)');
	yline(rise_lower_band, 'Color', col_rise, 'LineStyle', ':', 'LineWidth', 1.8, 'HandleVisibility', 'off');

	% mostra inizio tempo di salita sul grafico
	idx_10 = find(y >= rise_lower_band, 1);
	if ~isempty(idx_10)
		plot(t(idx_10), y(idx_10), 'Color', col_rise, 'Marker', 'd', 'MarkerSize', 8, 'MarkerFaceColor', col_rise, 'HandleVisibility', 'off');
		%text(t(idx_10), y(idx_10), sprintf('Start Rise Time: %.2fs', t(idx_10)), 'Color', col_rise, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
		xline(t(idx_10), 'Color', col_rise, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Delimitatori del Rise Time', 'LabelVerticalAlignment', 'bottom');
	end

	% mostra fine tempo di salita sul grafico
	idx_90 = find(y >= rise_upper_band, 1);
	if ~isempty(idx_90)
		plot(t(idx_90), y(idx_90), 'Color', col_rise, 'Marker', 'd', 'MarkerSize', 8, 'MarkerFaceColor', col_rise, 'HandleVisibility', 'off');
		%text(t(idx_90), y(idx_90), sprintf('End Rise Time: %.2fs', t(idx_90)), 'Color', col_rise, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
		xline(t(idx_90), 'Color', col_rise, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Rise Time', 'LabelVerticalAlignment', 'bottom', 'HandleVisibility', 'off');
	end

	% --- overshoot ---
	t_peak = metriche.PeakTime;
	y_peak = metriche.Peak + start_value;
	plot(t_peak, y_peak, 'Color', col_peak, 'Marker', 'o', 'MarkerSize', 8, 'MarkerFaceColor', col_peak, 'HandleVisibility', 'off');
	%text(t_peak, y_peak, sprintf('  Overshoot: %.1f%%', metriche.Overshoot), 'Color', col_peak, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');

	% --- estetica e legenda ---
	xlabel('Tempo (secondi)');
	ylabel(variabile);
	legend('Location', 'southeast');
	hold off;
end
