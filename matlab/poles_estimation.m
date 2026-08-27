function model_estimation(filename, input_ordine)
    % Se non viene specificato il nome del file
    if nargin < 1 || isempty(filename)
        error('Uso corretto: model_estimation <nome_file.csv> [ordine]');
    end

    % Gestione del parametro ordine opzionale
    if nargin < 2 || isempty(input_ordine)
        ordine = 3; % Valore di default
    else
        % Converte l'input (che da riga di comando arriva come stringa) in numero
        if ischar(input_ordine) || isstring(input_ordine)
            ordine = str2double(input_ordine);
        else
            ordine = input_ordine;
        end

        % Verifica che sia un numero intero positivo valido
        if isnan(ordine) || ordine < 1 || mod(ordine, 1) ~= 0
            error('L''ordine deve essere un numero intero positivo.');
        end
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

    % 1. Importazione della tabella dal CSV
    opts = detectImportOptions(filename);
    opts.VariableNamingRule = 'preserve';
    data = readtable(filename, opts);

    % 2. Vettore temporale e passo di campionamento Ts
    t_datetime = datetime(data.Timestamp, 'InputFormat', 'yyyy-MM-dd HH:mm:ss');
    t = seconds(t_datetime - t_datetime(1));
    Ts = mean(diff(t));

    % 3. Segnale centrato rispetto al setpoint di equilibrio
    T_inf = 30.5;
    y = data.Temperature - T_inf;

    % 4. Stima di un sistema del 3° ordine (3 poli) con Prony
    [b, a] = prony(y, 0, ordine);

    % Poli discreti e continui
    poli_z = roots(a);
    poli_s = log(poli_z) / Ts;

    % Estrazione parametri fisici
    sigma = real(poli_s);
    omega = abs(imag(poli_s));

    % 5. Calcolo dei coefficienti c_i (Ampiezze modali)
    % Risoluzione del sistema ai minimi quadrati: V * c = y
    % dove V è la matrice di Vandermonde V(k, i) = z_i^k
    N = length(y);
    V = zeros(N, ordine);
    for col = 1:ordine
        % Indici k = 0, 1, ..., N-1
        V(:, col) = poli_z(col).^(0 : N-1)';
    end

    % V \ y calcola i coefficienti c_i
    c = V \ y;

    % 6. Ricostruzione dell'evoluzione libera teorica
    % y_stimato(t) = T_inf + sum( c_i * exp(lambda_i * t) )
    y_stimato = zeros(size(t));
    for i = 1:ordine
        y_stimato = y_stimato + real(c(i) * exp(poli_s(i) * t));
    end
    T_stimata = y_stimato + T_inf;

    % 7. Stampa a schermo formattata
    fprintf('\n--- RISULTATI STIMA (Ordine %d) ---\n', ordine);
    for i = 1:length(poli_s)
        fprintf('Polo %d: lambda = %.4f %s %.4fj\n', i, real(poli_s(i)), ...
            char(sign(imag(poli_s(i)))*'+' + (imag(poli_s(i))==0)*' '), abs(imag(poli_s(i))));
        fprintf('   Decadimento (sigma): %.4f s^-1\n', sigma(i));
        fprintf('   Pulsazione (omega):  %.4f rad/s\n', omega(i));
        fprintf('   Ampiezza (c_%d):      %.4f\n', i, real(c(i)));
    end
    fprintf('----------------------------------\n\n');

    % =====================================================================
    % 8. GRAFICI: PIANO COMPLESSO S ED EVOLUZIONE NEL TEMPO
    % =====================================================================

    figure('Name', 'Analisi Modale del Sistema', 'Color', 'w', 'Position', [150, 150, 1000, 450]);

    % --- SUBPLOT 1: Mappa dei Poli nel piano s ---
    subplot(1, 2, 1);
    hold on; grid on; box on;

    lim_x = max([abs(real(poli_s)); 0.1]) * 1.5;
    lim_y = max([abs(imag(poli_s)); 0.1]) * 1.5;
    max_lim = max(lim_x, lim_y);
    xlim([-max_lim, max_lim]);
    ylim([-max_lim, max_lim]);

    fill([-max_lim, 0, 0, -max_lim], [-max_lim, -max_lim, max_lim, max_lim], ...
         [0.92, 0.97, 0.92], 'EdgeColor', 'none', 'DisplayName', 'Regione Stabile');

    xline(0, 'k-', 'LineWidth', 1.5, 'HandleVisibility', 'off');
    yline(0, 'k-', 'LineWidth', 1.5, 'HandleVisibility', 'off');

    plot(real(poli_s), imag(poli_s), 'rx', 'MarkerSize', 12, 'LineWidth', 2.5, 'DisplayName', 'Poli \lambda_i');

    for i = 1:length(poli_s)
        plot([real(poli_s(i)), real(poli_s(i))], [0, imag(poli_s(i))], 'r:', 'LineWidth', 1, 'HandleVisibility', 'off');
        plot([0, real(poli_s(i))], [imag(poli_s(i)), imag(poli_s(i))], 'r:', 'LineWidth', 1, 'HandleVisibility', 'off');
    end

    xlabel('Parte Reale \sigma [Decadimento]');
    ylabel('Parte Immaginaria j\omega [Frequenza rad/s]');
    title('Poli nel Piano Complesso s');
    legend('Location', 'northeast');

    % --- SUBPLOT 2: Confronto Misura vs Modello ---
    subplot(1, 2, 2);
    % Dati reali (punti o linea blu)
    plot(t, data.Temperature, 'bo', 'MarkerFaceColor', 'b', 'DisplayName', 'Misura (Dati)');
    hold on; grid on; box on;

    % Curva ricostruita dal modello (linea rossa spessa)
    plot(t, T_stimata, 'r-', 'LineWidth', 2, 'DisplayName', sprintf('Ricostruzione (Ordine %d)', ordine));

    % Linea di Setpoint
    yline(T_inf, 'k--', 'LineWidth', 1.5, 'DisplayName', 'T_{\infty} (Setpoint)');

    xlabel('Tempo [s]');
    ylabel('Temperatura [°C]');
    title('Fit Modello su Dati Sperimentali');
    legend('Location', 'best');
end
