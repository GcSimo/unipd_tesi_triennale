function plot_info(filename, variabile)
    % PLOT_INFO Legge dati PID da un CSV strutturato, calcola metriche e genera grafico.
    %   plot_info(filename) analizza di default la 'Temperature'
    %   plot_info(filename, 'Humidity') analizza la colonna 'Humidity'

	if nargin < 1 || isempty(filename)
		error('Uso corretto: csv_read <nome_file.csv>');
	end
    if nargin < 2
        variabile = 'Temperature'; % Variabile di default
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

	% 1. Importa i dati dal CSV usando le intestazioni delle colonne
	T = readtable(filename);

    % Verifica che la variabile esista nel file
    if ~ismember(variabile, T.Properties.VariableNames)
        error('La colonna "%s" non esiste nel file CSV.', variabile);
    end

    t = T.Time_Seconds;  % Vettore temporale
    y = T.(variabile);   % Vettore della risposta (es. Temperature o Humidity)

    % Estrae automaticamente il setpoint associato alla variabile
    nome_setpoint = sprintf('Setpoint_%s', variabile);
    if ismember(nome_setpoint, T.Properties.VariableNames)
        valore_di_riferimento = T.(nome_setpoint)(end); % Usa l'ultimo valore impostato
    else
        valore_di_riferimento = y(end);
        warning('Colonna %s non trovata. Uso l''ultimo valore letto come riferimento.', nome_setpoint);
    end

    % 2. Calcola tutte le metriche
    metriche = stepinfo(y, t, valore_di_riferimento);

    % Stampa i risultati nella Command Window
    disp('-----------------------------------------');
    fprintf('Risultati test PID (%s) per file: %s\n', variabile, filename);
    disp('-----------------------------------------');
    disp(metriche);

    % 3. Creazione del grafico
    figure('Name', sprintf('Analisi PID: %s - %s', variabile, filename), 'Color', 'w');
    plot(t, y, 'b-', 'LineWidth', 1.5);
    hold on;
    grid on;

    % --- Linea del Setpoint e Bande di Tolleranza (Assestamento) ---
    yline(valore_di_riferimento, 'k--', 'Setpoint', 'LineWidth', 1.5, 'LabelHorizontalAlignment', 'left');

    tolleranza = 0.02; % Banda di assestamento di default del 2%
    banda_sup = valore_di_riferimento * (1 + tolleranza);
    banda_inf = valore_di_riferimento * (1 - tolleranza);
    yline(banda_sup, 'g:', 'LineWidth', 1);
    yline(banda_inf, 'g:', 'LineWidth', 1);

    % --- Overshoot e Tempo di Picco ---
    t_peak = metriche.PeakTime;
    y_peak = metriche.Peak;
    plot(t_peak, y_peak, 'ro', 'MarkerSize', 8, 'MarkerFaceColor', 'r');
    text(t_peak, y_peak + 0.02 * abs(valore_di_riferimento), sprintf(' Overshoot: %.1f%%', metriche.Overshoot), 'Color', 'r', 'VerticalAlignment', 'bottom');

    % --- Settling Time ---
    t_sett = metriche.SettlingTime;
    if ~isnan(t_sett)
        [~, idx_sett] = min(abs(t - t_sett));
        plot(t_sett, y(idx_sett), 'gs', 'MarkerSize', 8, 'MarkerFaceColor', 'g');
        xline(t_sett, 'g--', 'Settling Time', 'LabelVerticalAlignment', 'bottom');
    end

    % --- Rise Time (Evidenziamo il passaggio al 90%) ---
    y_90 = valore_di_riferimento * 0.90;
    idx_90 = find(y >= y_90, 1);
    if ~isempty(idx_90)
        plot(t(idx_90), y(idx_90), 'md', 'MarkerSize', 8, 'MarkerFaceColor', 'm');
        text(t(idx_90), y(idx_90) - 0.02 * abs(valore_di_riferimento), sprintf(' Rise Time: %.2fs', metriche.RiseTime), 'Color', 'm', 'VerticalAlignment', 'top');
    end

    % --- Estetica e Legenda ---
    title(sprintf('Analisi Prestazioni Controllore PID: %s', variabile));
    xlabel('Tempo (secondi)');
    ylabel(variabile);
    legend('Risposta del Sistema', 'Setpoint', '+2% Tolleranza', '-2% Tolleranza', ...
           'Picco (Overshoot)', 'Settling Time', '90% Rise Time', 'Location', 'best');
    hold off;
end
