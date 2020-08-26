%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%    Supporting script for
%
%    SNACS - The Satellite Navigation Radio Channel Simulator
%
%    plot_snacs_results_example06.m
%
%    MATLAB script for displaying results of SNACS Example 02 simulation.
%
%    Copyright (C) 2010  F. M. Schubert
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

clear all
close all
clc

addpath('../helper_functions');

cir_file = '../example_05/snacs-cir-file_DLRLMS_example-05.h5';
snacs_result_file = '/tmp/snacs-result_example06/result.h5';

%% set interpolation parameters:
interp.skip_amount = 1; % interpolate only every interp.skip_amount-th CIR.
interp.bandwidth = 100e6; % in Hz, defines the interpolation resolution
interp.max_length = 400e-9; % in s, the length up to which the cirs will be plotted
interp.move_to_delay0 = false;
interp.add_delay_offset = 43e-9; % in s, this is added to the reference range for better visualization

%% set plotting parameters:
plotting.length = 5.8; % s, total length of plot
plotting.range_dB = [-60, 0]; % 2x in dB, plots values within this dB range
plotting.show_title = 0; % print title into figure?
plotting.line_width = 3;
plotting.font_size = 14;
plotting.save = 1; % save plot?
plotting.width = 1024;
plotting.height = 768;
plotting.save_name = [ '../../documentation/doxygen-sources/graphic/snacs-result-and-cir_example06.png' ];

%% plot the CIR first and get the reference ranges:
[ reference_ranges, delay_axis, sim_params ] = plot_snacs_cir_file(cir_file, interp, plotting);

%% get the pseudoranges:
pseudoranges = hdf5read(snacs_result_file, '/snSDRGPS/output/pr');

%% plot the reference range:

% calculate correct axis values first
ref_y_axis = 0:1/sim_params.cir_rate:plotting.length-1/sim_params.cir_rate;
% trim the number of reference_ranges to the plot length:
reference_ranges = reference_ranges(1:numel(ref_y_axis));
% reference_ranges are in m. transform to seconds:
reference_ranges = reference_ranges / sim_params.c0;

plot(reference_ranges/1e-9, ref_y_axis, 'k', 'LineWidth', plotting.line_width);

%% plot the pseudorange:

% calculate correct axis values first
pr_y_axis = 0:1/sim_params.cir_rate:plotting.length-1/sim_params.cir_rate;
% trim the number of reference_ranges to the plot length:
pseudoranges = pseudoranges(1:numel(ref_y_axis));
% reference_ranges are in m. transform to seconds:
pseudoranges = pseudoranges / sim_params.c0;

plot(pseudoranges/1e-9, pr_y_axis, 'm', 'LineWidth', plotting.line_width);

legend('channel model output', 'reference range', 'SNACS pseudorange result');

% save the figure:
if plotting.save
    save_figure_as_png(plotting.save_name, 1024, 768);
end
