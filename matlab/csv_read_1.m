function csv_read_1(filename)
	% Se non viene specificato alcun argomento
    if nargin < 1 || isempty(filename)
        error('Uso corretto: csv_read <nome_file.csv>');
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
	opts = setvaropts(opts, 'Timestamp', 'InputFormat', 'yyyy-MM-dd HH:mm:ss');

	% Lettura della tabella
	data = readtable(filename, opts);

	% Estrazione del tempo
	time = data.Timestamp;

	% Creazione della figura
	figure('Name', 'Monitoraggio Temperatura e Umidità', 'Color', 'w', 'Position', [100, 100, 900, 600]);
	tiledlayout(2, 1, 'TileSpacing', 'compact', 'Padding', 'compact');

	% ----------------------------------------------------
	% Riquadro 1: Controllo Temperatura
	% ----------------------------------------------------
	ax1 = nexttile;

	% Asse sinistro: Valori di temperatura
	yyaxis left
	plot(time, data.Temperature, 'r-', 'LineWidth', 1.5, 'DisplayName', 'Temp. Misurata');
	hold on;
	plot(time, data.Setpoint_Temperature, 'r--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint Temp.');
	ylabel('Temperatura (°C)');
	grid on;

	% Asse destro: Stato attuatore (Heater)
	yyaxis right
	stairs(time, data.Heater, 'm-', 'LineWidth', 1.3, 'DisplayName', 'Heater');
	ylabel('Stato Heater');
	ylim([-0.1, 1.1]);
	yticks([0 1]);
	yticklabels({'OFF', 'ON'});

	title('Controllo Temperatura');
	legend('Location', 'best');
	hold off;

	% ----------------------------------------------------
	% Riquadro 2: Controllo Umidità
	% ----------------------------------------------------
	ax2 = nexttile;

	% Asse sinistro: Valori di umidità
	yyaxis left
	plot(time, data.Humidity, 'b-', 'LineWidth', 1.5, 'DisplayName', 'Umidità Misurata');
	hold on;
	plot(time, data.Setpoint_Humidity, 'b--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint Umidità');
	ylabel('Umidità (%)');
	grid on;

	% Asse destro: Stato attuatore (Humidifier)
	yyaxis right
	stairs(time, data.Humidifier, 'c-', 'LineWidth', 1.3, 'DisplayName', 'Humidifier');
	ylabel('Stato Humidifier');
	ylim([-0.1, 1.1]);
	yticks([0 1]);
	yticklabels({'OFF', 'ON'});

	title('Controllo Umidità');
	xlabel('Data e Ora');
	legend('Location', 'best');
	hold off;

	% Sincronizzazione dello zoom e del pan sull'asse temporale
	linkaxes([ax1, ax2], 'x');

end
