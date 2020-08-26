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
% set(0,'ShowHiddenHandles','on')
% delete(get(0,'Children'))

%% initialization
close all
clear all
clear classes
clc

addpath('../../../../../cdx_format/CDX_library_functions');
%% plot CIR file

% set file parameters:
file_name = 'snacs_cir_multiple_reflectors_v01.h5';
link = 1; % plot the first link in file

% set plotting parameters:
plot_params.start = 0; % s
plot_params.length = .99; % length to plot in seconds
plot_params.title = '';
plot_params.legend_str = { 'channel impulse responses', 'true delay' };
plot_params.colormap = 'default'; % 'default' or 'gray'
plot_params.dB_range = [-50, 0];
plot_params.reference_delay_color = 'm';
plot_params.reference_delay_linewidth = 3;
plot_params.font_size = 14;
plot_params.save_format = [ 8 6 ];
plot_params.save_fig = false;
plot_params.save_pdf = false;
plot_params.shading = 'interp'; % 'flat', 'faceted', or 'interp'. Legend not supported for 'interp'.

% set interpolation parameters:
interp_params.max_delay = 1.8e-6; % s
interp_params.skip_step = 1; % only every skip_step-th CIR is plotted
interp_params.bandwidth = 80e6;
interp_params.move_to_delay0 = false;
interp_params.add_delay_offset = .1e-6;
interp_params.enable_windowing = 1;

cdx_file = CDXReadFile(file_name);

plot_continuous_delay_cir_file(cdx_file, link, plot_params, interp_params);
