% -----------------------------------------------------------------------------
% --------------- csv_read.m - Lettura dei dati da un file CSV ----------------
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
%  - showPlot:
%    permette di scegliere che parametri visualizzare nel grafico in base al
%    valore passato come parametro:
%     - 0 = nessun grafico
%     - 1 = solo il grafico per la temperatura
%     - 2 = solo il grafico per l'umidità
%     - 3 = entrambi i grafici per la temperatura e l'umidità (default)
%

function csv_read(filename, showPlot)
	%% ------ parsing dei parametri e assegnazione dei valori di default ------

	% nome del file CSV da leggere (obbligatorio)
	if nargin < 1 || isempty(filename)
		error('Uso corretto: csv_read <nome_file.csv>');
	end

	% tipo di grafico da visualizzare (default = 3 - entrambi i grafici)
	if nargin < 2 || isempty(showPlot)
		showPlot = 3;
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

	% estrae il vettore temporale
	time = data.Time_Seconds;

	%% -------------------------- setup dei grafici ---------------------------
	if showPlot > 0

		% crea una finestra grafica per stampare il grafico
		figure('Name', 'Analisi Dati CSV Arduino', 'Color', 'w', 'Units', 'centimeters', 'Position', [40, 7, 16, 21]);

		% crea il layout con 2 o 4 righe in base ai grafici da mostrare
		if showPlot == 3
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
		if showPlot == 1 || showPlot == 3

			% --- grafico con i dati di temperatura e setpoint ---
			% impostazioni di base
			ax1 = nexttile(t_layout);
			grid on; hold on;

			% set dei dati da mostrare (temperatura e setpoint)
			plot(time, data.Temperature, 'r-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');
			plot(time, data.Setpoint_Temperature, 'r--', 'LineWidth', 1, 'DisplayName', 'Setpoint');
			if ~all(isnan(data.T_min_thld))
				plot(time, data.T_min_thld, 'r:', 'LineWidth', 1, 'DisplayName', 'T_{min-thld}');
			end
			if ~all(isnan(data.T_max_thld))
				plot(time, data.T_max_thld, 'r:', 'LineWidth', 1, 'DisplayName', 'T_{max-thld}');
			end

			% preferenze di visualizzazione
			ylabel('Temperatura (°C)'); % etichetta asse y
			legend('Location', 'southeast'); % legenda
			ax1.XTick = 0 : 50 : max(time); % impostazioni della griglia
			ax1.YMinorGrid = 'on';

			% --- grafico con i dati del PID (P, I, D, Output) e PWM ---
			% impostazioni di base
			ax2 = nexttile(t_layout);
			grid on; hold on;

			% set di dati da mostrare (segnali pid e output)
			if ~all(isnan(data.T_PID_p))
				plot(time, data.T_PID_p, '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', 'P_{PID}');
			end
			if ~all(isnan(data.T_PID_i))
				plot(time, data.T_PID_i, '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', 'I_{PID}');
			end
			if ~all(isnan(data.T_PID_d))
				plot(time, data.T_PID_d, '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', 'D_{PID}');
			end
			if ~all(isnan(data.T_PID_out))
				plot(time, data.T_PID_out, '-.', 'Color', c_Out, 'LineWidth', 1, 'DisplayName', 'Out_{PID}');
			end
			if ~all(isnan(data.T_PWM))
				plot(time, data.T_PWM, 'k-', 'LineWidth', 1.5, 'DisplayName', 'Duty cycle_{PWM}');
			end
			if ~all(isnan(data.T_output))
				plot(time, data.T_output, 'k-', 'LineWidth', 1.5, 'DisplayName', 'Output');
			end

			% preferenze di visualizzazione
			ylabel('Controllo (Duty cycle/PID)'); % etichetta asse y
			legend('Location', 'northeast'); % legenda
			ax2.XTick = 0 : 50 : max(time);
			ax2.YMinorGrid = 'on';

			% --- linking degli assi x ---
			linkaxes([ax1, ax2], 'x');
		end

		% --- grafici per le misurazioni di umidità ---
		if showPlot == 2 || showPlot == 3

			% impostazioni di base
			ax2 = nexttile(t_layout);
			title('Controllo Umidità');
			grid on; hold on;

			% set di dati per l'asse sinistro (umidità)
			yyaxis left;
			ylabel('Umidità (%)');
			plot(time, data.Humidity, 'b-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');
			plot(time, data.Setpoint_Humidity, 'b--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint');
			plot(time, data.RH_min_thld, 'b:', 'LineWidth', 1, 'DisplayName', 'RH_{min-thld}');
			plot(time, data.RH_max_thld, 'b:', 'LineWidth', 1, 'DisplayName', 'RH_{max-thld}');

			% set di dati per l'asse destro (segnali di controllo)
			yyaxis right;
			ylabel('Segnali (PWM/PID)');
			plot(time, data.RH_PWM, 'k-', 'LineWidth', 1.5, 'DisplayName', 'PWM_{RH}');
			plot(time, data.RH_output, 'k-', 'LineWidth', 1.5, 'DisplayName', 'Output_{RH}');
			plot(time, data.RH_PID_p, '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', 'P_{PID}');
			plot(time, data.RH_PID_i, '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', 'I_{PID}');
			plot(time, data.RH_PID_d, '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', 'D_{PID}');
			plot(time, data.RH_PID_out, '-.', 'Color', c_Out, 'LineWidth', 1.2, 'DisplayName', 'Out_{PID}');

			% aggiunge la legenda al grafico
			legend('Location', 'westoutside');

			% adatta automaticamente l'asse x al numero di punti visualizzati
			ax2.XLimMode = 'auto';
		end

		% sincronizza gli assi x dei due grafici se entrambi sono stati creati
		if showPlot == 3
			linkaxes([ax1, ax2], 'x');
		end

		% etichetta per l'asse x
		xlabel(t_layout, 'Tempo trascorso (secondi)');
	end
end
