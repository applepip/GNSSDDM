%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%    Supporting script for
%
%    SNACS - The Satellite Navigation Radio Channel Simulator
%
%    plot_snacs_results.m
%
%    Matlab script for displaying results of SNACS simulations.
%
%    Copyright (C) 2009  F. M. Schubert
%
%    This program is free software: you can redistribute it and/or modify
%    it under the terms of the GNU General Public License as published by
%    the Free Software Foundation, either version 3 of the License, or
%    (at your option) any later version.
%
%    This program is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU General Public License for more details.
%
%    You should have received a copy of the GNU General Public License
%    along with this program.  If not, see <http://www.gnu.org/licenses/>.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function plot_snacs_results(file, plotting)

%% get results from result file
disp('reading simulation results...');



corr_length = hdf5read(file, '/snSDRGPS/parameters/tracking/correlation_length'); % in seconds
disp(sprintf('correlation interval: %f', corr_length));
pseudoranges = hdf5read(file, '/snSDRGPS/output/pseudoranges');

epochs = length(pseudoranges);
sim_length = epochs * corr_length;
if (plotting.length ~= 0)
    if sim_length < plotting.length
        error('provided parameter len: %f is too long for snacs simulation (%f).', plotting.length, sim_length);
    end
    if sim_length > plotting.length
        sim_length = plotting.length;
        epochs = sim_length / corr_length;
        pseudoranges = pseudoranges(1:epochs);
    end
end

disp(sprintf('simulated time: %f', sim_length));

cn0 = hdf5read(file, '/snSDRGPS/output/c_n0');
cn0 = cn0(1:epochs);
dll_filter = hdf5read(file, '/snSDRGPS/output/dll_filtered');
dll_filter= dll_filter(1:epochs);
pll_filter = hdf5read(file, '/snSDRGPS/output/pll_filtered');
pll_filter = pll_filter(1:epochs);

h = waitbar(0, 'please wait');
corr_prompt = get_complex_vector(file, '/snSDRGPS/output', 'corr_prompt');
corr_prompt = corr_prompt(1:epochs);
waitbar(1/3, h);
corr_early = get_complex_vector(file, '/snSDRGPS/output', 'corr_early');
corr_early = corr_early(1:epochs);
waitbar(2/3, h);
corr_late = get_complex_vector(file, '/snSDRGPS/output', 'corr_late');
corr_late = corr_late(1:epochs);
waitbar(3/3, h);
close(h);

% snSDRGPS outputs more parameters, use HDFView to open the file for
% evaluation

%% check for optional parameters

% example:
% was snNoise active?
f_info = hdf5info(file);
a_snblocks = numel(f_info.GroupHierarchy.Groups);

for k = 1:a_snblocks
    if strcmp(f_info.GroupHierarchy.Groups(k).Name, '/snNoise')
        noise_dB = hdf5read(file, '/snNoise/parameters/snr_db');
        disp(sprintf('snNoise was active. C/N0 = %f', noise_dB));
    end
end

%% plot
disp('plotting...');

% t axis
dt = 0:corr_length:sim_length-corr_length;

figure('Name','SNACS simulation results');

subplot(3, 2, 1);
title('pseudorange result');
hold on; grid on;
plot(dt, pseudoranges, 'b-','LineWidth', plotting.line_width);
legend('pseudorange');
xlabel('Time [s]');
ylabel('Distance [m]');
xlim([ 0, sim_length ]);

subplot(3, 2, 2);
title('correlation results');
hold on; grid on;
plot(dt, abs(corr_prompt), 'k-','LineWidth', plotting.line_width);
plot(dt, abs(corr_early), 'g-','LineWidth', plotting.line_width);
plot(dt, abs(corr_late), 'r-','LineWidth', plotting.line_width);
legend('|corr prompt|', '|corr early|', '|corr late|');
xlabel('Time [s]');
xlim([ 0, sim_length ]);

subplot(3, 2, 3);
title('prompt correlation scatter plot');
hold on; grid on;
plot(real(corr_prompt), imag(corr_prompt), 'b.','LineWidth', plotting.line_width);
legend('prompt correlation');
xlabel('I');
ylabel('Q');

subplot(3, 2, 4);
title('C/N_0 estimation');
hold on; grid on;
plot(dt, 10*log10(abs(cn0)), 'b-','LineWidth', plotting.line_width);
legend('C/N_0');
xlabel('Time [s]');
ylabel('dB');
xlim([ 0, sim_length ]);

subplot(3, 2, 5);
title('DLL filter output');
hold on; grid on;
plot(dt, dll_filter, 'b-','LineWidth', plotting.line_width);
legend('DLL filter');
xlabel('Time [s]');
xlim([ 0, sim_length ]);

subplot(3, 2, 6);
title('PLL filter output');
hold on; grid on;
plot(dt, pll_filter, 'b-','LineWidth', plotting.line_width);
legend('PLL filter');
xlabel('Time [s]');
xlim([ 0, sim_length ]);

disp('plotting done.');

end