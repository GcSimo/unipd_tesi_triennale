function csv_read_2(filename)
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

    % Lettura della tabella
    data = readtable(filename, opts);

    % Estrazione del tempo
    time = data.Time_Seconds;

    % Creazione della figura (finestra più alta per accomodare 4 grafici)
    figure('Name', 'Analisi Completa Sistema e PID', 'Color', 'w', 'Position', [100, 50, 950, 850]);
%    tiledlayout(4, 1, 'TileSpacing', 'compact', 'Padding', 'compact');

    % ====================================================
    % Riquadro 1: Controllo Temperatura (Sensore vs Setpoint vs PWM)
    % ====================================================
    ax1 = nexttile;
    yyaxis left
    plot(time, data.Temperature, 'r-', 'LineWidth', 1.5, 'DisplayName', 'Temp. Misurata');
    hold on;
    plot(time, data.Setpoint_Temperature, 'r--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint Temp.');
    ylabel('Temperatura (°C)');
    grid on;

    yyaxis right
    stairs(time, data.T_PWM, 'm-', 'LineWidth', 1.3, 'DisplayName', 'PWM Riscaldatore');
    ylabel('Valore T\_PWM');
    ylim([0, inf]);

    title('Controllo Temperatura');
    legend('Location', 'best', 'NumColumns', 3);
    hold off;

    % ====================================================
    % Riquadro 2: Componenti PID Temperatura (P, I, D, Out)
    % ====================================================
%    ax2 = nexttile;
%    stairs(time, data.T_PID_p, 'r-', 'LineWidth', 1.2, 'DisplayName', 'Proporzionale (P)');
%    hold on;
%    stairs(time, data.T_PID_i, 'g-', 'LineWidth', 1.2, 'DisplayName', 'Integrale (I)');
%    stairs(time, data.T_PID_d, 'b-', 'LineWidth', 1.2, 'DisplayName', 'Derivativa (D)');
%    stairs(time, data.T_PID_out, 'k--', 'LineWidth', 1.5, 'DisplayName', 'Output Totale (PID\_out)');
%
%    ylabel('Valori PID Temp');
%    grid on;
%    title('Dinamica PID Temperatura');
%    legend('Location', 'best', 'NumColumns', 4);
%    hold off;
%
%    % ====================================================
%    % Riquadro 3: Controllo Umidità (Sensore vs Setpoint vs PWM)
%    % ====================================================
%    ax3 = nexttile;
%    yyaxis left
%    plot(time, data.Humidity, 'b-', 'LineWidth', 1.5, 'DisplayName', 'Umidità Misurata');
%    hold on;
%    plot(time, data.Setpoint_Humidity, 'b--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint Umidità');
%    ylabel('Umidità (%)');
%    grid on;
%
%    yyaxis right
%    stairs(time, data.RH_PWM, 'c-', 'LineWidth', 1.3, 'DisplayName', 'PWM Umidificatore');
%    ylabel('Valore RH\_PWM');
%    ylim([0, inf]);
%
%    title('Controllo Umidità');
%    legend('Location', 'best', 'NumColumns', 3);
%    hold off;
%
%    % ====================================================
%    % Riquadro 4: Componenti PID Umidità (P, I, D, Out)
%    % ====================================================
%    ax4 = nexttile;
%    stairs(time, data.RH_PID_p, 'r-', 'LineWidth', 1.2, 'DisplayName', 'Proporzionale (P)');
%    hold on;
%    stairs(time, data.RH_PID_i, 'g-', 'LineWidth', 1.2, 'DisplayName', 'Integrale (I)');
%    stairs(time, data.RH_PID_d, 'b-', 'LineWidth', 1.2, 'DisplayName', 'Derivativa (D)');
%    stairs(time, data.RH_PID_out, 'k--', 'LineWidth', 1.5, 'DisplayName', 'Output Totale (PID\_out)');
%
%    ylabel('Valori PID Umidità');
%    xlabel('Tempo (secondi)');
%    grid on;
%    title('Dinamica PID Umidità');
%    legend('Location', 'best', 'NumColumns', 4);
%    hold off;

    % ====================================================
    % Sincronizzazione Zoom e Pan per tutti i grafici
    % ====================================================
%    linkaxes([ax1, ax2, ax3, ax4], 'x');

end
