% -----------------------------------------------------------------------------
% --------- test2_analysis.m - calcola le prestazioni del controllore ---------
% -----------------------------------------------------------------------------
%
% Questa funzione calcola i vari parametri di performance del controllore
% analizzando i dati acquisiti in un file CSV dalla funzione serial_read.m
% durante l'esecuzione del test di performance del controllore.
%
% Il test di performance prevede di partire da una condizione stazionaria del
% sistema con setpoint iniziale a 32°C e, successivamente, aumentare il setpoint
% a 36°C, osservando la risposta del sistema.
%
% I parametri analizzati sono i seguenti:
%  - settling time:
%    tempo necessario affinché la variabile di processo si stabilizzi entro
%    il range di ±0.5°C rispetto al nuovo setpoint
%
%  - overshoot:
%    massimo valore raggiunto dalla temperatura rispetto al nuovo setpoint,
%    espresso come differenza di temperatura in °C
%
%  - steady-state errors:
%    errori massimi (positivo e negativo) della temperatura rispetto al nuovo
%    setpoint durante il regime stazionario, espressi come differenza di
%    temperatura in °C
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

function test2_analysis(filename, start_time, end_time)

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

	% estrae il vettore con i tempi, le temperature e i setpoint iniziale e finale
	if isempty(end_time), end_time = height(data); end

	time = data.Time_Seconds(start_time:end_time);
	temp = data.Temperature(start_time:end_time);
	setpoint = data.Setpoint_Temperature(start_time:end_time);
	setpoint1 = setpoint(1);
	setpoint2 = setpoint(end);

	% calcola l'istante in cui il setpoint cambia da 32°C a 36°C
	setpoint_change_idx = find(setpoint ~= setpoint1, 1);

	% verifica che il setpoint iniziale sia regolato correttamente
	if setpoint1 ~= 32
		warning('Attenzione: Il setpoint iniziale non è stato regolato correttamente. I risultati dell''analisi potrebbero non essere accurati.');
	end

	% verifica che il setpoint finale sia regolato correttamente
	if setpoint2 ~= 36
		warning('Attenzione: Il setpoint finale non è stato regolato correttamente. I risultati dell''analisi potrebbero non essere accurati.');
	end

	% verifica l'effettivo cambiamento del setpoint
	%if isempty(setpoint_change_idx)
	%	error('Errore: Non è stato rilevato alcun cambiamento del setpoint nel file CSV. Assicurarsi che il test sia stato eseguito correttamente.');
	%end


	%% ---------------------- calcolo delle metriche PID ----------------------

	% seleziona i dati dal momento del cambio di setpoint in poi e azzera
	% l'asse temporale a partire da quell'istante
	t_post_change = time(setpoint_change_idx:end) - time(setpoint_change_idx);
	temp_post_change = temp(setpoint_change_idx:end);


	% --- calcolo del settling time (con finestra mobile di 1 min) ---
	% calcola il numero di campioni necessari per coprire 1 minuto
	t_stabilita = 60;
	campioni_stabilita = ceil(t_stabilita / mean(diff(t_post_change)));

	% crea un array logico: 1 se la temp è nel range, 0 se è fuori
	in_bounds = abs(temp_post_change - setpoint2) <= 0.5;

	% usa 'movsum' guardando "in avanti": somma i valori logici in finestre
	% di dimensione pari al numero di campioni 'campioni_stabilità'; se la
	% somma è esattamente uguale a 'campioni_stabilità', significa che tutti
	% i campioni in quella finestra valgono 1 (cioè sono nel range)
	somma_finestra = movsum(in_bounds, [0, campioni_stabilita - 1]);

	% trova il primo indice in cui l'intera finestra successiva è nel range
	settling_idx = find(somma_finestra == campioni_stabilita, 1);

	if isempty(settling_idx)
		% non è mai rimasto nel range per 1 minuto consecutivo
		settling_time = NaN;
	else
		% il settling time è l'istante in cui inizia questo periodo di stabilità
		settling_time = t_post_change(settling_idx);
	end

	% --- calcolo dell'overshoot ---
	% individua la temperatura massima raggiunta dal cambio di setpoint
	[max_temp_post, peak_idx] = max(temp_post_change);

	% calcola l'overshoot come differenza tra la temperatura massima e il nuovo setpoint,
	% viene impostato a 0 se la differenza è negativa (se non c'è overshoot)
	overshoot = max(0, max_temp_post - setpoint2);


	% --- calcolo degli steady-state errors ---
	if isempty(settling_idx)
		% in caso di instabilità, non è possibile calcolare gli errori a regime
		steady_state_error_pos = NaN;
		steady_state_error_neg = NaN;
	else
		% estrapola solo la porzione di temperature dopo che il sistema si è stabilizzato
		steady_state_temp = temp_post_change(settling_idx:end);

		% calcola gli errori rispetto al nuovo setpoint
		errors = steady_state_temp - setpoint2;

		% individua l'errore massimo positivo e negativo
		steady_state_error_pos = max(errors);
		steady_state_error_neg = min(errors);

		% pulizia dei dati (se il max errore è negativo, lo poniamo a 0 per chiarezza)
		if steady_state_error_pos < 0, steady_state_error_pos = 0; end
		if steady_state_error_neg > 0, steady_state_error_neg = 0; end
	end

	% stampa dei risultati dell'analisi
	fprintf(' --- file analizzato: %s ---\n', filename);
	fprintf(' - setpoint iniziale: ........ %.2f °C - (exp. 32°C)\n', setpoint1);
	fprintf(' - setpoint finale: .......... %.2f °C - (exp. 36°C)\n', setpoint2);
	fprintf(' - settling time: ............ %.2f sec - (exp. < 900 sec)\n', settling_time);
	fprintf(' - overshoot: ................ +%.2f °C - (exp. < 2°C)\n', overshoot);
	fprintf(' - pos. steady-state error: .. +%.2f °C - (exp. < 0.5°C)\n', steady_state_error_pos);
	fprintf(' - neg. steady-state error: .. %.2f °C - (exp. > -0.5°C)\n\n', steady_state_error_neg);


	%% ----------------------- generazione del grafico ------------------------

	% crea una finestra grafica per stampare il grafico
	figure('Name', sprintf('Analisi test settling time, overshoot, steady-state error - %s', filename), 'Color', 'w', 'Units', 'centimeters', 'Position', [40, 7, 16, 10]);

	% impostazioni di base
	grid on; hold on;

	% Definizione dei colori (esadecimali)
	col_segnale  = '#ff0000';
	col_setpoint = [0.8500 0.3250 0.0980];
	col_settling = [0.9290 0.6940 0.1250];
	col_peak     = '#ed5c1e';

	% traccia la risposta del sistema
	plot(time, temp, 'Color', col_segnale, 'LineStyle', '-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');

	% traccia il setpoint come linea tratteggiata nera
	plot(time, setpoint, 'Color', col_setpoint, 'LineStyle', '--', 'LineWidth', 1.5, 'DisplayName', 'Setpoint');

	% --- settling time ---
	% traccia le bande di assestamento di 0.5°C sopra e sotto il setpoint
	yline(setpoint2 + 0.5, 'Color', col_settling, 'LineStyle', ':', 'LineWidth', 1.8, 'DisplayName', 'Banda Assestamento (\pm5%)');
	yline(setpoint2 - 0.5, 'Color', col_settling, 'LineStyle', ':', 'LineWidth', 1.8, 'HandleVisibility', 'off');

	% mostra la fine del tempo di assestamento sul grafico
	if ~isnan(settling_time)
		t_settling_abs = time(setpoint_change_idx + settling_idx - 1);
    	y_settling_abs = temp_post_change(settling_idx);
		plot(t_settling_abs, y_settling_abs, 'Color', col_settling, 'Marker', 's', 'MarkerSize', 8, 'MarkerFaceColor', col_settling, 'HandleVisibility', 'off');
		xline(t_settling_abs, 'Color', col_settling, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Settling Time', 'LabelVerticalAlignment', 'bottom');
	end

	% --- overshoot ---
	% individua il picco massimo della temperatura dopo il cambio di setpoint
	if ~isnan(max_temp_post)
		t_peak = time(setpoint_change_idx + peak_idx - 1);
		y_peak = temp_post_change(peak_idx);
		plot(t_peak, y_peak, 'Color', col_peak, 'Marker', 'o', 'MarkerSize', 8, 'MarkerFaceColor', col_peak, 'HandleVisibility', 'off');
		text(t_peak, y_peak, sprintf('  Overshoot: %.02f°C', overshoot), 'Color', col_peak, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
	end

	% --- estetica e legenda ---
	xlabel('Tempo (secondi)');
	ylabel('Temperatura (°C)');
	legend('Location', 'southeast');
	hold off;
end
