% -----------------------------------------------------------------------------
% ------------- test1_analysis.m - calcola il periodo di warm-up --------------
% -----------------------------------------------------------------------------
%
% Questa funzione calcola il periodo di warm-up di un sistema a partire dai
% dati acquisiti in un file CSV dalla funzione serial_read.m durante
% l'esecuzione del test di warm-up.
%
% Il periodo di warm-up viene calcolato come il tempo necessario affinché
% la temperatura aumenti di 11°C rispetto al valore iniziale. Come istante
% iniziale si considera l'istante in cui l'output del controllore assume
% un valore non nullo.
%
% La temperatura iniziale viene calcolata come la media dei primi 10 valori
% letti dal sensore, per cui risulta opportuno che prima di accendere il
% riscaldatore, il sistema rimanga in equilibrio termico con l'ambiente
% circostante per il tempo necessario ad acquisire almeno 10 dati.
%
% La funzione accetta i seguenti parametri da riga di comando:
%  - filename:
%    nome del file CSV da leggere (obbligatorio)
%
%  - start_time:
%    indice di inizio per l'analisi dei dati (opzionale, default = 1)
%
%  - end_time:
%    indice di fine per l'analisi dei dati (opzionale, default = ultimo)
%

function test1_analysis(filename, start_time, end_time)

	%% ------ parsing dei parametri e assegnazione dei valori di default ------

	% nome del file CSV da leggere (obbligatorio)
	if nargin < 1 || isempty(filename)
		error('Uso corretto: csv_read <nome_file.csv>');
	end

	% indice di inizio per l'analisi dei dati (default = 1)
	if nargin < 2 || isempty(start_time)
		start_time = 1;
	end

	% indice di fine per l'analisi dei dati (default = ultimo dato)
	if nargin < 3 || isempty(end_time)
		end_time = [];
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

	% estrae il vettore con i tempi, le temperature, l'output e il setpoint finale

	% estrae il vettore con i tempi, le temperature e il setpoint
	if isempty(end_time), end_time = height(data); end

	time = data.Time_Seconds(start_time:end_time);
	temp = data.Temperature(start_time:end_time);
	setpoint = data.Setpoint_Temperature(end_time);

	% fa partire il tempo da 0 per una migliore leggibilità del grafico
	time = time - time(1);

	% estrae il vettore con l'output del controllore (PWM o output)
	output = [];
	if ismember('T_PWM', data.Properties.VariableNames) && ~all(isnan(data.T_PWM))
		output = data.T_PWM(start_time:end_time);
	elseif ismember('T_output', data.Properties.VariableNames) && ~all(isnan(data.T_output))
		output = data.T_output(start_time:end_time);
	end

	% calcola la temperatura iniziale e l'istante in cui inizia il warm-up
	start_temperature = sum(temp(1:10)) / 10; % media dei primi 10 valori
	start_warmup_idx = find(output > 0, 1); % primo istante in cui l'output del controllore è non nullo

	% verifica che sia stato rilevato un output del controllore
	if isempty(start_warmup_idx)
		error('Errore: Non è stato rilevato alcun output del controllore nel file CSV.');
	elseif start_warmup_idx < 10
		warning('Attenzione: L''output del controllore è stato rilevato prima di acquisire 10 valori di temperatura. Il calcolo della temperatura iniziale potrebbe non essere accurato.');
	end

	% verifica che il setpoint finale sia regolato correttamente
	if abs(setpoint - 12 - start_temperature) > 0.1
		warning('Attenzione: Il setpoint non è stato regolato correttamente. Il periodo di warm-up potrebbe non essere calcolato correttamente.');
	end

	%% --------------------- calcolo del tempo di warm-up ---------------------

	% calcolo del tempo di warm-up
	warmup_threshold = start_temperature + 11; % soglia di temperatura per il warm-up
	end_warmup_idx = find(temp >= warmup_threshold, 1); % primo istante in cui la temperatura supera il valore di soglia

	% verifica che sia stato rilevato il superamento della soglia di temperatura
	if isempty(end_warmup_idx)
		error('Errore: La temperatura non ha raggiunto il valore di soglia di %.2f °C nel file CSV.', warmup_threshold);
	end

	% stampa dei risultati dell'analisi
	fprintf(' --- file analizzato: %s ---\n', filename);
	fprintf(' - temperatura iniziale: .. %.2f °C\n', start_temperature);
	fprintf(' - temperatura finale: .... %.2f °C\n', temp(end_warmup_idx));
	fprintf(' - setpoint: .............. %.2f °C\n', setpoint);
	fprintf(' - tempo di warm-up: ...... %.2f s\n', time(end_warmup_idx) - time(start_warmup_idx));
	fprintf(' - inizio warm-up: ........ %.2f s\n', time(start_warmup_idx));
	fprintf(' - fine warm-up: .......... %.2f s\n\n', time(end_warmup_idx));


	%% ----------------------- generazione del grafico ------------------------

	% crea una finestra grafica per stampare il grafico
	figure('Name', sprintf('Analisi test warm-up - %s', filename), 'Color', 'w', 'Position', [950, 100, 560, 840]);

	% crea il layout a due righe per i grafici
	t_layout = tiledlayout(5, 1, 'TileSpacing', 'loose', 'Padding', 'compact');

	% --- grafico con i dati di temperatura e setpoint ---
	ax1 = nexttile(t_layout, [3, 1]);
	grid on; hold on;

	% Definizione dei colori (esadecimali)
	col_segnale  = '#ff0000';
	col_setpoint = [0.8500 0.3250 0.0980];
	col_band1    = [0.9290 0.6940 0.1250];
	col_band2    = [0.4660 0.6740 0.1880];

	% traccia la risposta del sistema
	plot(time, temp, 'Color', col_segnale, 'LineStyle', '-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');

	% traccia il setpoint come linea tratteggiata nera
	yline(setpoint, 'Color', col_setpoint, 'LineStyle', '--', 'LineWidth', 1.5, 'DisplayName', 'Setpoint', 'LabelHorizontalAlignment', 'left');

	% traccia le bande di inizio e fine del periodo warm-up
	warmup_lower_band = start_temperature;
	warmup_upper_band = warmup_threshold;
	yline(warmup_lower_band, 'Color', col_band1, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'Temperatura di inizio warm-up');
	text(time(1) + 0.30 * (time(end) - time(1)), warmup_lower_band, sprintf('Temperatura di inizio warm-up: %.2f °C', warmup_lower_band), 'Color', col_band1, 'VerticalAlignment', 'bottom', 'HorizontalAlignment', 'center');
	yline(warmup_upper_band, 'Color', col_band2, 'LineStyle', '--', 'LineWidth', 1, 'DisplayName', 'Temperatura di fine warm-up');
	text(time(1) + 0.30 * (time(end) - time(1)), warmup_upper_band, sprintf('Temperatura di fine warm-up: %.2f °C', warmup_upper_band), 'Color', col_band2, 'VerticalAlignment', 'top', 'HorizontalAlignment', 'center');

	% mostra inizio tempo di salita sul grafico
	plot(time(start_warmup_idx), temp(start_warmup_idx), 'Color', col_band1, 'Marker', 'd', 'MarkerSize', 8, 'MarkerFaceColor', col_band1, 'HandleVisibility', 'off');
	xline(time(start_warmup_idx), 'Color', col_band1, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Tempo di inizio warm-up');
	text(time(start_warmup_idx), temp(1) + 0.70 * (temp(end) - temp(1)), sprintf('  Istante di inizio warm-up:\n  %.2f secondi', time(start_warmup_idx)), 'Color', col_band1, 'VerticalAlignment', 'middle', 'HorizontalAlignment', 'left');
	%text(time(start_warmup_idx), temp(start_warmup_idx), sprintf('  Start warm-up time: %.2fs', time(start_warmup_idx)), 'Color', col_band1, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');

	% mostra fine tempo di salita sul grafico
	plot(time(end_warmup_idx), temp(end_warmup_idx), 'Color', col_band2, 'Marker', 'd', 'MarkerSize', 8, 'MarkerFaceColor', col_band2, 'HandleVisibility', 'off');
	xline(time(end_warmup_idx), 'Color', col_band2, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Tempo di fine warm-up');
	text(time(end_warmup_idx), temp(1) + 0.70 * (temp(end) - temp(1)), sprintf('  Istante di fine warm-up:\n  %.2f secondi', time(end_warmup_idx)), 'Color', col_band2, 'VerticalAlignment', 'middle', 'HorizontalAlignment', 'left');
	%text(time(end_warmup_idx), temp(end_warmup_idx), sprintf('  End Warm-up time: %.2fs', time(end_warmup_idx)), 'Color', col_band2, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');

	% --- estetica e legenda ---
	ylabel('Temperatura (°C)');
	legend('Location', 'southeast');
	ax1.YTick = floor(min(temp)) : 1 : ceil(max(temp));
	ax1.XTick = 0 : 50 : max(time);
	ax1.XMinorGrid = 'on';

	% --- grafico con i dati di PID, PWM e output della temperatura ---
	ax2 = nexttile(t_layout, [2, 1]);
	grid on; hold on;

	% colori personalizzati per i dati del PID
	c_P = [0.8500 0.3250 0.0980];
	c_I = [0.9290 0.6940 0.1250];
	c_D = [0.4940 0.1840 0.5560];
	c_Out = [0.4660 0.6740 0.1880];

	% set di dati da mostrare (segnali pid e output)
	if ismember('T_PID_p', data.Properties.VariableNames) && ~all(isnan(data.T_PID_p))
		plot(time, data.T_PID_p(start_time:end_time), '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', 'P_{PID}');
	end
	if ismember('T_PID_i', data.Properties.VariableNames) && ~all(isnan(data.T_PID_i))
		plot(time, data.T_PID_i(start_time:end_time), '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', 'I_{PID}');
	end
	if ismember('T_PID_d', data.Properties.VariableNames) && ~all(isnan(data.T_PID_d))
		plot(time, data.T_PID_d(start_time:end_time), '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', 'D_{PID}');
	end
	if ismember('T_PID_out', data.Properties.VariableNames) && ~all(isnan(data.T_PID_out))
		plot(time, data.T_PID_out(start_time:end_time), '-.', 'Color', c_Out, 'LineWidth', 1, 'DisplayName', 'Out_{PID}');
	end
	if ismember('T_PWM', data.Properties.VariableNames) && ~all(isnan(data.T_PWM))
		plot(time, data.T_PWM(start_time:end_time), 'k-', 'LineWidth', 1.5, 'DisplayName', 'Duty cycle_{PWM}');
	end
	if ismember('T_output', data.Properties.VariableNames) && ~all(isnan(data.T_output))
		plot(time, data.T_output(start_time:end_time), 'k-', 'LineWidth', 1.5, 'DisplayName', 'Output');
	end

	% preferenze di visualizzazione
	ylabel('Output controllore'); % etichetta asse y
	legend('Location', 'northeast'); % legenda
	ax2.XTick = 0 : 50 : max(time);
	ax2.XMinorGrid = 'on';

	% --- linking degli assi x ---
	linkaxes([ax1, ax2], 'x');
	xlim(ax1, [time(1), time(end)]);
	xlabel(t_layout, 'Tempo (secondi)');

end
