% -----------------------------------------------------------------------------
% ----------- plot_csv.m - Stampa del grafico a partire da dati CSV -----------
% -----------------------------------------------------------------------------
%
% La seguente funzione legge i dati da un file CSV generato dalla funzione
% serial_read.m, estrae le informazioni principali e genera un grafico con
% i dati contenuti nel file.
%
% La funzione accetta i seguenti parametri da riga di comando:
%  - filename:
%    nome del file CSV da leggere (obbligatorio)
%
%  - plot_type:
%    permette di scegliere che parametri visualizzare nel grafico in base al
%    valore passato come parametro:
%     - 0 = nessun grafico
%     - 1 = solo il grafico per la temperatura
%     - 2 = solo il grafico per l'umidità
%     - 3 = entrambi i grafici per la temperatura e l'umidità (default)
%
%  - start_time:
%    indice di inizio per l'analisi dei dati (opzionale, default = 1)
%
%  - end_time:
%    indice di fine per l'analisi dei dati (opzionale, default = ultimo)
%

function plot_csv(filename, plot_type, start_time, end_time)
	%% ------ parsing dei parametri e assegnazione dei valori di default ------

	% nome del file CSV da leggere (obbligatorio)
	if nargin < 1 || isempty(filename)
		error('Uso corretto: plot <nome_file.csv> [plot_type]');
	end

	% tipo di grafico da visualizzare (default = 3 - entrambi i grafici)
	if nargin < 2 || isempty(plot_type)
		plot_type = 3;
	end

	% indice di inizio per l'analisi dei dati (default = 1)
	if nargin < 3 || isempty(start_time)
		start_time = 1;
	end

	% indice di fine per l'analisi dei dati (default = ultimo dato)
	if nargin < 4 || isempty(end_time)
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

	% estrae il vettore temporale
	time = data.Time_Seconds(start_time:end_time);

		% fa partire il tempo da 0 per una migliore leggibilità del grafico
	time = time - time(1);

	%% -------------------------- setup dei grafici ---------------------------
	if plot_type > 0

		% crea una finestra grafica per stampare il grafico
		figure('Name', 'Analisi Dati CSV Arduino', 'Color', 'w', 'Position', [1200, 100, 1730, 720]);

		% crea il layout con 2 o 4 righe in base ai grafici da mostrare
		if plot_type == 3
			t_layout = tiledlayout(4, 1, 'TileSpacing', 'compact', 'Padding', 'compact');
		else
			t_layout = tiledlayout(2, 1, 'TileSpacing', 'compact', 'Padding', 'compact');
		end

		% colori personalizzati per i dati del PID
		c_P = [0.8500 0.3250 0.0980];
		c_I = [0.9290 0.6940 0.1250];
		c_D = [0.4940 0.1840 0.5560];
		c_Out = [0.4660 0.6740 0.1880];

		% --- grafici per le misurazioni di temperatura ---
		if plot_type == 1 || plot_type == 3

			% --- grafico con i dati di temperatura e setpoint ---
			ax1 = nexttile(t_layout);
			grid on; hold on;

			% set dei dati da mostrare (temperatura e setpoint)
			plot(time, data.Temperature(start_time:end_time), 'r-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');
			plot(time, data.Setpoint_Temperature(start_time:end_time), 'r--', 'LineWidth', 1, 'DisplayName', 'Setpoint');
			if ismember('T_min_thld', data.Properties.VariableNames) && ~all(isnan(data.T_min_thld(start_time:end_time)))
				plot(time, data.T_min_thld(start_time:end_time), 'r:', 'LineWidth', 1, 'DisplayName', 'THLD_{min}');
			end
			if ismember('T_max_thld', data.Properties.VariableNames) && ~all(isnan(data.T_max_thld(start_time:end_time)))
				plot(time, data.T_max_thld(start_time:end_time), 'r:', 'LineWidth', 1, 'DisplayName', 'THLD_{max}');
			end

			% preferenze di visualizzazione
			ylabel('Temperatura (°C)'); % etichetta asse y
			legend('Location', 'southeast'); % legenda
			ax1.XTick = 0 : 50 : max(time); % impostazioni della griglia
			ax1.YMinorGrid = 'on';

			% --- grafico con i dati di PID, PWM e output della temperatura ---
			ax2 = nexttile(t_layout);
			grid on; hold on;

			% set di dati da mostrare (segnali pid e output)
			if ismember('T_PID_p', data.Properties.VariableNames) && ~all(isnan(data.T_PID_p(start_time:end_time)))
				plot(time, data.T_PID_p(start_time:end_time), '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', 'P_{PID}');
			end
			if ismember('T_PID_i', data.Properties.VariableNames) && ~all(isnan(data.T_PID_i(start_time:end_time)))
				plot(time, data.T_PID_i(start_time:end_time), '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', 'I_{PID}');
			end
			if ismember('T_PID_d', data.Properties.VariableNames) && ~all(isnan(data.T_PID_d(start_time:end_time)))
				plot(time, data.T_PID_d(start_time:end_time), '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', 'D_{PID}');
			end
			if ismember('T_PID_out', data.Properties.VariableNames) && ~all(isnan(data.T_PID_out(start_time:end_time)))
				plot(time, data.T_PID_out(start_time:end_time), '-.', 'Color', c_Out, 'LineWidth', 1, 'DisplayName', 'Out_{PID}');
			end
			if ismember('T_PWM', data.Properties.VariableNames) && ~all(isnan(data.T_PWM(start_time:end_time)))
				plot(time, data.T_PWM(start_time:end_time), 'k-', 'LineWidth', 1.5, 'DisplayName', 'Duty cycle_{PWM}');
			end
			if ismember('T_output', data.Properties.VariableNames) && ~all(isnan(data.T_output(start_time:end_time)))
				plot(time, data.T_output(start_time:end_time), 'k-', 'LineWidth', 1.5, 'DisplayName', 'Output');
			end

			% preferenze di visualizzazione
			ylabel('Output controllore'); % etichetta asse y
			legend('Location', 'northeast'); % legenda
			ax2.XTick = 0 : 50 : max(time);
			ax2.YMinorGrid = 'on';

			% --- linking degli assi x ---
			linkaxes([ax1, ax2], 'x');
			xlim(ax2, [time(1), time(end)]);
		end

		% --- grafici per le misurazioni di umidità ---
		if plot_type == 2 || plot_type == 3

			% impostazioni di base
			ax3 = nexttile(t_layout);
			grid on; hold on;

			% set di dati per l'asse sinistro (umidità)
			plot(time, data.Humidity(start_time:end_time), 'b-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');
			plot(time, data.Setpoint_Humidity(start_time:end_time), 'b--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint');
			if ismember('RH_min_thld', data.Properties.VariableNames) && ~all(isnan(data.RH_min_thld(start_time:end_time)))
				plot(time, data.RH_min_thld(start_time:end_time), 'b:', 'LineWidth', 1, 'DisplayName', 'THLD_{min}');
			end
			if ismember('RH_max_thld', data.Properties.VariableNames) && ~all(isnan(data.RH_max_thld(start_time:end_time)))
				plot(time, data.RH_max_thld(start_time:end_time), 'b:', 'LineWidth', 1, 'DisplayName', 'THLD_{max}');
			end

			% preferenze di visualizzazione
			ylabel('Umidità (%)'); % etichetta asse y
			legend('Location', 'southeast'); % legenda
			ax3.XTick = 0 : 50 : max(time); % impostazioni della griglia
			ax3.YMinorGrid = 'on';

			% --- grafico con i dati di PID, PWM e output dell'umidità ---
			ax4 = nexttile(t_layout);
			grid on; hold on;

			% set di dati da mostrare (segnali pid e output)
			if ismember('RH_PID_p', data.Properties.VariableNames) && ~all(isnan(data.RH_PID_p(start_time:end_time)))
				plot(time, data.RH_PID_p(start_time:end_time), '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', 'P_{PID}');
			end
			if ismember('RH_PID_i', data.Properties.VariableNames) && ~all(isnan(data.RH_PID_i(start_time:end_time)))
				plot(time, data.RH_PID_i(start_time:end_time), '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', 'I_{PID}');
			end
			if ismember('RH_PID_d', data.Properties.VariableNames) && ~all(isnan(data.RH_PID_d(start_time:end_time)))
				plot(time, data.RH_PID_d(start_time:end_time), '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', 'D_{PID}');
			end
			if ismember('RH_PID_out', data.Properties.VariableNames) && ~all(isnan(data.RH_PID_out(start_time:end_time)))
				plot(time, data.RH_PID_out(start_time:end_time), '-.', 'Color', c_Out, 'LineWidth', 1.2, 'DisplayName', 'Out_{PID}');
			end
			if ismember('RH_PWM', data.Properties.VariableNames) && ~all(isnan(data.RH_PWM(start_time:end_time)))
				plot(time, data.RH_PWM(start_time:end_time), 'k-', 'LineWidth', 1.5, 'DisplayName', 'Duty cycle_{PWM}');
			end
			if ismember('RH_output', data.Properties.VariableNames) && ~all(isnan(data.RH_output(start_time:end_time)))
				plot(time, data.RH_output(start_time:end_time), 'k-', 'LineWidth', 1.5, 'DisplayName', 'Output');
			end

			% preferenze di visualizzazione
			ylabel('Output controllore'); % etichetta asse y
			legend('Location', 'northeast'); % legenda
			ax4.XTick = 0 : 50 : max(time);
			ax4.YMinorGrid = 'on';

			% --- linking degli assi x ---
			linkaxes([ax3, ax4], 'x');
			xlim(ax1, [time(1), time(end)]);
		end

		% sincronizza gli assi x dei due grafici se entrambi sono stati creati
		if plot_type == 3
			linkaxes([ax1, ax2], 'x');
		end

		% etichetta per l'asse x
		xlabel(t_layout, 'Tempo trascorso (secondi)');
	end
end
