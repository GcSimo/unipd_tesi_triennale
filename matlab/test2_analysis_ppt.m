% -----------------------------------------------------------------------------
% --------- test2_analysis.m - calcola le prestazioni del controllore ---------
% -----------------------------------------------------------------------------
%
% versione con disposizione delle informazioni sul grafico tarata per le
% immagini da inserire nella presentazione powerpoint con formato
% 1800 x 770 pixel scalato di un fattore 0.6 (1080 x 462 pixel)
%

function test2_analysis_ppt(filename, start_time, end_time)

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

	% fa partire il tempo da 0 per una migliore leggibilità del grafico
	time = time - time(1);

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
	if isempty(setpoint_change_idx)
		error('Errore: Non è stato rilevato alcun cambiamento del setpoint nel file CSV. Assicurarsi che il test sia stato eseguito correttamente.');
	end


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

	% individua il primo istante in cui la temperatura raggiunge o supera il nuovo setpoint
	cross_idx = find(temp_post_change >= setpoint2, 1);

	if isempty(cross_idx)
		% in caso di instabilità, non è possibile calcolare gli errori a regime
		ss_error_pos = NaN;
		ss_error_neg = NaN;
		ss_error_pos_idx = NaN;
		ss_error_neg_idx = NaN;
	else
		% estrapola solo la porzione di temperature dopo che il sistema si è stabilizzato
		steady_state_temp = temp_post_change(cross_idx:end);

		% calcola gli errori rispetto al nuovo setpoint
		errors = steady_state_temp - setpoint2;

		% individua l'errore massimo positivo e negativo
		[ss_error_pos,ss_error_pos_idx] = max(errors);
		[ss_error_neg,ss_error_neg_idx] = min(errors);

		% pulizia dei dati (se il max errore è negativo, lo poniamo a 0 per chiarezza)
		if ss_error_pos < 0, ss_error_pos = 0; ss_error_pos_idx = NaN; end
		if ss_error_neg > 0, ss_error_neg = 0; ss_error_neg_idx = NaN; end
	end

	% stampa dei risultati dell'analisi
	fprintf(' --- file analizzato: %s ---\n', filename);
	fprintf(' - setpoint iniziale: ........ %.2f °C - (exp. 32°C)\n', setpoint1);
	fprintf(' - setpoint finale: .......... %.2f °C - (exp. 36°C)\n', setpoint2);
	fprintf(' - settling time: ............ %.2f sec - (exp. < 900 sec)\n', settling_time);
	fprintf(' - overshoot: ................ +%.2f °C - (exp. < 2°C)\n', overshoot);
	fprintf(' - pos. steady-state error: .. +%.2f °C - (exp. < 0.5°C)\n', ss_error_pos);
	fprintf(' - neg. steady-state error: .. %.2f °C - (exp. > -0.5°C)\n\n', ss_error_neg);


	%% ----------------------- generazione del grafico ------------------------

	% crea una finestra grafica per stampare il grafico
	figure('Name', sprintf('Analisi test settling time, overshoot, steady-state error - %s', filename), 'Color', 'w', 'Position', [1200, 100, 1080, 462]);

	% crea il layout a due righe per i grafici
	t_layout = tiledlayout(20, 1, 'TileSpacing', 'loose', 'Padding', 'compact');

	% --- grafico con i dati di temperatura e setpoint ---
	ax1 = nexttile(t_layout, [15, 1]);
	grid on; hold on;

	% Definizione dei colori (esadecimali)
	col_segnale  = '#ff0000';
	col_setpoint = [0.8500 0.3250 0.0980];
	col_settling = [0.9290 0.6940 0.1250];
	col_peak     = [0.4940 0.1840 0.5560];
	col_band     = [0.4660 0.6740 0.1880];
	col_err	     = [0.4940 0.1840 0.5560];

	% traccia la risposta del sistema
	plot(time, temp, 'Color', col_segnale, 'LineStyle', '-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');

	% traccia il setpoint come linea tratteggiata nera
	plot(time, setpoint, 'Color', col_setpoint, 'LineStyle', '--', 'LineWidth', 1.5, 'DisplayName', 'Setpoint');

	% --- settling time ---
	% traccia le bande di assestamento di 0.5°C sopra e sotto il setpoint
	yline(setpoint2 + 0.5, 'Color', col_band, 'LineStyle', ':', 'LineWidth', 1.8, 'DisplayName', 'Limiti di temperatura (\pm0.5°C)');
	text(time(1) + 0.5 * (time(end) - time(1)), setpoint2 + 0.5, sprintf('Limite di temperatura superiore: %.2f °C', setpoint2 + 0.5), 'Color', col_band, 'VerticalAlignment', 'top', 'HorizontalAlignment', 'center');
	yline(setpoint2 - 0.5, 'Color', col_band, 'LineStyle', ':', 'LineWidth', 1.8, 'HandleVisibility', 'off');
	text(time(1) + 0.5 * (time(end) - time(1)), setpoint2 - 0.5, sprintf('Limite di temperatura inferiore: %.2f °C', setpoint2 - 0.5), 'Color', col_band, 'VerticalAlignment', 'bottom', 'HorizontalAlignment', 'center');

	% mostra la fine del tempo di assestamento sul grafico
	if ~isnan(settling_time)
		t_settling_abs = time(setpoint_change_idx + settling_idx - 1);
		y_settling_abs = temp_post_change(settling_idx);
		plot(t_settling_abs, y_settling_abs, 'Color', col_settling, 'Marker', 's', 'MarkerSize', 8, 'MarkerFaceColor', col_settling, 'HandleVisibility', 'off');
		plot([time(setpoint_change_idx), time(setpoint_change_idx)], ylim, 'Color', col_settling, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Istante di inizio del tempo di assestamento');
		plot([t_settling_abs, t_settling_abs], ylim, 'Color', col_settling, 'LineStyle', '-.', 'LineWidth', 1, 'DisplayName', 'Istante di fine del tempo di assestamento');

		x_offset = (time(end) - time(1)) * 0.005;
		text(time(setpoint_change_idx) + x_offset, temp(1) + 0.70 * (temp(end) - temp(1)), sprintf(' Istante di inizio del tempo\n di assestamento:\n %.2f secondi', time(setpoint_change_idx)), 'Color', col_settling, 'VerticalAlignment', 'middle', 'HorizontalAlignment', 'left'); %'BackgroundColor', 'w', 'Margin', 1);
		text(t_settling_abs + x_offset, temp(1) + 0.70 * (temp(end) - temp(1)), sprintf(' Istante di fine del tempo\n di assestamento:\n %.2f secondi', time(setpoint_change_idx + settling_idx - 1)), 'Color', col_settling, 'VerticalAlignment', 'middle', 'HorizontalAlignment', 'left'); %'BackgroundColor', 'w', 'Margin', 1);
	end

	% --- overshoot ---
	% individua il picco massimo della temperatura dopo il cambio di setpoint
	%if ~isnan(max_temp_post)
	%	t_peak = time(setpoint_change_idx + peak_idx - 1);
	%	y_peak = temp_post_change(peak_idx);
	%	plot(t_peak, y_peak, 'Color', col_peak, 'Marker', 'o', 'MarkerSize', 8, 'MarkerFaceColor', col_peak, 'HandleVisibility', 'off');
	%	text(t_peak, y_peak, sprintf('  Overshoot: %.02f°C', overshoot), 'Color', col_peak, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'bottom');
	%end

	% --- steady-state errors ---
	% individua l'errore massimo positivo a regime
	if ~isnan(ss_error_pos)
		yline(setpoint2 + ss_error_pos, 'Color', col_err, 'LineStyle', ':', 'LineWidth', 1.8, 'DisplayName', 'Errore massimo a regime');
		%plot(time(setpoint_change_idx + cross_idx + ss_error_pos_idx - 2), setpoint2 + ss_error_pos, 'Color', col_peak, 'Marker', 'o', 'MarkerSize', 8, 'MarkerFaceColor', col_peak, 'HandleVisibility', 'off');
		text(time(1) + 0.8 * (time(end) - time(1)), setpoint2 + ss_error_pos, sprintf('Errore massimo positivo a regime: +%.2f °C', ss_error_pos), 'Color', col_err, 'VerticalAlignment', 'bottom', 'HorizontalAlignment', 'center');
	end
	%% individua l'errore massimo negativo a regime
	if ~isnan(ss_error_neg)
		yline(setpoint2 + ss_error_neg, 'Color', col_err, 'LineStyle', ':', 'LineWidth', 1.8, 'HandleVisibility', 'off');
		%plot(time(setpoint_change_idx + cross_idx + ss_error_neg_idx - 2), setpoint2 + ss_error_neg, 'Color', col_peak, 'Marker', 'o', 'MarkerSize', 8, 'MarkerFaceColor', col_peak, 'HandleVisibility', 'off');
		text(time(1) + 0.8 * (time(end) - time(1)), setpoint2 + ss_error_neg, sprintf('Errore massimo negativo a regime: -%.2f °C', abs(ss_error_neg)), 'Color', col_err, 'VerticalAlignment', 'top', 'HorizontalAlignment', 'center');
	end

	% --- estetica e legenda ---
	ylabel('Temperatura (°C)');
	legend('Location', 'southeast');
	ylim(ax1, [31.7, 36.6]);
	ax1.YTick = floor(min(temp)) : 0.5 : ceil(max(max(temp), setpoint2 + 0.5));
	ax1.XTick = 0 : 50 : max(time);
	ax1.XMinorGrid = 'on';

	% --- grafico con i dati di PID, PWM e output della temperatura ---
	ax2 = nexttile(t_layout, [5, 1]);
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

	% esportazione del grafico come pdf
	exportgraphics(gcf, sprintf('/Users/giacomo/Desktop/%s.pdf', filename(1:end-4)), 'ContentType', 'vector');
end
