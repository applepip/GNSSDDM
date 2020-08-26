%   Example file for
%
%   SNACS - The Satellite Navigation Radio Channel Simulator
%
%   Copyright (C) 2010  F. M. Schubert
%
%   This program is free software: you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation, either version 3 of the License, or
%   (at your option) any later version.
%
%   This program is distributed in the hope that it will be useful,
%   but WITHOUT ANY WARRANTY; without even the implied warranty of
%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%   GNU General Public License for more details.
%
%   You should have received a copy of the GNU General Public License
%   along with this program.  If not, see <http://www.gnu.org/licenses/>.
%
%
%   This file generates an artifical times series with 1 line-of-sight path
%   that begins to accelerate slowly up to a constant speed.

% if waitbars are still open from a previous program interruption, call:
set(0,'ShowHiddenHandles','on')
delete(get(0,'Children'))

%% initialization
close all
clear all
clear classes
clc

addpath('../helper_functions');
addpath('../../../../../cdx_format/CDX_library_functions');

save_figures = false;

%% parameter setting
file_name = 'snacs_cir_moving_los_v01.h5';
len = 10; % signal length in s

% set parameters for CDX file:
parameters.delay_type = 'continuous-delay';
parameters.c0 = 3e8;
parameters.cir_rate = 500;
parameters.nof_links = 1;
parameters.enable_compression = 0;
parameters.transmitter_frequency = 1.51e9;

% create CDX file:
cdx_file = CDXWriteFile(file_name, parameters);

nof_cirs = len * parameters.cir_rate;

speed = 10; % in m/s
start_acceleration = 1.0; % in s
full_speed_at = 3.0; % in s

carrier_wavelength = parameters.c0 / parameters.transmitter_frequency;

%% parameter checks

% TODO check for maximum vehicle speed:
maximum_possible_speed = parameters.c0 * parameters.cir_rate / carrier_frequency / 2; % sampling theorem

cir_start_acceleration = start_acceleration * parameters.cir_rate;
cir_full_speed_at = full_speed_at  * parameters.cir_rate;
cirs_acceleration = cir_full_speed_at - cir_start_acceleration;

speeds = zeros(nof_cirs, 1);
distances = zeros(nof_cirs, 1);
los_complex_amplitudes = zeros(nof_cirs, 1);

%% generate channel impulse response

wb = waitbar(0, 'generating channel impulse response...', 'CreateCancelBtn',...
    'setappdata(gcbf,''canceling'',1)');

for k = 1:nof_cirs
    if (mod(k, 100) == 0) waitbar(k/nof_cirs, wb); end;
    
    cir_number = k-1; % k-1: CIR number should start at 0
    
    act_speed = 0;
    
    if (k < cir_start_acceleration)
        % no movement, delay = 0:
        delay = 0;
    elseif ( (k >= cir_start_acceleration) && (k < cir_full_speed_at) )
        % acceleration using a cos function:
        speeds(k) = speed * (cos(pi + (k-cir_start_acceleration) / cirs_acceleration * pi) + 1)/2;
        
        % linear acceleration:
        %         speeds(k) = speed * (k-cir_start_acceleration) / cirs_acceleration;
    elseif (k >= cir_full_speed_at)
        % constant full speed:
        speeds(k) = speed;
    end
    
    % calculate distance using the speed. k * 1/cir_rate is the current
    % time
    if k < 2
        distances(k) = 0;
    else
        distances(k) = distances(k-1) + speeds(k) * 1/parameters.cir_rate;
    end
    
    % build struct for writing to HDF5 file:
    cir_link = struct('delay', {0}, 'real', {0}, 'imag', {0});
    
    % calculate the line of sight complex amplitude:
    los_complex_amplitudes(k) = 1.0 * exp(-1i * (2.0 * pi / carrier_wavelength) * distances(k));
    
    % calculate LOS delay from distance using speed of light:
    cir_link.delays = distances(k) / parameters.c0;
    cir_link.weights = los_complex_amplitudes(k);
    cir_link.reference_delay = distances(k) / parameters.c0;
    
    % write this CIR to file:
    append_cir(cdx_file, cir_link);
    
    % Check for Cancel button press
    if getappdata(wb, 'canceling')
        break
    end
end

delete(wb)

% IMPORTANT: close the file
delete(cdx_file);

%% plotting parameters separately

x_ax = 0:1/parameters.cir_rate:nof_cirs/parameters.cir_rate - 1/parameters.cir_rate;

f1 = figure;
title('Receiver Speed');
hold on;
plot(x_ax, speeds);
grid on;
xlabel('Time [s]');
ylabel('Speed [m/s]');
ylim([-10, speed + 20]);

if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_receiver-speed.png');  end

figure;
title('Distance Receiver - Transmitter');
hold on;
plot(x_ax, distances);
grid on;
xlabel('Time [s]');
ylabel('Distance [m]');
ylim([-10, distances(end) + 20]);

if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_distance.png'); end

figure;
title('Line-Of-Sight Amplitude');
hold on;
plot(x_ax, abs(los_complex_amplitudes));
grid on;
xlabel('Time [s]');
ylabel('Magnitude');
ylim([-10, los_complex_amplitudes(end) + 10]);

if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_amplitude.png');  end

figure;
title('Line-Of-Sight Phase');
hold on;
plot(x_ax, angle(los_complex_amplitudes));
grid on;
xlabel('Time [s]');
ylabel('Phase [rad]');
ylim([-4, 4]);

if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_phase.png');  end
