%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% This script plots a CDX (HDF5 CIR) file with continuous delay data
%
% Author: F. Schubert
% Date: 08-09-2010
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [ f ] = plot_snacs_results_with_cirs(cdx_file_name, SNACS_result_file, plot_params, interp_params)

% the most interesting parameters are probably:
% plot_params.length = 1; % length to plot in seconds
% interp_params.skip_step = 1; % only every skip_step-th CIR is plotted

%% set file parameters:
% SNACS_result_file = '/home/frank-numeric/SiS2Eart_Sounding/030_gnss-simulation/simulation_01/simulation_01_result_2010-12-15_10-44-25/result.h5';
% cdx_file_name = '/home/frank-numeric/SiS2Eart_Sounding/020_channels/snacs_cir_multiple_reflectors_v01.h5';

link = 1; % plot the first link in file

cdx_file = CDXReadFile(cdx_file_name);

% cir_rate = get_cir_rate(cdx_file);
% if plot_params.length * cir_rate / interp_params.skip_step > 2000
%     disp('This will plot more than 2000 CIRs. Are you sure? Press
% Ctrl-C to abort or any other key to proceed.');
%     pause
% end

f = plot_continuous_delay_cir_file(cdx_file, link, plot_params, interp_params);

%% insert SNACS result into plot

% read correlation length
corr_len = hdf5read(SNACS_result_file, '/snSDRGPS/parameters/tracking/correlation_length');

% read pseudoranges:
pr_result = hdf5read(SNACS_result_file, '/snSDRGPS/output/pseudoranges') / get_c0(cdx_file) + interp_params.add_delay_offset;

% calculate time axis:
pr_times = plot_params.start:corr_len:(plot_params.start + plot_params.length);

% and the corresponding indices for pr_result:
pr_indices = floor(pr_times / corr_len)+1;

plot(pr_times, pr_result(pr_indices)*3e8, ['-', plot_params.snacs_result_color], 'LineWidth', plot_params.reference_delay_linewidth);
% /1e-9
legend(plot_params.legend_str);
delete(cdx_file);

end