commandwindow; clc; clearvars; close all force;
% UAV_CenterOfGravity.m — CG along chosen axis (one value per question)

disp('---------------------------------------------');
disp('         UAV Center of Gravity Calculator     ');
disp('---------------------------------------------');
disp('Select the axis to calculate CG for:');
disp('1 = X-axis (side view)');
disp('2 = Y-axis (front view)');
disp('3 = Z-axis (tilted / angle of attack view)');
axisChoice = input('Enter 1, 2, or 3: ');

switch axisChoice
    %% ===== X-AXIS CG =====
    case 1  % X
        disp('--- X-Axis CG Calculation ---');
        % Question 1
        nX = input('How many pegs (wheels/points where scales are placed) are on the X-axis? ');
        
        % Distances x_i (Question 3, 4, ..., for each peg)
        x = zeros(1, nX);
        for i = 1:nX
            prompt = sprintf('Distance from the nose to peg %d on X-axis (x_%d): ', i, i);
            x(i) = input(prompt);
        end
        
        % Weights W_i (Question 6, 7, ..., for each peg)
        Wx = zeros(1, nX);
        for i = 1:nX
            prompt = sprintf('Weight at peg %d (W_%d): ', i, i);
            Wx(i) = input(prompt);
        end
        
        % Total weight (Question 9)
        W_total = input('Total weight of UAV (W): ');
        
        % CG calculation: X_cg = sum(W_i * x_i) / W
        X_cg = sum(Wx .* x) / W_total;
        fprintf('\nX_cg = %.4f (same units as x)\n', X_cg);

    %% ===== Y-AXIS CG =====
    case 2  % Y
        disp('--- Y-Axis CG Calculation ---');
        % Question 1
        nY = input('How many pegs (wheels/points where scales are placed) are on the Y-axis? ');
        
        % Distances y_i
        y = zeros(1, nY);
        for i = 1:nY
            prompt = sprintf('Distance from nose (point O) to peg %d on Y-axis (y_%d): ', i, i);
            y(i) = input(prompt);
        end
        
        % Weights W_i
        Wy = zeros(1, nY);
        for i = 1:nY
            prompt = sprintf('Weight at peg %d (W_%d): ', i, i);
            Wy(i) = input(prompt);
        end
        
        % Total weight
        W_total = input('Total weight of UAV (W): ');
        
        % CG calculation: Y_cg = sum(W_i * y_i) / W
        Y_cg = sum(Wy .* y) / W_total;
        fprintf('\nY_cg = %.4f (same units as y)\n', Y_cg);

    %% ===== Z-AXIS CG =====
    case 3  % Z
        disp('--- Z-Axis CG Calculation ---');
        % Question 1
        nZ = input('How many pegs (wheels/points where scales are placed) are on this axis view? ');
        
        % Distances x_i (from nose along x-axis)
        xz = zeros(1, nZ);
        for i = 1:nZ
            prompt = sprintf('Distance from nose (point O) to peg %d along x-axis (x_%d): ', i, i);
            xz(i) = input(prompt);
        end
        
        % Distances z_i (from x-axis down to ground at each peg)
        zz = zeros(1, nZ);
        for i = 1:nZ
            prompt = sprintf('Distance from x-axis to ground at peg %d (z_%d): ', i, i);
            zz(i) = input(prompt);
        end
        
        % Weights W_i
        Wz = zeros(1, nZ);
        for i = 1:nZ
            prompt = sprintf('Weight at peg %d (W_%d): ', i, i);
            Wz(i) = input(prompt);
        end
        
        % Total weight
        W_total = input('Total weight of UAV (W): ');
        
        % Angle of attack
        theta_deg = input('Angle of attack, θ (degrees): ');
        theta = deg2rad(theta_deg);
        
        % Previously computed X_cg
        X_cg = input('Enter previously calculated X_cg value: ');
        
        % Z_cg equation:
        % Z_cg = { Σ[W_i*sinθ*z_i] + Σ[W_i*cosθ*x_i] - W*cosθ*X_cg } / (W*sinθ)
        Z_cg = ( sum(Wz .* sin(theta) .* zz) + ...
                 sum(Wz .* cos(theta) .* xz) - ...
                 W_total * cos(theta) * X_cg ) / (W_total * sin(theta));
        
        fprintf('\nZ_cg = %.4f (same units as z)\n', Z_cg);

    otherwise
        disp('Invalid choice. Please run the script again and enter 1, 2, or 3.');
end

disp('---------------------------------------------');
disp('Calculation complete.');
disp('Run again to compute another axis.');
disp('---------------------------------------------');
