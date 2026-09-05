function csv_read_3(filenames, showPlot)
	% Se non viene specificato alcun argomento
	if nargin < 1 || isempty(filenames)
		error('Uso corretto: csv_read_2({''file1.csv'', ''file2.csv''}) oppure csv_read_2(''file1.csv'')');
	end

	% Se non viene specificato il parametro dei grafici, mostra entrambi (3)
	if nargin < 2 || isempty(showPlot)
		showPlot = 3;
	end

	% --- 1. GESTIONE INPUT MULTIPLI ---
	% Se l'utente passa una singola stringa, la trasformiamo in un cell array
	if ischar(filenames)
		filenames = {filenames};
	elseif isstring(filenames)
		filenames = cellstr(filenames);
	end
	numFiles = length(filenames);

	% --- 2. LETTURA DATI MULTIPLI ---
	data_list = cell(numFiles, 1);
	for i = 1:numFiles
		fname = char(filenames{i});

		% Aggiunge in automatico .csv se viene omesso dall'utente
		if ~isfile(fname) && ~endsWith(fname, '.csv', 'IgnoreCase', true)
			if isfile([fname, '.csv'])
				fname = [fname, '.csv'];
			end
		end

		% Controllo esistenza file
		if ~isfile(fname)
			error('Errore: Il file "%s" non è stato trovato nella cartella corrente.', fname);
		end

		% Opzioni di importazione per preservare i nomi originali delle colonne
		opts = detectImportOptions(fname);
		opts.VariableNamingRule = 'preserve';

		% Lettura della tabella e salvataggio nella lista
		data_list{i} = readtable(fname, opts);
	end

	% --- 3. ALLINEAMENTO TEMPORALE (SHIFT a 32.5°C) ---
	time_shifted = cell(numFiles, 1);
	targetTemp = 32.5;
	t_ref = 0; % Tempo di riferimento (basato sul primo file)

	for i = 1:numFiles
		time_raw = data_list{i}.Time_Seconds;
		temp_raw = data_list{i}.Temperature;

		% Trova il primo indice in cui la temp raggiunge o supera i 32.5°C
		idx = find(temp_raw >= targetTemp, 1);

		if isempty(idx)
			warning('Temperatura di %.1f°C non raggiunta nel file %s. Nessuno shift applicato.', targetTemp, filenames{i});
			offset = 0;
		else
			t_reach = time_raw(idx);
			if i == 1
				t_ref = t_reach; % Usiamo il tempo del primo file come ancoraggio
				offset = 0;
			else
				% L'offset è la differenza tra quando ci arriva il file 1 e quando ci arriva il file i
				offset = t_ref - t_reach;
			end
		end
		% Applica lo shift a tutto l'asse dei tempi di questo specifico file
		time_shifted{i} = time_raw + offset;
	end

	% --- 4. CREAZIONE DELLA FIGURA E PLOT ---
	if showPlot > 0
		figure('Name', 'Analisi Dati CSV Arduino - Confronto', 'Color', 'w', 'Position', [100, 100, 1000, 700]);

		if showPlot == 3
			t_layout = tiledlayout(2, 1, 'TileSpacing', 'compact', 'Padding', 'compact');
		else
			t_layout = tiledlayout(1, 1, 'TileSpacing', 'compact', 'Padding', 'compact');
		end

		% Colori personalizzati per il PID
		c_P = [0.8500 0.3250 0.0980];
		c_I = [0.9290 0.6940 0.1250];
		c_D = [0.4940 0.1840 0.5560];
		c_Out = [0.4660 0.6740 0.1880];

		% Per variare lo stile della linea e distinguere i file (File1 solido, File2 puntinato, ecc.)
		% Se preferisci averli identici, puoi forzare styleList = {'-', '-', '-', ...}
		styleList = {'-', ':', '-.', '--'};

		% ====================================================
		% Riquadro 1: Temperatura
		% ====================================================
		if showPlot == 1 || showPlot == 3
			ax1 = nexttile(t_layout);
			title('Controllo Temperatura');
			grid on; hold on;

			for i = 1:numFiles
				data = data_list{i};
				time = time_shifted{i};

				% Estrae il nome e lo forza a carattere continuo per evitare errori
				[~, f_name, ~] = fileparts(char(filenames{i}));
				st = char(styleList{mod(i-1, length(styleList)) + 1});

				% Asse Sinistro (Gradi)
				yyaxis left;
				plot(time, data.Temperature, 'Color', 'r', 'LineStyle', st, 'LineWidth', 1.5, 'DisplayName', sprintf('Misurazione (%s)', f_name));
				%plot(time, data.Setpoint_Temperature, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 1.2, 'DisplayName', sprintf('Setpoint (%s)', f_name));
				ylabel('Temperatura (°C)');

				% Asse Destro (Segnali di controllo)
				yyaxis right;
				plot(time, data.T_PWM, 'Color', 'k', 'LineStyle', st, 'LineWidth', 1.5, 'DisplayName', sprintf('PWM_{T} (%s)', f_name));
				plot(time, data.T_PID_p, 'LineStyle', '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', sprintf('P_{PID} (%s)', f_name));
				plot(time, data.T_PID_i, 'LineStyle', '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', sprintf('I_{PID} (%s)', f_name));
				plot(time, data.T_PID_d, 'LineStyle', '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', sprintf('D_{PID} (%s)', f_name));
				%plot(time, data.T_PID_out, 'LineStyle', '-.', 'Color', c_Out, 'LineWidth', 1, 'DisplayName', sprintf('Out_{PID} (%s)', f_name));
				ylabel('Segnali (PWM/PID)');
			end
			legend('Location', 'westoutside');
			ax1.XLimMode = 'auto';
		end

		% ====================================================
		% Riquadro 2: Umidità
		% ====================================================
		if showPlot == 2 || showPlot == 3
			ax2 = nexttile(t_layout);
			title('Controllo Umidità');
			grid on; hold on;

			for i = 1:numFiles
				data = data_list{i};
				time = time_shifted{i};

				[~, f_name, ~] = fileparts(char(filenames{i}));
				st = char(styleList{mod(i-1, length(styleList)) + 1});

				% Asse Sinistro (Percentuale)
				yyaxis left;
				plot(time, data.Humidity, 'Color', 'b', 'LineStyle', st, 'LineWidth', 1.5, 'DisplayName', sprintf('Misurazione (%s)', f_name));
				plot(time, data.Setpoint_Humidity, 'Color', 'b', 'LineStyle', '--', 'LineWidth', 1.2, 'DisplayName', sprintf('Setpoint (%s)', f_name));
				ylabel('Umidità (%)');

				% Asse Destro (Segnali di controllo)
				yyaxis right;
				plot(time, data.RH_PWM, 'Color', 'k', 'LineStyle', st, 'LineWidth', 1.5, 'DisplayName', sprintf('PWM_{RH} (%s)', f_name));
				plot(time, data.RH_PID_p, 'LineStyle', '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', sprintf('P_{PID} (%s)', f_name));
				plot(time, data.RH_PID_i, 'LineStyle', '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', sprintf('I_{PID} (%s)', f_name));
				plot(time, data.RH_PID_d, 'LineStyle', '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', sprintf('D_{PID} (%s)', f_name));
				plot(time, data.RH_PID_out, 'LineStyle', '-.', 'Color', c_Out, 'LineWidth', 1.2, 'DisplayName', sprintf('Out_{PID} (%s)', f_name));
				ylabel('Segnali (PWM/PID)');
			end
			legend('Location', 'westoutside');
			ax2.XLimMode = 'auto';
		end

		% Sincronizzazione Zoom
		if showPlot == 3
			linkaxes([ax1, ax2], 'x');
		end

		xlabel(t_layout, 'Tempo trascorso (secondi)');
	end
end
