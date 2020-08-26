%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%    Supporting script for
%
%    SNACS - The Satellite Navigation Radio Channel Simulator
%
%    plot_snacs_results_example02.m
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

%% set interpolation parameters:
interp.skip_amount = 1; % interpolate only every interp.skip_amount-th CIR.
interp.bandwidth = 100e6; % in Hz, defines the interpolation resolution
interp.max_length = 1000e-9; % in s, the length up to which the cirs will be plotted
interp.move_to_delay0 = false;
interp.add_delay_offset = 17e-9; % in s, this is added to the reference range for better visualization

%% set plotting parameters:
plotting.length = 6; % s, total length of plot
plotting.range_dB = [-60, 0]; % 2x in dB, plots values within this dB range
plotting.show_title = 0; % print title into figure?
plotting.line_width = 3;
plotting.font_size = 14;
plotting.save = 1; % save plot?
plotting.width = 1024;
plotting.height = 768;
plotting.save_name = '../../documentation/doxygen-sources/graphic/DLRLMS_cir-result.png';

plot_snacs_cir_file('snacs-cir-file_DLRLMS_example-05.h5', interp, plotting);

% save the figure:
if plotting.save
    disp('saving plot...');
    save_figure_as_png(plotting.save_name, plotting.width, plotting.height);
end

