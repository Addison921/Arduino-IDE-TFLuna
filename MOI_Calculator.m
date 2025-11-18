commandwindow; clc; clearvars; close all force;
% Moment of Inertia calculations test

disp('---------------------------------------------');
disp('         UAV Moment of Inertia Calculator     ');
disp('---------------------------------------------');
disp('Select the axis to calculate MOI for:');
disp('1 = X-axis (side view)');
disp('2 = Y-axis (front view)');
disp('3 = Z-axis (tilted / angle of attack view)');
axisChoice = input('Enter 1, 2, or 3: ');

switch axisChoice
    %% ===== X-AXIS MOI =====
    case 1  % X
        disp('--- X-Axis MOI Calculation ---');
        % Question 1 (UAV weight)
        w_u = input('What is the weight of the UAV? ');
        
        % Q2 (Cradle weight)
        w_c = input('What is the weight of the cradle? ');

        % Q3 (distance from sxis of oscillation to CoG of UAV)
        l = input('What is the distance from the axis of oscillation to the center of gravity of the UAV');

        % Q4 (Distance between AoO and CoG pendulum)
        L = input('What is the distance from the axis of oscillation to the center of gravity of the pendulum?');

        % Q5 (Period of Oscillation)
        T = input('What is the period of oscillation');

        % Q6 (MOI of empty cradle in x-axis)
        Ic_x = input('What is the Moment of Inertia of the empty cradle in the x-axis?');

        
        % x-axis MOI calculation: 
        % Ix = (((w_c + w_u) * L * T^2) / (4 * pi^2)) -l^2 - Ic_x
        Ix = (((w_c + w_u) * L * T^2) / (4 * pi^2)) -l^2 - Ic_x;
        fprintf('\nIx = %.4f kg * m^2)\n', Ix);

    %% ===== Y-AXIS CG =====
    case 2  % Y
        disp('--- Y-Axis MOI Calculation ---');
        % Question 1 (UAV weight)
        w_u = input('What is the weight of the UAV? ');
        
        % Q2 (Cradle weight)
        w_c = input('What is the weight of the cradle? ');

        % Q3 (distance from sxis of oscillation to CoG of UAV)
        l = input('What is the distance from the axis of oscillation to the center of gravity of the UAV');

        % Q4 (Distance between AoO and CoG pendulum)
        L = input('What is the distance from the axis of oscillation to the center of gravity of the pendulum?');

        % Q5 (Period of Oscillation)
        T = input('What is the period of oscillation');

        % Q6 (MOI of empty cradle in Y-axis)
        Ic_y = input('What is the Moment of Inertia of the empty cradle in the Y-axis?');

        
        % y-axis MOI calculation: 
        % Iy = (((w_c + w_u) * L * T^2) / (4 * pi^2)) -l^2 - Ic_y
        Iy = (((w_c + w_u) * L * T^2) / (4 * pi^2)) -l^2 - Ic_y;
        fprintf('\nIy = %.4f kg * m^2)\n', Iy);

    %% ===== Z-AXIS CG =====
    case 3  % Z
        disp('--- X-Axis MOI Calculation ---');
        % Question 1 (UAV weight)
        w_u = input('What is the weight of the UAV? ');
        
        % Q2 (Cradle weight)
        w_c = input('What is the weight of the cradle? ');

        % Q3 (distance between bifilars)
        d = input('What is the distance between the bifilars?');

        % Q4 (Length of bifilars)
        D = input('What is the length of the bifilars?');

        % Q5 (Period of Oscillation)
        T = input('What is the period of oscillation');

        % Q6 (MOI of empty cradle in Y-axis)
        Ic_z = input('What is the Moment of Inertia of the empty cradle in the Z-axis?');

   
        % z-axis MOI calculation: 
        % Iz = (((w_c + w_u) * d^2 * T^2) / (16 * pi^2 * D)) - Ic_z
        Iz = (((w_c + w_u) * d^2 * T^2) / (16 * pi^2 * D)) - Ic_z;
        fprintf('\nIz = %.4f kg * m^2)\n', Iz);

    otherwise
        disp('Invalid choice. Please run the script again and enter 1, 2, or 3.');
end

disp('---------------------------------------------');
disp('Calculation complete.');
disp('Run again to compute another axis.');
disp('---------------------------------------------');
