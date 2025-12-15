%% Quick Plot for Traffic Analysis System
% Usage: quick_plot('scenario_name')
% Example: quick_plot('my_scenario')

function quick_plot(scenario_name)
    
    % Create output directory if it doesn't exist
    if ~exist('output', 'dir')
        mkdir('output');
    end
    
    % File paths
    csv_file = fullfile('output', [scenario_name '.csv']);
    plot_file = fullfile('output', [scenario_name '_plot.png']);
    
    % Check if file exists
    if ~isfile(csv_file)
        fprintf('Error: File %s not found!\n', csv_file);
        return;
    end
    
    % Read data from CSV
    try
        opts = detectImportOptions(csv_file);
        T = readtable(csv_file, opts);
    catch ME
        fprintf('Error reading CSV file: %s\n', ME.message);
        return;
    end
    
    % Extract columns
    k = T.k;
    v = T.v;
    q = T.q;
    
    % Calculate basic statistics
    [q_max, idx_max] = max(q);
    k_opt = k(idx_max);
    v_free = v(1);
    
    % Calculate jam density
    low_v_idx = v <= 0.1;
    if any(low_v_idx)
        k_jam = min(k(low_v_idx));
    else
        k_jam = max(k);
    end
    
    % Create figure with subplots
    fig = figure('Position', [100, 100, 1200, 500]);
    
    % Plot 1: Speed-Density
    subplot(1, 2, 1);
    plot(k, v, 'b-', 'LineWidth', 2);
    xlabel('Density (veh/km)', 'FontSize', 12);
    ylabel('Speed (km/h)', 'FontSize', 12);
    title('Speed-Density Relationship', 'FontSize', 14, 'FontWeight', 'bold');
    grid on;
    set(gca, 'GridAlpha', 0.3);
    legend('Speed', 'Location', 'best');
    
    % Plot 2: Flow-Density
    subplot(1, 2, 2);
    plot(k, q, 'r-', 'LineWidth', 2);
    hold on;
    plot(k_opt, q_max, 'g*', 'MarkerSize', 15);
    hold off;
    xlabel('Density (veh/km)', 'FontSize', 12);
    ylabel('Flow (veh/h)', 'FontSize', 12);
    title('Flow-Density Relationship', 'FontSize', 14, 'FontWeight', 'bold');
    grid on;
    set(gca, 'GridAlpha', 0.3);
    legend({'Flow'; sprintf('Capacity: %.0f veh/h', q_max)}, 'Location', 'best');
    
    % Add summary text as annotation
    summary_text = sprintf(['Analysis Summary:\n\n' ...
                           'Scenario: %s\n' ...
                           'Free-flow speed: %.1f km/h\n' ...
                           'Jam density: %.1f veh/km\n' ...
                           'Maximum flow: %.0f veh/h\n' ...
                           'Optimal density: %.1f veh/km\n' ...
                           'Data points: %d'], ...
                           scenario_name, v_free, k_jam, q_max, k_opt, length(k));
    
    tb = annotation('textbox', [0.02, 0.02, 0.3, 0.35], ...
               'String', summary_text, ...
               'FontSize', 10, ...
               'BackgroundColor', 'white', ...
               'EdgeColor', 'black', ...
               'LineWidth', 1, ...
               'Interpreter', 'none');
    tb.Color = 'black';
    
    % Save plot
    saveas(fig, plot_file);
    
    % Display plot
    % (MATLAB will show the figure window automatically)
    
    % Console output
    fprintf('✓ Plot generated: %s\n', plot_file);
    fprintf('✓ Data points: %d\n', length(k));
    fprintf('✓ Maximum flow: %.0f veh/h at density %.1f veh/km\n', q_max, k_opt);
    
    % Keep the figure window open - wait for user to close it
    disp(' ');
    disp('Plot is displayed. Close the figure window to continue.');
    waitfor(fig);

end
