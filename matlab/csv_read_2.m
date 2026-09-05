function csv_read_2(filename, showPlot)
	% Se non viene specificato alcun argomento
	if nargin < 1 || isempty(filename)
		error('Uso corretto: csv_read <nome_file.csv>');
	end

	% Se non viene specificato il parametro dei grafici, mostra entrambi (3)
	if nargin < 2 || isempty(showPlot)
		showPlot = 3;
	end

	filename = char(filename);

	% Aggiunge in automatico .csv se viene omesso dall'utente
	if ~isfile(filename) && ~endsWith(filename, '.csv', 'IgnoreCase', true)
		if isfile([filename, '.csv'])
			filename = [filename, '.csv'];
		end
	end

	% Controllo esistenza file
	if ~isfile(filename)
		error('Errore: Il file "%s" non è stato trovato nella cartella corrente.', filename);
	end

	% Opzioni di importazione per preservare i nomi originali delle colonne
	opts = detectImportOptions(filename);
	opts.VariableNamingRule = 'preserve';

	% Lettura della tabella
	data = readtable(filename, opts);

	% Estrazione del tempo
	time = data.Time_Seconds;

	% Creazione della figura
	if showPlot > 0
		figure('Name', 'Analisi Dati CSV Arduino', 'Color', 'w', 'Position', [100, 100, 1000, 700]);

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

		% ====================================================
		% Riquadro 1: Temperatura
		% ====================================================
		if showPlot == 1 || showPlot == 3
			ax1 = nexttile(t_layout);
			title('Controllo Temperatura');
			grid on; hold on;

			% Asse Sinistro (Gradi)
			yyaxis left;
			plot(time, data.Temperature, 'r-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');
			plot(time, data.Setpoint_Temperature, 'r--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint');
			plot(time, data.T_min_thld, 'r:', 'LineWidth', 1, 'DisplayName', 'T_{min_thld}');
			plot(time, data.T_max_thld, 'r:', 'LineWidth', 1, 'DisplayName', 'T_{max_thld}');
			ylabel('Temperatura (°C)');

			% Asse Destro (Segnali di controllo)
			yyaxis right;
			plot(time, data.T_PWM, 'k-', 'LineWidth', 1.5, 'DisplayName', 'PWM_{T}');
			plot(time, data.T_output, 'k-', 'LineWidth', 1.5, 'DisplayName', 'Output_{T}');
			plot(time, data.T_PID_p, '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', 'P_{PID}');
			plot(time, data.T_PID_i, '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', 'I_{PID}');
			plot(time, data.T_PID_d, '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', 'D_{PID}');
			plot(time, data.T_PID_out, '-.', 'Color', c_Out, 'LineWidth', 1, 'DisplayName', 'Out_{PID}');
			ylabel('Segnali (PWM/PID)');

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

			% Asse Sinistro (Percentuale)
			yyaxis left;
			plot(time, data.Humidity, 'b-', 'LineWidth', 1.5, 'DisplayName', 'Misurazione');
			plot(time, data.Setpoint_Humidity, 'b--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint');
			plot(time, data.RH_min_thld, 'b:', 'LineWidth', 1, 'DisplayName', 'RH_{min_thld}');
			plot(time, data.RH_max_thld, 'b:', 'LineWidth', 1, 'DisplayName', 'RH_{max_thld}');
			ylabel('Umidità (%)');

			% Asse Destro (Segnali di controllo)
			yyaxis right;
			plot(time, data.RH_PWM, 'k-', 'LineWidth', 1.5, 'DisplayName', 'PWM_{RH}');
			plot(time, data.RH_output, 'k-', 'LineWidth', 1.5, 'DisplayName', 'Output_{RH}');
			plot(time, data.RH_PID_p, '--', 'Color', c_P, 'LineWidth', 1, 'DisplayName', 'P_{PID}');
			plot(time, data.RH_PID_i, '--', 'Color', c_I, 'LineWidth', 1, 'DisplayName', 'I_{PID}');
			plot(time, data.RH_PID_d, '--', 'Color', c_D, 'LineWidth', 1, 'DisplayName', 'D_{PID}');
			plot(time, data.RH_PID_out, '-.', 'Color', c_Out, 'LineWidth', 1.2, 'DisplayName', 'Out_{PID}');
			ylabel('Segnali (PWM/PID)');

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
